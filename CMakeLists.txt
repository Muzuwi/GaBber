cmake_minimum_required(VERSION 3.14)
project(GaBber)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 20)

set(GABBER_CXX_FLAGS -fno-exceptions -fstrict-aliasing -Wstrict-aliasing=1 -Wall -Wextra -Wdisabled-optimization -Werror=reorder -Werror=return-type -Werror=strict-aliasing -Wno-packed-bitfield-compat)
set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g --coverage)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -O2 -ggdb -g -fsanitize=address -fsanitize=undefined)
#set(GABBER_CXX_FLAGS ${GABBER_CXX_FLAGS} -ggdb -g --sanitize=address -fsanitize=undefined)

include_directories(src/)
include_directories(extern/)
add_subdirectory(extern/)

add_executable(GaBber
    src/CPU/ARM7TDMI.cpp
    src/main.cpp
    src/GaBber.cpp
    src/Headers/ARM_Instruction.hpp src/MMU/BusInterface.hpp src/MMU/BusDevice.hpp src/PPU/PPU.hpp src/IO/Interrupt.hpp src/MMU/IOReg.hpp src/Headers/THUMB_Instruction.hpp src/Bus/GamePak.hpp
    src/MMU/BusInterface.cpp
    src/CPU/OpcodesARM.cpp
    src/PPU/PPU.cpp
    src/CPU/OpcodesTHUMB.cpp
    src/CPU/Interrupt.cpp
    src/CPU/ALU.cpp
    src/Debugger/Debugger.cpp
    src/GaBberDisplay.cpp
    src/Tests/Harness.cpp
    src/CPU/DMA.cpp
    src/IO/DebugBackdoor.hpp
    src/CPU/Timer.cpp
    src/PPU/BG.hpp
    src/PPU/BG.cpp
    src/Debugger/Screen.cpp
    src/Debugger/MemoryEditor.cpp
    src/Debugger/Registers.cpp
    src/Debugger/DebuggerWindow.cpp
    src/Debugger/IORegisters.cpp
    src/IO/IOContainer.hpp
    src/MMU/MemoryLayout.hpp
    src/CPU/Bus.cpp
    src/Debugger/MemoryImage.cpp
    src/Sound/GBASound.cpp
    src/Sound/GBASound.hpp
    src/IO/Sound.cpp
    src/Headers/Bits.hpp src/MMU/ReaderArray.hpp
    src/Bus/BIOS.hpp
    src/Bus/IWRAM.hpp
    src/Bus/OAM.hpp
    src/Bus/Palette.hpp
    src/Bus/ROM.hpp
    src/Bus/SRAM.hpp
    src/Bus/VRAM.hpp
    src/Bus/WRAM.hpp
    src/Bus/SRAM.cpp
    )

target_compile_options(GaBber PRIVATE ${GABBER_CXX_FLAGS})

target_link_libraries(GaBber
    fmt
    GL
    gl3w
    ImGui
    SDL2
    dl
    gcov
)