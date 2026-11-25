#pragma once

#include <memory>
#include <string>

#include <opencv2/videoio.hpp>

#include "configuracion.hpp"

// Interfaz polimórfica para cualquier origen de cuadros de video.
// Posee un cv::VideoCapture (RAII): el dispositivo/archivo se libera al destruirse.
class FuenteVideo {
public:
    virtual ~FuenteVideo();

    FuenteVideo(const FuenteVideo&) = delete;
    FuenteVideo& operator=(const FuenteVideo&) = delete;

    // Lee el siguiente cuadro. Devuelve false cuando ya no hay más.
    virtual bool leer(cv::Mat& cuadro);

    // Texto descriptivo para mostrar en pantalla.
    virtual std::string descripcion() const = 0;

    // Milisegundos a esperar entre cuadros para respetar la velocidad del origen.
    virtual int retardoMs() const = 0;

protected:
    FuenteVideo() = default;
    cv::VideoCapture captura_;
};

class Webcam : public FuenteVideo {
public:
    Webcam(int dispositivo, int ancho, int alto);

    std::string descripcion() const override;
    int retardoMs() const override { return 1; }

private:
    int dispositivo_;
};

class ArchivoVideo : public FuenteVideo {
public:
    ArchivoVideo(const std::string& ruta, bool repetir);

    bool leer(cv::Mat& cuadro) override;
    std::string descripcion() const override;
    int retardoMs() const override;

private:
    std::string ruta_;
    bool repetir_;
    double fps_;
};

// Fábrica: crea la fuente adecuada según la configuración.
std::unique_ptr<FuenteVideo> crearFuente(const Configuracion& config);
