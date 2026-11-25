#pragma once

#include <string>

#include <opencv2/core.hpp>

// Interfaz de un filtro de imagen.
// Contrato: la entrada y la salida son imágenes BGR de 8 bits (CV_8UC3),
// de modo que los filtros pueden encadenarse en cualquier orden.
class Filtro {
public:
    virtual ~Filtro() = default;

    virtual void aplicar(const cv::Mat& entrada, cv::Mat& salida) const = 0;
    virtual std::string nombre() const = 0;

protected:
    // Copia/movimiento protegidos para evitar "slicing" a través de la base.
    Filtro() = default;
    Filtro(const Filtro&) = default;
    Filtro& operator=(const Filtro&) = default;
    Filtro(Filtro&&) = default;
    Filtro& operator=(Filtro&&) = default;
};

class FiltroGrises : public Filtro {
public:
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Grises"; }
};

class FiltroDesenfoque : public Filtro {
public:
    explicit FiltroDesenfoque(int kernel);
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Desenfoque"; }

private:
    int kernel_;
};

class FiltroCanny : public Filtro {
public:
    FiltroCanny(double umbralBajo, double umbralAlto);
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Canny"; }

private:
    double umbralBajo_;
    double umbralAlto_;
};

class FiltroSepia : public Filtro {
public:
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Sepia"; }
};

class FiltroNegativo : public Filtro {
public:
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Negativo"; }
};

class FiltroPixelado : public Filtro {
public:
    explicit FiltroPixelado(int tamanoBloque);
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Pixelado"; }

private:
    int tamanoBloque_;
};

class FiltroRelieve : public Filtro {
public:
    void aplicar(const cv::Mat& entrada, cv::Mat& salida) const override;
    std::string nombre() const override { return "Relieve"; }
};
