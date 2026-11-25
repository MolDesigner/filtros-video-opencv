#include "cadena_filtros.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>

void CadenaFiltros::agregar(char tecla, std::unique_ptr<Filtro> filtro) {
    if (!filtro)
        throw std::invalid_argument("CadenaFiltros: filtro nulo");

    const bool repetida = std::any_of(entradas_.cbegin(), entradas_.cend(),
                                      [tecla](const Entrada& e) { return e.tecla == tecla; });
    if (repetida)
        throw std::invalid_argument(std::string("CadenaFiltros: tecla repetida '") + tecla + "'");

    entradas_.push_back(Entrada{tecla, std::move(filtro), false});
}

bool CadenaFiltros::alternar(char tecla) {
    const auto it = std::find_if(entradas_.begin(), entradas_.end(),
                                 [tecla](const Entrada& e) { return e.tecla == tecla; });
    if (it == entradas_.end()) return false;
    it->activo = !it->activo;
    return true;
}

void CadenaFiltros::desactivarTodos() {
    std::for_each(entradas_.begin(), entradas_.end(), [](Entrada& e) { e.activo = false; });
}

void CadenaFiltros::procesar(const cv::Mat& entrada, cv::Mat& salida) const {
    // Doble búfer: cada filtro lee de 'actual' y escribe en 'siguiente'.
    cv::Mat actual = entrada.clone();
    cv::Mat siguiente;
    for (const auto& e : entradas_) {
        if (!e.activo) continue;
        e.filtro->aplicar(actual, siguiente);
        std::swap(actual, siguiente);
    }
    salida = actual;
}

std::vector<std::string> CadenaFiltros::nombresActivos() const {
    std::vector<std::string> nombres;
    nombres.reserve(cantidadActivos());
    for (const auto& e : entradas_)
        if (e.activo) nombres.push_back(e.filtro->nombre());
    return nombres;
}

std::vector<std::pair<char, std::string>> CadenaFiltros::listado() const {
    std::vector<std::pair<char, std::string>> resultado;
    resultado.reserve(entradas_.size());
    std::transform(entradas_.cbegin(), entradas_.cend(), std::back_inserter(resultado),
                   [](const Entrada& e) { return std::make_pair(e.tecla, e.filtro->nombre()); });
    return resultado;
}

std::size_t CadenaFiltros::cantidadActivos() const {
    return static_cast<std::size_t>(std::count_if(
        entradas_.cbegin(), entradas_.cend(), [](const Entrada& e) { return e.activo; }));
}
