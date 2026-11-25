#pragma once

#include <functional>
#include <map>
#include <memory>

#include <opencv2/core.hpp>

#include "cadena_filtros.hpp"
#include "configuracion.hpp"
#include "fuente_video.hpp"
#include "medidor_fps.hpp"
#include "ventana.hpp"

// Orquesta el ciclo capturar -> procesar -> mostrar -> leer teclado.
// Es dueña de todos los recursos; al destruirse se cierran cámara y ventana.
class Aplicacion {
public:
    explicit Aplicacion(const Configuracion& config);

    void ejecutar();

private:
    static CadenaFiltros construirFiltros(const Configuracion& config);
    void registrarControles();
    void manejarTecla(int tecla);
    void dibujarInterfaz(cv::Mat& imagen) const;

    // El orden de declaración es el orden de construcción: los filtros se validan
    // antes de abrir la cámara y la ventana.
    CadenaFiltros filtros_;
    std::unique_ptr<FuenteVideo> fuente_;
    MedidorFPS medidorFps_;
    Ventana ventana_;
    std::map<int, std::function<void()>> controles_; // teclas que no son filtros

    bool ejecutando_ = true;
    bool pausado_ = false;
    bool mostrarAyuda_ = true;
};
