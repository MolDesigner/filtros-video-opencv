#pragma once

#include <chrono>
#include <cstddef>
#include <deque>

// Calcula los FPS como promedio móvil sobre los últimos N cuadros.
class MedidorFPS {
public:
    using Reloj = std::chrono::steady_clock;

    explicit MedidorFPS(std::size_t ventanaCuadros = 30);

    void registrarCuadro();                       // usa el tiempo actual
    void registrarCuadro(Reloj::time_point tiempo); // inyectable para pruebas

    double fps() const;
    void reiniciar() { marcas_.clear(); }

private:
    std::size_t ventana_;
    std::deque<Reloj::time_point> marcas_;
};
