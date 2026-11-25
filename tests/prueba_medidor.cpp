#include <chrono>

#include "marco_pruebas.hpp"
#include "medidor_fps.hpp"

using namespace std::chrono_literals;

PRUEBA(medidor, "una ventana menor a 2 cuadros se rechaza") {
    VERIFICAR_LANZA(MedidorFPS(1), std::invalid_argument);
}

PRUEBA(medidor, "con menos de dos cuadros reporta 0") {
    MedidorFPS m(10);
    VERIFICAR_CERCA(m.fps(), 0.0, 1e-9);
    m.registrarCuadro(MedidorFPS::Reloj::time_point{});
    VERIFICAR_CERCA(m.fps(), 0.0, 1e-9);
}

PRUEBA(medidor, "cuadros cada 40 ms dan 25 FPS") {
    MedidorFPS m(30);
    MedidorFPS::Reloj::time_point t{};
    for (int i = 0; i < 30; ++i, t += 40ms) m.registrarCuadro(t);
    VERIFICAR_CERCA(m.fps(), 25.0, 1e-6);
}

PRUEBA(medidor, "la ventana olvida los cuadros antiguos") {
    MedidorFPS m(10);
    MedidorFPS::Reloj::time_point t{};
    for (int i = 0; i < 10; ++i, t += 100ms) m.registrarCuadro(t);  // 10 FPS
    VERIFICAR_CERCA(m.fps(), 10.0, 1e-6);
    for (int i = 0; i < 10; ++i, t += 50ms) m.registrarCuadro(t);   // 20 FPS
    VERIFICAR_CERCA(m.fps(), 20.0, 1e-6);
}

PRUEBA(medidor, "reiniciar borra el historial") {
    MedidorFPS m(5);
    m.registrarCuadro(MedidorFPS::Reloj::time_point{});
    m.registrarCuadro(MedidorFPS::Reloj::time_point{} + 10ms);
    m.reiniciar();
    VERIFICAR_CERCA(m.fps(), 0.0, 1e-9);
}
