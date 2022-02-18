#include "Bus/GamePak.hpp"
#include <cstring>
#include <fmt/format.h>
#include "Bus/SRAM/Flash.hpp"

std::optional<BackupCartType> GamePak::autodetect_flash(std::vector<uint8> const& rom) {
	const unsigned aligned_size = rom.size() & ~3u;
	for(unsigned i = 0; i < aligned_size; i += 4) {
		const uint32 value = *reinterpret_cast<uint32 const*>(&rom[i]);
		//  'EEPR', 'SRAM', 'FLAS' in word form
		if(value != 0x52504545 && value != 0x4d415253 && value != 0x53414c46) {
			continue;
		}
		if(i + 10 > rom.size()) {
			break;
		}

		const char* str_ptr = reinterpret_cast<const char*>(&rom[i]);
		if(std::strncmp("EEPROM_V", str_ptr, 8) == 0) {
			return { BackupCartType::EEPROM };
		}
		if(std::strncmp("SRAM_V", str_ptr, 6) == 0) {
			return { BackupCartType::SRAM32K };
		}
		if(std::strncmp("FLASH_V", str_ptr, 7) == 0) {
			return { BackupCartType::FLASH64K };
		}
		if(std::strncmp("FLASH512_V", str_ptr, 10) == 0) {
			return { BackupCartType::FLASH64K };
		}
		if(std::strncmp("FLASH1M_V", str_ptr, 9) == 0) {
			return { BackupCartType::FLASH128K };
		}
	}

	return {};
}
bool GamePak::load_pak(std::vector<uint8>&& rom_, std::vector<uint8>&& sram_) {
	auto result = autodetect_flash(rom_);
	BackupCartType type;

	if(!result.has_value()) {
		type = BackupCartType::FLASH64K;
		fmt::print("GamePak/ Backup cart type autodetection failed! Assuming FLASH 64K\n");
	} else {
		type = *result;
		fmt::print("GamePak/ Backup cart type: {}\n", static_cast<unsigned>(type));
	}

	std::unique_ptr<BackupCart> cart;
	switch(type) {
		case BackupCartType::FLASH64K: {
			cart = std::make_unique<Flash>(m_emu, 65536);
			cart->from_vec(std::move(sram_));
			break;
		}
		case BackupCartType::FLASH128K: {
			cart = std::make_unique<Flash>(m_emu, 131072);
			cart->from_vec(std::move(sram_));
			break;
		}
		default: {
			fmt::print("GamePak/ Unimplemented cart type: {}\n", static_cast<unsigned>(type));
			break;
		}
	}

	rom.from_vec(std::move(rom_));
	sram.set_cart(std::move(cart));

	return true;
}
