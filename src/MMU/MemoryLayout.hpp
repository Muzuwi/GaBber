#pragma once
#include "IO/IOContainer.hpp"
#include "Devices/GamePak.hpp"
#include "Devices/SystemBIOS.hpp"
#include "IO/DebugBackdoor.hpp"
#include "Devices/RAM.hpp"
#include "Devices/VRAM.hpp"

struct MemoryLayout {
	SystemBIOS bios;
	[[maybe_unused]] OnboardWRAM onboard_wram;
	[[maybe_unused]] OnchipWRAM onchip_wram;
	IOContainer io;

	VRAM vram;
	PaletteRAM palette_ram;
	OAM oam;

	GamePak pak;
};