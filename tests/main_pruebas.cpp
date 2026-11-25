// Ejecuta las pruebas registradas. Uso: pruebas_unitarias [grupo]
#include <exception>
#include <iostream>
#include <string>

#include "marco_pruebas.hpp"

int main(int argc, char* argv[]) {
    const std::string filtro = argc > 1 ? argv[1] : "";
    int ejecutadas = 0, fallidas = 0;

    for (const auto& p : pruebas::registro()) {
        if (!filtro.empty() && p.grupo != filtro) continue;
        ++ejecutadas;
        try {
            p.cuerpo();
            std::cout << "[ OK  ] " << p.grupo << " / " << p.nombre << '\n';
        } catch (const pruebas::FalloPrueba& f) {
            ++fallidas;
            std::cout << "[FALLA] " << p.grupo << " / " << p.nombre << "\n        " << f.mensaje << '\n';
        } catch (const std::exception& e) {
            ++fallidas;
            std::cout << "[FALLA] " << p.grupo << " / " << p.nombre
                      << "\n        excepción inesperada: " << e.what() << '\n';
        }
    }

    std::cout << '\n' << ejecutadas - fallidas << " de " << ejecutadas << " pruebas pasaron\n";
    if (ejecutadas == 0) {
        std::cout << "No hay pruebas para el grupo '" << filtro << "'\n";
        return 1;
    }
    return fallidas == 0 ? 0 : 1;
}
