cmake_minimum_required(VERSION 3.14)
project(GaBber)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
find_library(SOXR soxr libsoxr REQUIRED)
find_path(SOXR_HEADERS soxr.h REQUIRED)

add_subdirectory(extern/)
include_directories(src/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 20)

set(GABBER_CXX_FLAGS
    -flto
    -fno-exceptions
    -fstrict-aliasing -Wstrict-aliasing=1
    -Wall -Wextra -Wdisabled-optimization
    -Werror=reorder -Werror=return-type -Werror=strict-aliasing
    -Wno-packed-bitfield-compat
    )
set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g --coverage)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g -fsanitize=address -fsanitize=undefined)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -ggdb -g --sanitize=address -fsanitize=undefined)

file(GLOB_RECURSE GABBER_SOURCES src/*.cpp)
add_executable(GaBber ${GABBER_SOURCES})

target_compile_options(GaBber PRIVATE ${GABBER_CXX_FLAGS})

target_link_libraries(GaBber
    OpenGL::OpenGL
    GLEW::GLEW
    SDL2::SDL2
    ImGui
    fmt::fmt
    ${SOXR}
    )
target_include_directories(GaBber
    PRIVATE ${SOXR_HEADERS})