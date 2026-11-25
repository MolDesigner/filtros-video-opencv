#include "aplicacion.hpp"

#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

#include <opencv2/imgproc.hpp>

namespace {

constexpr int kTeclaEsc = 27;
constexpr int kTeclaEspacio = 32;

void escribir(cv::Mat& img, const std::string& texto, cv::Point pos,
              const cv::Scalar& color = cv::Scalar(255, 255, 255)) {
    // Contorno negro para que el texto sea legible sobre cualquier fondo.
    cv::putText(img, texto, pos, cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 0, 0), 3, cv::LINE_AA);
    cv::putText(img, texto, pos, cv::FONT_HERSHEY_SIMPLEX, 0.55, color, 1, cv::LINE_AA);
}

std::string unir(const std::vector<std::string>& partes, const std::string& sep) {
    if (partes.empty()) return "ninguno";
    return std::accumulate(std::next(partes.begin()), partes.end(), partes.front(),
                           [&sep](std::string acc, const std::string& p) { return acc + sep + p; });
}

} // namespace

Aplicacion::Aplicacion(const Configuracion& config)
    : filtros_(construirFiltros(config)),
      fuente_(crearFuente(config)),
      medidorFps_(30),
      ventana_("Filtros en tiempo real") {
    registrarControles();
}

CadenaFiltros Aplicacion::construirFiltros(const Configuracion& config) {
    CadenaFiltros cadena;
    // El orden de registro es el orden de aplicación: el desenfoque va antes que Canny
    // para reducir ruido, y Canny al final porque descarta el color.
    cadena.agregar('b', std::make_unique<FiltroDesenfoque>(config.kernelDesenfoque));
    cadena.agregar('g', std::make_unique<FiltroGrises>());
    cadena.agregar('s', std::make_unique<FiltroSepia>());
    cadena.agregar('n', std::make_unique<FiltroNegativo>());
    cadena.agregar('p', std::make_unique<FiltroPixelado>(config.tamanoPixel));
    cadena.agregar('e', std::make_unique<FiltroRelieve>());
    cadena.agregar('c', std::make_unique<FiltroCanny>(config.cannyBajo, config.cannyAlto));

    for (const char tecla : config.filtrosIniciales) {
        if (!cadena.alternar(tecla))
            throw std::invalid_argument(std::string("filtro inicial desconocido: '") + tecla + "'");
    }
    return cadena;
}

void Aplicacion::registrarControles() {
    auto salir = [this] { ejecutando_ = false; };
    controles_[kTeclaEsc] = salir;
    controles_['q'] = salir;
    controles_[kTeclaEspacio] = [this] { pausado_ = !pausado_; };
    controles_['r'] = [this] { filtros_.desactivarTodos(); };
    controles_['h'] = [this] { mostrarAyuda_ = !mostrarAyuda_; };
}

void Aplicacion::manejarTecla(int tecla) {
    if (tecla < 0) return;
    tecla &= 0xFF;

    if (const auto it = controles_.find(tecla); it != controles_.end()) {
        it->second();
        return;
    }
    filtros_.alternar(static_cast<char>(tecla)); // teclas desconocidas se ignoran
}

void Aplicacion::dibujarInterfaz(cv::Mat& imagen) const {
    std::ostringstream fps;
    fps << "FPS: " << std::fixed << std::setprecision(1) << medidorFps_.fps();

    int y = 22;
    const int salto = 22;
    escribir(imagen, fps.str(), {10, y}, cv::Scalar(0, 255, 0));
    escribir(imagen, fuente_->descripcion() + (pausado_ ? "  [PAUSA]" : ""), {10, y += salto});
    escribir(imagen, "Filtros: " + unir(filtros_.nombresActivos(), " + "), {10, y += salto},
             cv::Scalar(0, 255, 255));

    if (!mostrarAyuda_) return;

    y += salto / 2;
    for (const auto& [tecla, nombre] : filtros_.listado())
        escribir(imagen, std::string("[") + tecla + "] " + nombre, {10, y += salto});
    escribir(imagen, "[r] quitar  [espacio] pausa", {10, y += salto});
    escribir(imagen, "[h] ayuda   [q] salir", {10, y += salto});
}

void Aplicacion::ejecutar() {
    cv::Mat cuadro, procesado, mostrado;

    while (ejecutando_) {
        if (!pausado_ || cuadro.empty()) {
            if (!fuente_->leer(cuadro)) {
                std::cout << "Fin del video o fuente desconectada.\n";
                break;
            }
        }

        filtros_.procesar(cuadro, procesado);
        medidorFps_.registrarCuadro();

        mostrado = procesado; // la interfaz se dibuja encima del resultado
        dibujarInterfaz(mostrado);
        ventana_.mostrar(mostrado);

        manejarTecla(ventana_.esperarTecla(fuente_->retardoMs()));
        if (!ventana_.estaAbierta()) ejecutando_ = false;
    }
}
