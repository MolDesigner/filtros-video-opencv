#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "cola_segura.hpp"
#include "marco_pruebas.hpp"

PRUEBA(cola, "la capacidad 0 se rechaza") {
    VERIFICAR_LANZA(ColaSegura<int>(0), std::invalid_argument);
}

PRUEBA(cola, "entrega los elementos en orden FIFO") {
    ColaSegura<int> cola(3);
    VERIFICAR(cola.insertar(1));
    VERIFICAR(cola.insertar(2));
    VERIFICAR(cola.insertar(3));
    VERIFICAR_IGUAL(cola.tamano(), 3u);
    VERIFICAR_IGUAL(*cola.extraer(), 1);
    VERIFICAR_IGUAL(*cola.extraer(), 2);
    VERIFICAR_IGUAL(*cola.extraer(), 3);
}

PRUEBA(cola, "al cerrarse entrega lo pendiente y luego nullopt") {
    ColaSegura<std::string> cola(2);
    cola.insertar("a");
    cola.cerrar();
    VERIFICAR(cola.cerrada());
    VERIFICAR(!cola.insertar("b"));       // ya no acepta datos
    VERIFICAR_IGUAL(*cola.extraer(), std::string("a"));
    VERIFICAR(!cola.extraer().has_value());
}

PRUEBA(cola, "cerrar despierta a un consumidor bloqueado") {
    ColaSegura<int> cola(1);
    bool recibioNada = false;
    std::thread consumidor([&] { recibioNada = !cola.extraer().has_value(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    cola.cerrar();
    consumidor.join();
    VERIFICAR(recibioNada);
}

PRUEBA(cola, "productor y consumidor en hilos distintos no pierden datos") {
    constexpr int kTotal = 10000;
    ColaSegura<int> cola(2);  // capacidad pequeña: el productor se bloquea seguido

    std::thread productor([&] {
        for (int i = 0; i < kTotal; ++i) cola.insertar(i);
        cola.cerrar();
    });

    std::vector<int> recibidos;
    while (auto v = cola.extraer()) recibidos.push_back(*v);
    productor.join();

    VERIFICAR_IGUAL(recibidos.size(), static_cast<std::size_t>(kTotal));
    for (int i = 0; i < kTotal; ++i) VERIFICAR_IGUAL(recibidos[i], i);
}
