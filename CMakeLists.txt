cmake_minimum_required(VERSION 3.14)
project(GaBber)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fno-exceptions -Wall -Wextra -Werror=reorder -Werror=return-type -Wno-packed-bitfield-compat")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -no-pie -fno-pic -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -ggdb -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -ggdb -g --coverage")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -g --sanitize=address -fsanitize=undefined")

include_directories(src/)
include_directories(extern/)
add_subdirectory(extern/)

add_executable(GaBber
               src/CPU/ARM7TDMI.cpp
               src/main.cpp
               src/GaBber.cpp
               src/Headers/ARM_Instruction.hpp src/MMU/BusInterface.hpp src/MMU/BusDevice.hpp src/Devices/SystemBIOS.hpp src/PPU/PPU.hpp src/CPU/Interrupt.hpp src/MMU/IOReg.hpp src/Headers/THUMB_Instruction.hpp src/Devices/GamePak.hpp
    src/MMU/BusInterface.cpp
    src/CPU/OpcodesARM.cpp
               src/PPU/PPU.cpp
               src/CPU/OpcodesTHUMB.cpp
               src/CPU/Interrupt.cpp
               src/CPU/ALU.cpp
               src/Debugger/Debugger.cpp
               src/GaBberDisplay.cpp
               src/Tests/Harness.cpp
               src/CPU/DMA.cpp src/Devices/DebugBackdoor.hpp src/CPU/Timer.cpp src/PPU/BG.hpp src/PPU/BG.cpp src/Debugger/Screen.cpp src/Debugger/MemoryEditor.cpp src/Debugger/Registers.cpp src/Debugger/DebuggerWindow.cpp src/Debugger/IORegisters.cpp)

target_link_libraries(GaBber
    fmt
    GL
    gl3w
    ImGui
    SDL2
    dl
    gcov
)