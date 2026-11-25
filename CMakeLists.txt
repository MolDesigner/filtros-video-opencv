cmake_minimum_required(VERSION 3.16)
project(FiltrosTiempoReal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)

# Biblioteca con toda la lógica (reutilizable por el ejecutable y por futuras pruebas)
add_library(nucleo_video STATIC
  src/configuracion.cpp
  src/fuente_video.cpp
  src/filtros.cpp
  src/cadena_filtros.cpp
  src/medidor_fps.cpp
  src/ventana.cpp
  src/aplicacion.cpp
)
target_include_directories(nucleo_video PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nucleo_video PUBLIC ${OpenCV_LIBS})

if(MSVC)
  target_compile_options(nucleo_video PRIVATE /W4)
else()
  target_compile_options(nucleo_video PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(filtros_video src/main.cpp)
target_link_libraries(filtros_video PRIVATE nucleo_video)
