#include <exception>
#include <iostream>

#include <opencv2/core.hpp>

#include "aplicacion.hpp"
#include "configuracion.hpp"
#include "errores.hpp"

int main(int argc, char* argv[]) {
    const std::string programa = argc > 0 ? argv[0] : "filtros_video";

    try {
        const Configuracion config = parsearArgumentos(argc, argv);
        if (config.mostrarAyuda) {
            std::cout << textoUso(programa);
            return 0;
        }

        Aplicacion app(config);
        app.ejecutar();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Parámetro inválido: " << e.what() << "\n\n" << textoUso(programa);
        return 2;
    } catch (const ErrorFuenteVideo& e) {
        std::cerr << "Error de fuente de video: " << e.what() << '\n';
        return 3;
    } catch (const cv::Exception& e) {
        std::cerr << "Error de OpenCV: " << e.what() << '\n';
        return 4;
    } catch (const std::exception& e) {
        std::cerr << "Error inesperado: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
