#include <memory>
#include <stdexcept>
#include <string>

#include <opencv2/core.hpp>

#include "errores.hpp"
#include "fuente_video.hpp"
#include "marco_pruebas.hpp"

namespace {

// Fuente falsa: produce 'total' cuadros cuyo valor es su número de orden.
// Si 'fallarEn' >= 0, lanza una excepción al leer ese cuadro.
class FuenteSintetica : public FuenteVideo {
public:
    explicit FuenteSintetica(int total, int fallarEn = -1) : total_(total), fallarEn_(fallarEn) {}

    bool leer(cv::Mat& cuadro) override {
        if (siguiente_ == fallarEn_) throw std::runtime_error("fallo simulado");
        if (siguiente_ >= total_) return false;
        cuadro = cv::Mat(4, 4, CV_8UC3, cv::Scalar::all(siguiente_++));
        return true;
    }
    std::string descripcion() const override { return "Sintetica"; }
    int retardoMs() const override { return 7; }

private:
    int total_;
    int fallarEn_;
    int siguiente_ = 0;
};

} // namespace

PRUEBA(fuentes, "un archivo inexistente lanza ErrorFuenteVideo") {
    VERIFICAR_LANZA(ArchivoVideo("/no/existe/video.mp4", false), ErrorFuenteVideo);
}

PRUEBA(fuentes, "un índice de cámara negativo se rechaza sin abrir nada") {
    VERIFICAR_LANZA(Webcam(-1, 640, 480), std::invalid_argument);
}

PRUEBA(fuentes, "FuenteEnHilo entrega todos los cuadros en orden") {
    FuenteEnHilo fuente(std::make_unique<FuenteSintetica>(200), 3);
    cv::Mat cuadro;
    for (int i = 0; i < 200; ++i) {
        VERIFICAR(fuente.leer(cuadro));
        VERIFICAR_IGUAL(static_cast<int>(cuadro.at<cv::Vec3b>(0, 0)[0]), i % 256);
    }
    VERIFICAR(!fuente.leer(cuadro));  // fin de la fuente
}

PRUEBA(fuentes, "FuenteEnHilo delega descripción y retardo") {
    FuenteEnHilo fuente(std::make_unique<FuenteSintetica>(1), 1);
    VERIFICAR_IGUAL(fuente.descripcion(), std::string("Sintetica (hilo)"));
    VERIFICAR_IGUAL(fuente.retardoMs(), 7);
}

PRUEBA(fuentes, "una excepción del hilo productor llega al consumidor") {
    FuenteEnHilo fuente(std::make_unique<FuenteSintetica>(10, 3), 2);
    cv::Mat cuadro;
    VERIFICAR(fuente.leer(cuadro));
    VERIFICAR(fuente.leer(cuadro));
    VERIFICAR(fuente.leer(cuadro));
    VERIFICAR_LANZA(fuente.leer(cuadro), std::runtime_error);
}

PRUEBA(fuentes, "destruir FuenteEnHilo con la cola llena no se bloquea") {
    // El productor queda bloqueado en insertar(); el destructor debe cerrarlo y unir el hilo.
    auto fuente = std::make_unique<FuenteEnHilo>(std::make_unique<FuenteSintetica>(1000), 1);
    fuente.reset();
    VERIFICAR(true);
}

PRUEBA(fuentes, "FuenteEnHilo rechaza una fuente nula") {
    VERIFICAR_LANZA(FuenteEnHilo(nullptr, 2), std::invalid_argument);
}
