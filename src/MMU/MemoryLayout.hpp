#pragma once
#include "IO/IOContainer.hpp"
#include "Bus/GamePak.hpp"
#include "IO/DebugBackdoor.hpp"
#include "Bus/BIOS.hpp"
#include "Bus/IWRAM.hpp"
#include "Bus/OAM.hpp"
#include "Bus/Palette.hpp"
#include "Bus/ROM.hpp"
#include "Bus/SRAM.hpp"
#include "Bus/VRAM.hpp"
#include "Bus/WRAM.hpp"

struct MemoryLayout {
	BIOS bios;
	WRAM wram;
	IWRAM iwram;
	IOContainer io;
	VRAM vram;
	OAM oam;
	Palette palette;
	GamePak pak;
};