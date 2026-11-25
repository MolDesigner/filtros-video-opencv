#pragma once

#include <stdexcept>
#include <string>

// Error al abrir o leer una fuente de video (cámara no disponible, archivo inexistente...).
// Los parámetros inválidos se reportan con std::invalid_argument.
class ErrorFuenteVideo : public std::runtime_error {
public:
    explicit ErrorFuenteVideo(const std::string& mensaje)
        : std::runtime_error(mensaje) {}
};
