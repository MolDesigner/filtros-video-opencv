#include "ventana.hpp"

#include <utility>

#include <opencv2/highgui.hpp>

Ventana::Ventana(std::string titulo) : titulo_(std::move(titulo)) {
    cv::namedWindow(titulo_, cv::WINDOW_AUTOSIZE);
}

Ventana::~Ventana() {
    try {
        cv::destroyWindow(titulo_);
        cv::waitKey(1); // permite al sistema de ventanas procesar el cierre
    } catch (...) {
        // Un destructor nunca debe propagar excepciones.
    }
}

void Ventana::mostrar(const cv::Mat& imagen) const {
    cv::imshow(titulo_, imagen);
}

int Ventana::esperarTecla(int milisegundos) const {
    return cv::waitKey(milisegundos);
}

bool Ventana::estaAbierta() const {
    return cv::getWindowProperty(titulo_, cv::WND_PROP_VISIBLE) >= 1.0;
}
