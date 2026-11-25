#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <string>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "cola_segura.hpp"
#include "configuracion.hpp"

// Interfaz polimórfica para cualquier origen de cuadros de video.
class FuenteVideo {
public:
    virtual ~FuenteVideo() = default;

    FuenteVideo(const FuenteVideo&) = delete;
    FuenteVideo& operator=(const FuenteVideo&) = delete;

    // Lee el siguiente cuadro. Devuelve false cuando ya no hay más.
    virtual bool leer(cv::Mat& cuadro) = 0;

    // Texto descriptivo para mostrar en pantalla.
    virtual std::string descripcion() const = 0;

    // Milisegundos a esperar entre cuadros para respetar la velocidad del origen.
    virtual int retardoMs() const = 0;

protected:
    FuenteVideo() = default;
};

// Base de las fuentes que leen con OpenCV. Posee un cv::VideoCapture (RAII):
// el dispositivo o archivo se libera al destruirse el objeto.
class FuenteOpenCV : public FuenteVideo {
public:
    ~FuenteOpenCV() override;
    bool leer(cv::Mat& cuadro) override;

protected:
    FuenteOpenCV() = default;
    cv::VideoCapture captura_;
};

class Webcam : public FuenteOpenCV {
public:
    Webcam(int dispositivo, int ancho, int alto);

    std::string descripcion() const override;
    int retardoMs() const override { return 1; }

private:
    int dispositivo_;
};

class ArchivoVideo : public FuenteOpenCV {
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

// Decorador: envuelve cualquier FuenteVideo y la lee desde un hilo propio.
// El hilo productor deja los cuadros en una ColaSegura y leer() los extrae.
// El hilo se detiene y se une (join) en el destructor.
class FuenteEnHilo : public FuenteVideo {
public:
    FuenteEnHilo(std::unique_ptr<FuenteVideo> interna, std::size_t capacidadCola);
    ~FuenteEnHilo() override;

    // Si el hilo productor terminó por una excepción, se relanza aquí.
    bool leer(cv::Mat& cuadro) override;
    std::string descripcion() const override;
    int retardoMs() const override;

private:
    void producir();

    std::unique_ptr<FuenteVideo> interna_;
    ColaSegura<cv::Mat> cola_;
    std::atomic<bool> detener_{false};
    std::exception_ptr errorProductor_;  // escrito por el productor antes de cerrar la cola
    std::thread hilo_;                   // se declara al final: arranca con todo lo demás listo
};

// Fábrica: crea la fuente adecuada según la configuración.
std::unique_ptr<FuenteVideo> crearFuente(const Configuracion& config);
