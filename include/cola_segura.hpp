#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

// Cola FIFO acotada y segura entre hilos (productor-consumidor).
// - insertar() bloquea mientras la cola está llena.
// - extraer() bloquea mientras está vacía.
// - cerrar() despierta a todos: los productores dejan de insertar y los
//   consumidores vacían lo que quede y después reciben std::nullopt.
template <typename T>
class ColaSegura {
public:
    explicit ColaSegura(std::size_t capacidad) : capacidad_(capacidad) {
        if (capacidad == 0)
            throw std::invalid_argument("ColaSegura: la capacidad debe ser >= 1");
    }

    ColaSegura(const ColaSegura&) = delete;
    ColaSegura& operator=(const ColaSegura&) = delete;

    // Devuelve false si la cola se cerró y el valor no se insertó.
    bool insertar(T valor) {
        std::unique_lock<std::mutex> candado(mutex_);
        hayEspacio_.wait(candado, [this] { return cerrada_ || datos_.size() < capacidad_; });
        if (cerrada_) return false;
        datos_.push_back(std::move(valor));
        candado.unlock();
        hayDatos_.notify_one();
        return true;
    }

    // Devuelve std::nullopt solo cuando la cola está cerrada y vacía.
    std::optional<T> extraer() {
        std::unique_lock<std::mutex> candado(mutex_);
        hayDatos_.wait(candado, [this] { return cerrada_ || !datos_.empty(); });
        if (datos_.empty()) return std::nullopt;
        T valor = std::move(datos_.front());
        datos_.pop_front();
        candado.unlock();
        hayEspacio_.notify_one();
        return valor;
    }

    void cerrar() {
        {
            std::lock_guard<std::mutex> candado(mutex_);
            cerrada_ = true;
        }
        hayDatos_.notify_all();
        hayEspacio_.notify_all();
    }

    bool cerrada() const {
        std::lock_guard<std::mutex> candado(mutex_);
        return cerrada_;
    }

    std::size_t tamano() const {
        std::lock_guard<std::mutex> candado(mutex_);
        return datos_.size();
    }

    std::size_t capacidad() const { return capacidad_; }

private:
    const std::size_t capacidad_;
    mutable std::mutex mutex_;
    std::condition_variable hayDatos_;
    std::condition_variable hayEspacio_;
    std::deque<T> datos_;
    bool cerrada_ = false;
};
