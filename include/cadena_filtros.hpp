#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>

#include "filtros.hpp"

// Conjunto ordenado de filtros, cada uno asociado a una tecla y activable de forma
// independiente. Los filtros activos se aplican en el orden en que se registraron.
class CadenaFiltros {
public:
    // Registra un filtro. Lanza std::invalid_argument si es nulo o la tecla ya existe.
    void agregar(char tecla, std::unique_ptr<Filtro> filtro);

    // Activa/desactiva el filtro de esa tecla. Devuelve false si no existe.
    bool alternar(char tecla);

    void desactivarTodos();

    // Aplica en cadena los filtros activos. Sin filtros activos, salida = copia de entrada.
    void procesar(const cv::Mat& entrada, cv::Mat& salida) const;

    std::vector<std::string> nombresActivos() const;
    std::vector<std::pair<char, std::string>> listado() const; // (tecla, nombre) de todos
    std::size_t cantidadActivos() const;
    std::size_t tamano() const { return entradas_.size(); }

private:
    struct Entrada {
        char tecla;
        std::unique_ptr<Filtro> filtro;
        bool activo = false;
    };

    std::vector<Entrada> entradas_;
};
