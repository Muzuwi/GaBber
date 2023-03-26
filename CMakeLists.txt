cmake_minimum_required(VERSION 3.14)
project(GaBber)

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
# soxr does not have a FindXXX CMake module, we have to find it manually
find_library(SOXR soxr libsoxr REQUIRED)
find_path(SOXR_HEADERS soxr.h REQUIRED)

# For convenience, output the binaries in the repository root by default
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

add_subdirectory(lib/)
add_subdirectory(extern/)
add_subdirectory(src/)
