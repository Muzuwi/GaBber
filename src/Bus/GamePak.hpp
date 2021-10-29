#pragma once
#include "Headers/StdTypes.hpp"
#include "Bus/ROM.hpp"
#include "Bus/SRAM.hpp"

struct GamePak {
	ROM rom;
	SRAM sram;

	static Optional<BackupCartType> autodetect_flash(Vector<uint8> const& rom) {
		const unsigned aligned_size = rom.size() & ~3u;
		for(unsigned i = 0; i < aligned_size; i += 4) {
			const uint32 value = *reinterpret_cast<uint32 const*>(&rom[i]);
			//  'EEPR', 'SRAM', 'FLAS' in word form
			if(value != 0x52504545 && value != 0x4d415253 && value != 0x53414c46) continue;
			if(i + 10 > rom.size()) break;

			const char* str_ptr = reinterpret_cast<const char*>(&rom[i]);
			if(std::strncmp("EEPROM_V", str_ptr, 8) == 0) {
				return {BackupCartType::EEPROM};
			}
			if(std::strncmp("SRAM_V", str_ptr, 6) == 0) {
				return {BackupCartType::SRAM32K};
			}
			if(std::strncmp("FLASH_V", str_ptr, 7) == 0) {
				return {BackupCartType::FLASH64K};
			}
			if(std::strncmp("FLASH512_V", str_ptr, 10) == 0) {
				return {BackupCartType::FLASH64K};
			}
			if(std::strncmp("FLASH1M_V", str_ptr, 9) == 0) {
				return {BackupCartType::FLASH128K};
			}
		}

		return {};
	}

	bool load_pak(Vector<uint8>&& rom_, Vector<uint8>&& sram_) {
		auto result = autodetect_flash(rom_);
		BackupCartType type;
		Vector<uint8> s {std::move(sram_)};

		if(!result.has_value()) {
			fmt::print("GamePak/ Backup cart type autodetection failed! Assuming FLASH 64K\n");
			type = BackupCartType::FLASH64K;
		} else {
			type = *result;
			fmt::print("GamePak/ Backup cart type: {}\n", type);
			switch (type) {
				case BackupCartType::FLASH64K:
					s.resize(64*kB, 0xFF); break;
				case BackupCartType::SRAM32K:
					s.resize(32*kB, 0xFF); break;
				case BackupCartType::FLASH128K:
					s.resize(128*kB, 0xFF); break;
				default:
					s.resize(64*kB, 0xFF);    //  FIXME:
			}
		}

		rom.from_vec(std::move(rom_));
		sram.from_vec(std::move(s), type);

		return true;
	}
};
