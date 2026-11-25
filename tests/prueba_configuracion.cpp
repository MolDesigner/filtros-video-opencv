#include <initializer_list>
#include <string>
#include <vector>

#include "configuracion.hpp"
#include "marco_pruebas.hpp"

namespace {

// Simula argv a partir de una lista de cadenas.
Configuracion parsear(std::initializer_list<const char*> args) {
    std::vector<const char*> argv{"programa"};
    argv.insert(argv.end(), args.begin(), args.end());
    return parsearArgumentos(static_cast<int>(argv.size()), argv.data());
}

} // namespace

PRUEBA(configuracion, "sin argumentos usa los valores por defecto") {
    const Configuracion c = parsear({});
    VERIFICAR(c.tipoFuente == TipoFuente::Camara);
    VERIFICAR_IGUAL(c.dispositivo, 0);
    VERIFICAR_IGUAL(c.kernelDesenfoque, 15);
    VERIFICAR(!c.usarHilos);
    VERIFICAR(!c.mostrarAyuda);
}

PRUEBA(configuracion, "lee todas las opciones") {
    const Configuracion c = parsear({"-a", "video.mp4", "--repetir", "--desenfoque", "7",
                                     "--canny-bajo", "20", "--canny-alto", "90.5", "--pixel", "6",
                                     "-f", "bc", "--hilos", "--cola", "8"});
    VERIFICAR(c.tipoFuente == TipoFuente::Archivo);
    VERIFICAR_IGUAL(c.rutaArchivo, std::string("video.mp4"));
    VERIFICAR(c.repetir);
    VERIFICAR_IGUAL(c.kernelDesenfoque, 7);
    VERIFICAR_CERCA(c.cannyAlto, 90.5, 1e-9);
    VERIFICAR_IGUAL(c.tamanoPixel, 6);
    VERIFICAR_IGUAL(c.filtrosIniciales, std::string("bc"));
    VERIFICAR(c.usarHilos);
    VERIFICAR_IGUAL(c.tamanoCola, 8u);
}

PRUEBA(configuracion, "rechaza opciones y valores inválidos") {
    VERIFICAR_LANZA(parsear({"--zzz"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--camara"}), std::invalid_argument);          // falta el valor
    VERIFICAR_LANZA(parsear({"--camara", "abc"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--camara", "1x"}), std::invalid_argument);    // basura al final
    VERIFICAR_LANZA(parsear({"--camara", "-1"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--desenfoque", "4"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--canny-bajo", "200"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--pixel", "1"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--ancho", "0"}), std::invalid_argument);
    VERIFICAR_LANZA(parsear({"--cola", "0"}), std::invalid_argument);
}

PRUEBA(configuracion, "el texto de uso menciona las opciones nuevas") {
    const std::string uso = textoUso("x");
    VERIFICAR(uso.find("--hilos") != std::string::npos);
    VERIFICAR(uso.find("--cola") != std::string::npos);
}
