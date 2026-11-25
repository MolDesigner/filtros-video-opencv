#pragma once

// Marco de pruebas mínimo, sin dependencias externas.
//   PRUEBA(grupo, nombre) { VERIFICAR(cond); VERIFICAR_IGUAL(a, b); VERIFICAR_LANZA(expr, Tipo); }
// El ejecutable recibe opcionalmente un grupo y corre solo las pruebas de ese grupo.

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace pruebas {

struct FalloPrueba {
    std::string mensaje;
};

struct Prueba {
    std::string grupo;
    std::string nombre;
    std::function<void()> cuerpo;
};

inline std::vector<Prueba>& registro() {
    static std::vector<Prueba> pruebas;  // construida en el primer uso
    return pruebas;
}

struct Registrador {
    Registrador(std::string grupo, std::string nombre, std::function<void()> cuerpo) {
        registro().push_back({std::move(grupo), std::move(nombre), std::move(cuerpo)});
    }
};

inline std::string ubicacion(const char* archivo, int linea) {
    return std::string(archivo) + ":" + std::to_string(linea);
}

} // namespace pruebas

#define PRUEBAS_CONCAT_(a, b) a##b
#define PRUEBAS_CONCAT(a, b) PRUEBAS_CONCAT_(a, b)

#define PRUEBA(grupo, nombre)                                                        \
    static void PRUEBAS_CONCAT(prueba_, __LINE__)();                                 \
    static const pruebas::Registrador PRUEBAS_CONCAT(registro_, __LINE__)(           \
        #grupo, nombre, &PRUEBAS_CONCAT(prueba_, __LINE__));                         \
    static void PRUEBAS_CONCAT(prueba_, __LINE__)()

#define VERIFICAR(cond)                                                              \
    do {                                                                             \
        if (!(cond))                                                                 \
            throw pruebas::FalloPrueba{pruebas::ubicacion(__FILE__, __LINE__) +      \
                                       ": no se cumple " #cond};                     \
    } while (0)

#define VERIFICAR_IGUAL(a, b)                                                        \
    do {                                                                             \
        const auto va_ = (a); /* copia: (a) puede ser un temporal */             \
        const auto vb_ = (b);                                                        \
        if (!(va_ == vb_)) {                                                         \
            std::ostringstream os_;                                                  \
            os_ << pruebas::ubicacion(__FILE__, __LINE__) << ": " #a " == " #b       \
                << " falló (" << va_ << " vs " << vb_ << ")";                        \
            throw pruebas::FalloPrueba{os_.str()};                                   \
        }                                                                            \
    } while (0)

#define VERIFICAR_CERCA(a, b, tol)                                                   \
    do {                                                                             \
        const double da_ = (a), db_ = (b);                                           \
        if (!(da_ - db_ <= (tol) && db_ - da_ <= (tol))) {                           \
            std::ostringstream os_;                                                  \
            os_ << pruebas::ubicacion(__FILE__, __LINE__) << ": " #a " ~ " #b        \
                << " falló (" << da_ << " vs " << db_ << ")";                        \
            throw pruebas::FalloPrueba{os_.str()};                                   \
        }                                                                            \
    } while (0)

#define VERIFICAR_LANZA(expr, Tipo)                                                  \
    do {                                                                             \
        bool lanzo_ = false;                                                         \
        try { (void)(expr); } catch (const Tipo&) { lanzo_ = true; }                 \
        if (!lanzo_)                                                                 \
            throw pruebas::FalloPrueba{pruebas::ubicacion(__FILE__, __LINE__) +      \
                                       ": " #expr " no lanzó " #Tipo};               \
    } while (0)
