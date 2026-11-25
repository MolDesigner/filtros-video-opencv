cmake_minimum_required(VERSION 3.16)
project(FiltrosTiempoReal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Biblioteca con toda la lógica, compartida por el ejecutable y las pruebas
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
target_link_libraries(nucleo_video PUBLIC ${OpenCV_LIBS} Threads::Threads)

if(MSVC)
  target_compile_options(nucleo_video PRIVATE /W4)
else()
  target_compile_options(nucleo_video PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(filtros_video src/main.cpp)
target_link_libraries(filtros_video PRIVATE nucleo_video)

# ---------------- Pruebas unitarias (no usan la cámara) ----------------
option(CONSTRUIR_PRUEBAS "Compilar las pruebas unitarias" ON)
if(CONSTRUIR_PRUEBAS)
  enable_testing()
  add_executable(pruebas_unitarias
    tests/main_pruebas.cpp
    tests/prueba_cola.cpp
    tests/prueba_medidor.cpp
    tests/prueba_filtros.cpp
    tests/prueba_configuracion.cpp
    tests/prueba_fuentes.cpp
  )
  target_link_libraries(pruebas_unitarias PRIVATE nucleo_video)
  target_include_directories(pruebas_unitarias PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  if(NOT MSVC)
    target_compile_options(pruebas_unitarias PRIVATE -Wall -Wextra -Wpedantic)
  endif()

  # Un caso de CTest por grupo, para ver en la salida qué parte falla.
  foreach(grupo cola medidor filtros cadena configuracion fuentes)
    add_test(NAME ${grupo} COMMAND pruebas_unitarias ${grupo})
    set_tests_properties(${grupo} PROPERTIES TIMEOUT 30)
  endforeach()
endif()
