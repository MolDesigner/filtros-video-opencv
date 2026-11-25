# Filtros de video en tiempo real (OpenCV + C++17)

Proyecto final de *Advanced Computing*, opción 1: **filtros en tiempo real**.
El programa captura video de una webcam o de un archivo, aplica una cadena de filtros
que se activan o desactivan con el teclado y muestra el resultado con los FPS medidos.

## Requisitos

- Compilador con soporte de C++17 (probado con g++ 13.3)
- CMake ≥ 3.16
- OpenCV 4 (probado con 4.6.0). En Ubuntu: `sudo apt install libopencv-dev`

## Compilación

```bash
cmake -S . -B build
cmake --build build -j
```

El ejecutable queda en `build/filtros_video`.

## Uso

```bash
./build/filtros_video                          # webcam 0
./build/filtros_video --camara 1               # otra webcam
./build/filtros_video --archivo video.mp4      # archivo de video (sin cámara)
./build/filtros_video -a video.mp4 --repetir   # reproducir el archivo en bucle
./build/filtros_video -f bc --desenfoque 7     # iniciar con desenfoque + Canny
./build/filtros_video --hilos --cola 4         # capturar en un hilo aparte
./build/filtros_video --ayuda
```

### Opciones

| Opción | Descripción | Por defecto |
|---|---|---|
| `-c, --camara N` | Índice de la webcam | `0` |
| `-a, --archivo RUTA` | Usar un archivo de video como fuente | |
| `--repetir` | Reiniciar el archivo al llegar al final | no |
| `--ancho N`, `--alto N` | Resolución solicitada a la webcam | `640x480` |
| `--hilos` | Capturar en un hilo aparte, con una cola protegida por `std::mutex` | no |
| `--cola N` | Capacidad de esa cola (≥ 1) | `4` |
| `--desenfoque K` | Kernel gaussiano (impar, ≥ 3) | `15` |
| `--canny-bajo U`, `--canny-alto U` | Umbrales de Canny (`0 ≤ bajo < alto`) | `50`, `150` |
| `--pixel N` | Tamaño del bloque de pixelado (≥ 2) | `10` |
| `-f, --filtros TECLAS` | Filtros activos al iniciar | ninguno |
| `-h, --ayuda` | Mostrar la ayuda | |

### Teclas

| Tecla | Acción |
|---|---|
| `b` | Desenfoque gaussiano |
| `g` | Escala de grises |
| `s` | Sepia |
| `n` | Negativo |
| `p` | Pixelado |
| `e` | Relieve (emboss) |
| `c` | Bordes (Canny) |
| `r` | Quitar todos los filtros |
| `h` | Mostrar/ocultar la ayuda en pantalla |
| `espacio` | Pausar/reanudar |
| `q` / `Esc` | Salir (también se puede cerrar la ventana) |

Los filtros activos se aplican **en el orden de la tabla**, sin importar en qué orden se
activen. Por ejemplo, `b` + `c` primero desenfoca y después detecta bordes.

### Códigos de salida

| Código | Significado |
|---|---|
| `0` | Ejecución normal |
| `1` | Error inesperado |
| `2` | Parámetro inválido en la línea de comandos |
| `3` | No se pudo abrir la cámara o el archivo de video |
| `4` | Error interno de OpenCV |

## Pruebas unitarias

Las pruebas no usan la cámara: cubren la cola entre hilos, los filtros, la cadena de filtros,
el medidor de FPS, el parser de la línea de comandos y la fuente con hilo, esta última con una
fuente falsa. No necesitan bibliotecas externas.

```bash
cmake --build build -j
ctest --test-dir build --output-on-failure   # 6 grupos, 31 pruebas
./build/pruebas_unitarias cola               # un solo grupo, con el detalle de cada prueba
```

Para compilar sin pruebas: `cmake -S . -B build -DCONSTRUIR_PRUEBAS=OFF`.

## Estructura

```
include/              cabeceras (.hpp)
  configuracion.hpp   struct Configuracion y parser de la línea de comandos
  errores.hpp         excepción ErrorFuenteVideo
  cola_segura.hpp     ColaSegura<T>: cola acotada con std::mutex y condition_variable
  fuente_video.hpp    FuenteVideo -> FuenteOpenCV -> Webcam, ArchivoVideo;
                      FuenteEnHilo (decorador que captura en otro hilo) + fábrica
  filtros.hpp         Filtro (abstracta) -> 7 filtros concretos
  cadena_filtros.hpp  CadenaFiltros: filtros asociados a teclas, activables
  medidor_fps.hpp     MedidorFPS con std::chrono (promedio móvil)
  ventana.hpp         Ventana: envoltorio RAII de una ventana de HighGUI
  aplicacion.hpp      Aplicacion: ciclo capturar -> procesar -> mostrar
src/                  implementaciones (.cpp) y main.cpp
tests/                pruebas unitarias y un marco de pruebas mínimo
docs/                 reporte (Word y PDF), diagrama de clases y figuras
CMakeLists.txt
```

## Diseño (resumen)

- **Polimorfismo:** `Aplicacion` solo conoce `FuenteVideo` y `Filtro` a través de punteros
  a la clase base, así que agregar una fuente o un filtro nuevo no obliga a cambiarla.
- **RAII:** `FuenteVideo` es dueña del `cv::VideoCapture`, `Ventana` crea y destruye la ventana,
  y `Aplicacion` es dueña de todo mediante `std::unique_ptr`. No hay `new` ni `delete`
  explícitos. Si ocurre una excepción, la cámara y la ventana se cierran de todos modos.
- **STL:** `std::vector`, `std::deque`, `std::map` con `std::function`, y los algoritmos
  `find_if`, `any_of`, `count_if`, `for_each`, `transform` y `accumulate`.
- **Concurrencia (opcional):** `FuenteEnHilo` envuelve cualquier fuente y la lee desde otro
  hilo, dejando los cuadros en una `ColaSegura`. Si el hilo productor falla, la excepción se
  guarda en un `std::exception_ptr` y se relanza en el hilo principal.
- **Excepciones:** `std::invalid_argument` para parámetros inválidos y `ErrorFuenteVideo`
  para cámaras o archivos que no se pueden abrir. `main` los atrapa y devuelve un código distinto para cada caso.
