#include "fuente_video.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>

#include "errores.hpp"

// ---------------- FuenteVideo ----------------

FuenteVideo::~FuenteVideo() {
    // VideoCapture ya libera en su destructor; lo hacemos explícito por claridad.
    captura_.release();
}

bool FuenteVideo::leer(cv::Mat& cuadro) {
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
    if (FuenteVideo::leer(cuadro)) return true;
    if (!repetir_) return false;

    // Fin del archivo: volver al inicio e intentar de nuevo.
    captura_.set(cv::CAP_PROP_POS_FRAMES, 0);
    return FuenteVideo::leer(cuadro);
}

std::string ArchivoVideo::descripcion() const {
    return "Archivo " + std::filesystem::path(ruta_).filename().string();
}

int ArchivoVideo::retardoMs() const {
    return std::max(1, static_cast<int>(1000.0 / fps_));
}

// ---------------- Fábrica ----------------

std::unique_ptr<FuenteVideo> crearFuente(const Configuracion& config) {
    switch (config.tipoFuente) {
    case TipoFuente::Camara:
        return std::make_unique<Webcam>(config.dispositivo, config.ancho, config.alto);
    case TipoFuente::Archivo:
        return std::make_unique<ArchivoVideo>(config.rutaArchivo, config.repetir);
    }
    throw std::invalid_argument("tipo de fuente desconocido");
}
