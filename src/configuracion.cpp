#include "configuracion.hpp"

#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

int aEntero(const std::string& texto, const std::string& opcion) {
    try {
        std::size_t usados = 0;
        const int valor = std::stoi(texto, &usados);
        if (usados != texto.size()) throw std::invalid_argument("");
        return valor;
    } catch (const std::exception&) {
        throw std::invalid_argument("'" + texto + "' no es un entero válido para " + opcion);
    }
}

double aReal(const std::string& texto, const std::string& opcion) {
    try {
        std::size_t usados = 0;
        const double valor = std::stod(texto, &usados);
        if (usados != texto.size()) throw std::invalid_argument("");
        return valor;
    } catch (const std::exception&) {
        throw std::invalid_argument("'" + texto + "' no es un número válido para " + opcion);
    }
}

void validar(const Configuracion& c) {
    if (c.dispositivo < 0)
        throw std::invalid_argument("el índice de cámara debe ser >= 0");
    if (c.tipoFuente == TipoFuente::Archivo && c.rutaArchivo.empty())
        throw std::invalid_argument("falta la ruta del archivo de video");
    if (c.ancho <= 0 || c.alto <= 0)
        throw std::invalid_argument("la resolución debe ser positiva");
    if (c.kernelDesenfoque < 3 || c.kernelDesenfoque % 2 == 0)
        throw std::invalid_argument("el kernel de desenfoque debe ser impar y >= 3");
    if (c.cannyBajo < 0 || c.cannyBajo >= c.cannyAlto)
        throw std::invalid_argument("se requiere 0 <= canny-bajo < canny-alto");
    if (c.tamanoPixel < 2)
        throw std::invalid_argument("el tamaño de pixel debe ser >= 2");
}

} // namespace

Configuracion parsearArgumentos(int argc, const char* const argv[]) {
    Configuracion c;
    const std::vector<std::string> args(argv + 1, argv + argc);

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& opcion = args[i];
        // Devuelve el valor que sigue a la opción actual o lanza si falta.
        auto valor = [&]() -> const std::string& {
            if (i + 1 >= args.size())
                throw std::invalid_argument("la opción " + opcion + " requiere un valor");
            return args[++i];
        };

        if (opcion == "-h" || opcion == "--ayuda") {
            c.mostrarAyuda = true;
        } else if (opcion == "-c" || opcion == "--camara") {
            c.tipoFuente = TipoFuente::Camara;
            c.dispositivo = aEntero(valor(), opcion);
        } else if (opcion == "-a" || opcion == "--archivo") {
            c.tipoFuente = TipoFuente::Archivo;
            c.rutaArchivo = valor();
        } else if (opcion == "--repetir") {
            c.repetir = true;
        } else if (opcion == "--ancho") {
            c.ancho = aEntero(valor(), opcion);
        } else if (opcion == "--alto") {
            c.alto = aEntero(valor(), opcion);
        } else if (opcion == "--desenfoque") {
            c.kernelDesenfoque = aEntero(valor(), opcion);
        } else if (opcion == "--canny-bajo") {
            c.cannyBajo = aReal(valor(), opcion);
        } else if (opcion == "--canny-alto") {
            c.cannyAlto = aReal(valor(), opcion);
        } else if (opcion == "--pixel") {
            c.tamanoPixel = aEntero(valor(), opcion);
        } else if (opcion == "-f" || opcion == "--filtros") {
            c.filtrosIniciales = valor();
        } else {
            throw std::invalid_argument("opción desconocida: " + opcion);
        }
    }

    validar(c);
    return c;
}

std::string textoUso(const std::string& programa) {
    std::ostringstream os;
    os << "Uso: " << programa << " [opciones]\n\n"
       << "Fuente de video:\n"
       << "  -c, --camara N        usar la webcam N (por defecto 0)\n"
       << "  -a, --archivo RUTA    usar un archivo de video\n"
       << "      --repetir         reiniciar el archivo al llegar al final\n"
       << "      --ancho N         ancho solicitado a la webcam (640)\n"
       << "      --alto N          alto solicitado a la webcam (480)\n\n"
       << "Parámetros de filtros:\n"
       << "      --desenfoque K    kernel gaussiano, impar >= 3 (15)\n"
       << "      --canny-bajo U    umbral bajo de Canny (50)\n"
       << "      --canny-alto U    umbral alto de Canny (150)\n"
       << "      --pixel N         tamaño del bloque de pixelado, >= 2 (10)\n"
       << "  -f, --filtros TECLAS  filtros activos al iniciar, p. ej. \"bc\"\n\n"
       << "  -h, --ayuda           mostrar esta ayuda\n\n"
       << "Teclas: g grises, b desenfoque, c Canny, s sepia, n negativo,\n"
       << "        p pixelado, e relieve, r quitar filtros, h ayuda en pantalla,\n"
       << "        espacio pausa, q/Esc salir\n";
    return os.str();
}
