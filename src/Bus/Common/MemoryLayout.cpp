#include "Bus/Common/MemoryLayout.hpp"

MemoryLayout::MemoryLayout(GaBber& emu)
    : bios(emu)
    , wram(emu)
    , iwram(emu)
    , io(emu)
    , vram(emu)
    , oam(emu)
    , palette(emu)
    , pak(emu) {}
