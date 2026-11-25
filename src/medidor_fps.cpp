#include "medidor_fps.hpp"

#include <stdexcept>

MedidorFPS::MedidorFPS(std::size_t ventanaCuadros) : ventana_(ventanaCuadros) {
    if (ventanaCuadros < 2)
        throw std::invalid_argument("MedidorFPS: la ventana debe ser >= 2 cuadros");
}

void MedidorFPS::registrarCuadro() {
    registrarCuadro(Reloj::now());
}

void MedidorFPS::registrarCuadro(Reloj::time_point tiempo) {
    marcas_.push_back(tiempo);
    while (marcas_.size() > ventana_) marcas_.pop_front();
}

double MedidorFPS::fps() const {
    if (marcas_.size() < 2) return 0.0;
    const std::chrono::duration<double> transcurrido = marcas_.back() - marcas_.front();
    if (transcurrido.count() <= 0.0) return 0.0;
    return static_cast<double>(marcas_.size() - 1) / transcurrido.count();
}
