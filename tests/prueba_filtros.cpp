#include <memory>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "cadena_filtros.hpp"
#include "filtros.hpp"
#include "marco_pruebas.hpp"

namespace {

cv::Mat imagenAleatoria() {
    cv::Mat img(48, 64, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    return img;
}

bool iguales(const cv::Mat& a, const cv::Mat& b) {
    return a.size() == b.size() && a.type() == b.type() && cv::norm(a, b, cv::NORM_INF) == 0.0;
}

// Filtros de prueba: el orden en que se aplican cambia el resultado,
// (x + 10) * 2 != x * 2 + 10, así que sirven para comprobar el orden de la cadena.
class FiltroSumar : public Filtro {
public:
    void aplicar(const cv::Mat& e, cv::Mat& s) const override { s = e + cv::Scalar::all(10); }
    std::string nombre() const override { return "Sumar"; }
};
class FiltroDuplicar : public Filtro {
public:
    void aplicar(const cv::Mat& e, cv::Mat& s) const override { s = e * 2; }
    std::string nombre() const override { return "Duplicar"; }
};

std::vector<std::unique_ptr<Filtro>> todosLosFiltros() {
    std::vector<std::unique_ptr<Filtro>> v;
    v.push_back(std::make_unique<FiltroGrises>());
    v.push_back(std::make_unique<FiltroDesenfoque>(5));
    v.push_back(std::make_unique<FiltroCanny>(50, 150));
    v.push_back(std::make_unique<FiltroSepia>());
    v.push_back(std::make_unique<FiltroNegativo>());
    v.push_back(std::make_unique<FiltroPixelado>(4));
    v.push_back(std::make_unique<FiltroRelieve>());
    return v;
}

} // namespace

// ---------------- Filtros ----------------

PRUEBA(filtros, "todos entregan BGR de 8 bits del mismo tamaño") {
    const cv::Mat entrada = imagenAleatoria();
    for (const auto& f : todosLosFiltros()) {
        cv::Mat salida;
        f->aplicar(entrada, salida);
        if (salida.type() != CV_8UC3 || salida.size() != entrada.size())
            throw pruebas::FalloPrueba{"el filtro " + f->nombre() + " rompe el contrato BGR"};
    }
}

PRUEBA(filtros, "los constructores rechazan parámetros inválidos") {
    VERIFICAR_LANZA(FiltroDesenfoque(4), std::invalid_argument);   // par
    VERIFICAR_LANZA(FiltroDesenfoque(1), std::invalid_argument);   // muy pequeño
    VERIFICAR_LANZA(FiltroCanny(150, 50), std::invalid_argument);  // invertidos
    VERIFICAR_LANZA(FiltroCanny(-1, 50), std::invalid_argument);   // negativo
    VERIFICAR_LANZA(FiltroPixelado(1), std::invalid_argument);
}

PRUEBA(filtros, "negativo aplicado dos veces devuelve la imagen original") {
    const cv::Mat entrada = imagenAleatoria();
    cv::Mat una, dos;
    FiltroNegativo n;
    n.aplicar(entrada, una);
    n.aplicar(una, dos);
    VERIFICAR(!iguales(entrada, una));
    VERIFICAR(iguales(entrada, dos));
}

PRUEBA(filtros, "grises deja los tres canales iguales") {
    cv::Mat salida;
    FiltroGrises().aplicar(imagenAleatoria(), salida);
    std::vector<cv::Mat> canales;
    cv::split(salida, canales);
    VERIFICAR(iguales(canales[0], canales[1]));
    VERIFICAR(iguales(canales[1], canales[2]));
}

PRUEBA(filtros, "Canny no encuentra bordes en una imagen uniforme") {
    const cv::Mat gris(48, 64, CV_8UC3, cv::Scalar(120, 120, 120));
    cv::Mat salida;
    FiltroCanny(50, 150).aplicar(gris, salida);
    VERIFICAR_IGUAL(cv::countNonZero(salida.reshape(1)), 0);
}

// ---------------- CadenaFiltros ----------------

PRUEBA(cadena, "rechaza filtros nulos y teclas repetidas") {
    CadenaFiltros c;
    VERIFICAR_LANZA(c.agregar('x', nullptr), std::invalid_argument);
    c.agregar('n', std::make_unique<FiltroNegativo>());
    VERIFICAR_LANZA(c.agregar('n', std::make_unique<FiltroGrises>()), std::invalid_argument);
    VERIFICAR_IGUAL(c.tamano(), 1u);
}

PRUEBA(cadena, "sin filtros activos copia la entrada sin compartir memoria") {
    CadenaFiltros c;
    c.agregar('n', std::make_unique<FiltroNegativo>());
    const cv::Mat entrada = imagenAleatoria();
    cv::Mat salida;
    c.procesar(entrada, salida);
    VERIFICAR(iguales(entrada, salida));
    VERIFICAR(salida.data != entrada.data);
}

PRUEBA(cadena, "alternar activa, desactiva e ignora teclas desconocidas") {
    CadenaFiltros c;
    c.agregar('g', std::make_unique<FiltroGrises>());
    c.agregar('n', std::make_unique<FiltroNegativo>());
    VERIFICAR(!c.alternar('z'));
    VERIFICAR(c.alternar('n'));
    VERIFICAR(c.alternar('g'));
    VERIFICAR_IGUAL(c.cantidadActivos(), 2u);
    VERIFICAR(c.alternar('n'));
    VERIFICAR_IGUAL(c.cantidadActivos(), 1u);
    c.desactivarTodos();
    VERIFICAR_IGUAL(c.cantidadActivos(), 0u);
}

PRUEBA(cadena, "aplica en orden de registro, no de activación") {
    CadenaFiltros c;
    c.agregar('s', std::make_unique<FiltroSumar>());
    c.agregar('d', std::make_unique<FiltroDuplicar>());
    c.alternar('d');  // se activa primero el segundo filtro registrado
    c.alternar('s');

    const cv::Mat entrada(2, 2, CV_8UC3, cv::Scalar::all(5));
    cv::Mat salida;
    c.procesar(entrada, salida);
    VERIFICAR_IGUAL(static_cast<int>(salida.at<cv::Vec3b>(0, 0)[0]), 30);  // (5 + 10) * 2

    const std::vector<std::string> esperados{"Sumar", "Duplicar"};
    VERIFICAR(c.nombresActivos() == esperados);
}

PRUEBA(cadena, "listado devuelve tecla y nombre de cada filtro") {
    CadenaFiltros c;
    c.agregar('g', std::make_unique<FiltroGrises>());
    c.agregar('c', std::make_unique<FiltroCanny>(10, 20));
    const auto l = c.listado();
    VERIFICAR_IGUAL(l.size(), 2u);
    VERIFICAR_IGUAL(l[0].first, 'g');
    VERIFICAR_IGUAL(l[1].second, std::string("Canny"));
}
