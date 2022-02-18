#pragma once
#include <optional>
#include <vector>
#include "Bus/ROM.hpp"
#include "Bus/SRAM.hpp"
#include "Emulator/StdTypes.hpp"

struct GamePak : Module {
	ROM rom;
	SRAM sram;

	GamePak(GaBber& emu)
	    : Module(emu)
	    , rom(emu)
	    , sram(emu) {}

	static std::optional<BackupCartType> autodetect_flash(std::vector<uint8> const& rom);
	bool load_pak(std::vector<uint8>&& rom_, std::vector<uint8>&& sram_);
};
