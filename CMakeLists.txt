cmake_minimum_required(VERSION 3.14)
project(GaBber)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)

include_directories(src/)
include_directories(extern/)
add_subdirectory(extern/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 20)

set(GABBER_CXX_FLAGS -fno-exceptions
    -fstrict-aliasing -Wstrict-aliasing=1
    -Wall -Wextra
    -Wdisabled-optimization -Werror=reorder -Werror=return-type -Werror=strict-aliasing
    -Wno-packed-bitfield-compat)
set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g --coverage)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g -fsanitize=address -fsanitize=undefined)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -ggdb -g --sanitize=address -fsanitize=undefined)

add_executable(GaBber
    src/CPU/ARM7TDMI.cpp
    src/main.cpp
    src/GaBber.cpp
    src/MMU/BusInterface.cpp
    src/CPU/Instructions/ARM.cpp
    src/PPU/PPU.cpp
    src/CPU/Instructions/THUMB.cpp
    src/CPU/Interrupt.cpp
    src/CPU/ALU.cpp
    src/Debugger/Debugger.cpp
    src/GaBberDisplay.cpp
    src/Tests/Harness.cpp
    src/CPU/DMA.cpp
    src/CPU/Timer.cpp
    src/PPU/BG.cpp
    src/Debugger/Screen.cpp
    src/Debugger/MemoryEditor.cpp
    src/Debugger/Registers.cpp
    src/Debugger/DebuggerWindow.cpp
    src/Debugger/IORegisters.cpp
    src/CPU/Bus.cpp
    src/Sound/GBASound.cpp
    src/IO/Sound.cpp
    src/IO/DMA.cpp
    src/Bus/SRAM/Flash.cpp
    src/Debugger/BreakpointControl.cpp
    src/Debugger/Stacktrace.cpp
    src/GaBberShell.cpp
    )

target_compile_options(GaBber PRIVATE ${GABBER_CXX_FLAGS})

target_link_libraries(GaBber
    OpenGL
    GLEW
    SDL2
    ImGui
    fmt
    )