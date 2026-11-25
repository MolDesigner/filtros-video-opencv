#include "fuente_video.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include "errores.hpp"

// ---------------- FuenteOpenCV ----------------

FuenteOpenCV::~FuenteOpenCV() {
    // VideoCapture ya libera en su destructor; lo hacemos explícito por claridad.
    captura_.release();
}

bool FuenteOpenCV::leer(cv::Mat& cuadro) {
    return captura_.read(cuadro) && !cuadro.empty();
}

// ---------------- Webcam ----------------

Webcam::Webcam(int dispositivo, int ancho, int alto) : dispositivo_(dispositivo) {
    if (dispositivo < 0)
        throw std::invalid_argument("índice de cámara inválido: " + std::to_string(dispositivo));
    if (ancho <= 0 || alto <= 0)
        throw std::invalid_argument("resolución de cámara inválida");

    if (!captura_.open(dispositivo))
        throw ErrorFuenteVideo("no se pudo abrir la cámara " + std::to_string(dispositivo));

    captura_.set(cv::CAP_PROP_FRAME_WIDTH, ancho);
    captura_.set(cv::CAP_PROP_FRAME_HEIGHT, alto);
}

std::string Webcam::descripcion() const {
    return "Webcam " + std::to_string(dispositivo_);
}

// ---------------- ArchivoVideo ----------------

ArchivoVideo::ArchivoVideo(const std::string& ruta, bool repetir)
    : ruta_(ruta), repetir_(repetir), fps_(30.0) {
    if (!std::filesystem::exists(ruta))
        throw ErrorFuenteVideo("el archivo no existe: " + ruta);

    if (!captura_.open(ruta))
        throw ErrorFuenteVideo("no se pudo abrir el archivo de video: " + ruta);

    const double fps = captura_.get(cv::CAP_PROP_FPS);
    if (std::isfinite(fps) && fps > 0.0) fps_ = fps;
}

bool ArchivoVideo::leer(cv::Mat& cuadro) {
    if (FuenteOpenCV::leer(cuadro)) return true;
    if (!repetir_) return false;

    // Fin del archivo: volver al inicio e intentar de nuevo.
    captura_.set(cv::CAP_PROP_POS_FRAMES, 0);
    return FuenteOpenCV::leer(cuadro);
}

std::string ArchivoVideo::descripcion() const {
    return "Archivo " + std::filesystem::path(ruta_).filename().string();
}

int ArchivoVideo::retardoMs() const {
    return std::max(1, static_cast<int>(1000.0 / fps_));
}

// ---------------- FuenteEnHilo ----------------

FuenteEnHilo::FuenteEnHilo(std::unique_ptr<FuenteVideo> interna, std::size_t capacidadCola)
    : interna_(std::move(interna)), cola_(capacidadCola) {
    if (!interna_)
        throw std::invalid_argument("FuenteEnHilo: la fuente interna es nula");
    hilo_ = std::thread(&FuenteEnHilo::producir, this);
}

FuenteEnHilo::~FuenteEnHilo() {
    detener_ = true;
    cola_.cerrar();  // despierta al productor si está bloqueado en insertar()
    if (hilo_.joinable()) hilo_.join();
}

void FuenteEnHilo::producir() {
    try {
        while (!detener_) {
            cv::Mat cuadro;  // un Mat nuevo por cuadro: la cola no comparte búferes
            if (!interna_->leer(cuadro)) break;
            if (!cola_.insertar(std::move(cuadro))) break;
        }
    } catch (...) {
        // Una excepción no puede cruzar el límite del hilo: se guarda y se
        // relanza en el hilo consumidor desde leer().
        errorProductor_ = std::current_exception();
    }
    cola_.cerrar();
}

bool FuenteEnHilo::leer(cv::Mat& cuadro) {
    std::optional<cv::Mat> siguiente = cola_.extraer();
    if (!siguiente) {
        // cerrar() y extraer() usan el mismo mutex, así que errorProductor_ ya es visible.
        if (errorProductor_) std::rethrow_exception(errorProductor_);
        return false;
    }
    cuadro = std::move(*siguiente);
    return true;
}

std::string FuenteEnHilo::descripcion() const {
    return interna_->descripcion() + " (hilo)";
}

int FuenteEnHilo::retardoMs() const {
    return interna_->retardoMs();
}

// ---------------- Fábrica ----------------

std::unique_ptr<FuenteVideo> crearFuente(const Configuracion& config) {
    std::unique_ptr<FuenteVideo> fuente;
    switch (config.tipoFuente) {
    case TipoFuente::Camara:
        fuente = std::make_unique<Webcam>(config.dispositivo, config.ancho, config.alto);
        break;
    case TipoFuente::Archivo:
        fuente = std::make_unique<ArchivoVideo>(config.rutaArchivo, config.repetir);
        break;
    }
    if (!fuente) throw std::invalid_argument("tipo de fuente desconocido");

    if (config.usarHilos)
        return std::make_unique<FuenteEnHilo>(std::move(fuente), config.tamanoCola);
    return fuente;
}
