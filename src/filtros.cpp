#include "filtros.hpp"

#include <algorithm>
#include <stdexcept>

#include <opencv2/imgproc.hpp>

// ---------------- Grises ----------------

void FiltroGrises::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    cv::Mat gris;
    cv::cvtColor(entrada, gris, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gris, salida, cv::COLOR_GRAY2BGR); // mantener 3 canales
}

// ---------------- Desenfoque ----------------

FiltroDesenfoque::FiltroDesenfoque(int kernel) : kernel_(kernel) {
    if (kernel < 3 || kernel % 2 == 0)
        throw std::invalid_argument("FiltroDesenfoque: el kernel debe ser impar y >= 3");
}

void FiltroDesenfoque::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    cv::GaussianBlur(entrada, salida, cv::Size(kernel_, kernel_), 0);
}

// ---------------- Canny ----------------

FiltroCanny::FiltroCanny(double umbralBajo, double umbralAlto)
    : umbralBajo_(umbralBajo), umbralAlto_(umbralAlto) {
    if (umbralBajo < 0 || umbralBajo >= umbralAlto)
        throw std::invalid_argument("FiltroCanny: se requiere 0 <= bajo < alto");
}

void FiltroCanny::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    cv::Mat gris, bordes;
    cv::cvtColor(entrada, gris, cv::COLOR_BGR2GRAY);
    cv::Canny(gris, bordes, umbralBajo_, umbralAlto_);
    cv::cvtColor(bordes, salida, cv::COLOR_GRAY2BGR);
}

// ---------------- Sepia ----------------

void FiltroSepia::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    // Matriz de sepia clásica, con filas/columnas en orden BGR.
    static const cv::Matx33f kSepia(
        0.131f, 0.534f, 0.272f,   // B'
        0.168f, 0.686f, 0.349f,   // G'
        0.189f, 0.769f, 0.393f);  // R'
    cv::transform(entrada, salida, kSepia); // satura a [0,255] en CV_8U
}

// ---------------- Negativo ----------------

void FiltroNegativo::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    cv::bitwise_not(entrada, salida);
}

// ---------------- Pixelado ----------------

FiltroPixelado::FiltroPixelado(int tamanoBloque) : tamanoBloque_(tamanoBloque) {
    if (tamanoBloque < 2)
        throw std::invalid_argument("FiltroPixelado: el bloque debe ser >= 2");
}

void FiltroPixelado::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    const cv::Size pequeno(std::max(1, entrada.cols / tamanoBloque_),
                           std::max(1, entrada.rows / tamanoBloque_));
    cv::Mat reducido;
    cv::resize(entrada, reducido, pequeno, 0, 0, cv::INTER_LINEAR);
    cv::resize(reducido, salida, entrada.size(), 0, 0, cv::INTER_NEAREST);
}

// ---------------- Relieve ----------------

void FiltroRelieve::aplicar(const cv::Mat& entrada, cv::Mat& salida) const {
    static const cv::Matx33f kRelieve(
        -2.f, -1.f, 0.f,
        -1.f,  1.f, 1.f,
         0.f,  1.f, 2.f);
    cv::filter2D(entrada, salida, -1, kRelieve, cv::Point(-1, -1), 0.0);
}
