#pragma once

#include <string>

#include <opencv2/core.hpp>

// Ventana de HighGUI con RAII: se crea en el constructor y se destruye en el destructor,
// incluso si una excepción interrumpe el programa.
class Ventana {
public:
    explicit Ventana(std::string titulo);
    ~Ventana();

    Ventana(const Ventana&) = delete;
    Ventana& operator=(const Ventana&) = delete;

    void mostrar(const cv::Mat& imagen) const;
    int esperarTecla(int milisegundos) const; // -1 si no se presionó nada
    bool estaAbierta() const;                 // false si el usuario la cerró con la X

    const std::string& titulo() const { return titulo_; }

private:
    std::string titulo_;
};
