#pragma once

#include <string>

enum class TipoFuente { Camara, Archivo };

// Parámetros de ejecución, obtenidos de la línea de comandos.
struct Configuracion {
    TipoFuente tipoFuente = TipoFuente::Camara;
    int dispositivo = 0;          // índice de la webcam
    std::string rutaArchivo;      // usado si tipoFuente == Archivo
    bool repetir = false;         // reiniciar el archivo al terminar
    int ancho = 640;              // resolución solicitada a la webcam
    int alto = 480;

    int kernelDesenfoque = 15;    // impar y > 1
    double cannyBajo = 50.0;      // 0 <= bajo < alto
    double cannyAlto = 150.0;
    int tamanoPixel = 10;         // >= 2

    std::string filtrosIniciales; // teclas de filtros activos al inicio, p. ej. "bc"
    bool mostrarAyuda = false;
};

// Lanza std::invalid_argument si algún parámetro es desconocido o inválido.
Configuracion parsearArgumentos(int argc, const char* const argv[]);

std::string textoUso(const std::string& programa);
