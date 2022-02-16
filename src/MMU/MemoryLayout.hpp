#pragma once
#include "Bus/BIOS.hpp"
#include "Bus/GamePak.hpp"
#include "Bus/IWRAM.hpp"
#include "Bus/OAM.hpp"
#include "Bus/Palette.hpp"
#include "Bus/ROM.hpp"
#include "Bus/SRAM.hpp"
#include "Bus/VRAM.hpp"
#include "Bus/WRAM.hpp"
#include "IO/DebugBackdoor.hpp"
#include "IO/IOContainer.hpp"

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