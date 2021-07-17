#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ArrayMem.hpp"
#include "MMU/IOReg.hpp"

enum class BackupCartType {
	EEPROM,
	SRAM32K,
	FLASH64K,
	FLASH128K,
};

class WaitCtl : public IOReg16<0x04000204> {

};

class PakSRAM : public BusDevice {
	Vector<uint8> m_buffer;
public:
	PakSRAM()
	: BusDevice(0xe000000, 0x0e010000) {}

	Vector<uint8> const& get_buffer() const {
		return m_buffer;
	}

	void from_vec(Vector<uint8>&& save_data) {
		m_buffer = save_data;
		if(m_buffer.size() > 0x10000)
			m_buffer.resize(0x10000);
	}

	uint32 read32(uint32 offset) override {
		fmt::print("PakSRAM/ Read of size 32 Unsupported, offset={:08x}\n", offset);
		return 0xffffffff;
	}

	uint16 read16(uint32 offset) override {
		fmt::print("PakSRAM/ Read of size 16 Unsupported, offset={:08x}\n", offset);
		return 0xffff;
	}

	void write32(uint32 offset, uint32 value) override {
		fmt::print("PakSRAM/ Write of size 32 Unsupported, offset={:08x}\n", offset);
	}

	void write16(uint32 offset, uint16 value) override {
		fmt::print("PakSRAM/ Write of size 16 Unsupported, offset={:08x}\n", offset);
	}

	uint8 read8(uint32 offset) override {
		if(offset >= m_buffer.size()) {
			fmt::print("PakSRAM/ Undefined read from offset={:08x}\n", offset);
			return 0xff;
		}

		return m_buffer[offset];
	}

	void write8(uint32 offset, uint8 value) override {
		if(offset >= m_buffer.size()) {
			fmt::print("PakSRAM/ Undefined write to offset={:08x}\n", offset);
			return;
		}

		m_buffer[offset] = value;
	}

	std::string identify() const override {
		return "GamePak - SRAM";
	}
};

class PakROM : public BusDevice {
	Vector<uint8> m_buffer;

	//  FIXME: Taken from ArrayMem, because i don't want to extend that class that much

	template<class R>
	inline R read_safe(uint32 offset) const {
		offset = offset % 0x02000000;

		if(offset < m_buffer.size() && offset + sizeof(R) <= m_buffer.size()) {
			return *reinterpret_cast<R const*>(&m_buffer[offset]);
		} else {
			fmt::print("PakROM/ Out of bounds read{} for offset={:x}\n", sizeof(R)*8, offset);
			return (1ul << (sizeof(R)*8)) - 1;
		}
	}
public:
	PakROM()
	: BusDevice(0x08000000, 0x0e000000) {}

	void from_vec(Vector<uint8>&& rom) {
		m_buffer = rom;
		if(m_buffer.size() > 32*MB)
			m_buffer.resize(32*MB);
	}

	uint32 read32(uint32 offset) override {
		return read_safe<uint32>(offset);
	}

	uint16 read16(uint32 offset) override {
		return read_safe<uint16>(offset);
	}

	uint8 read8(uint32 offset) override {
		return read_safe<uint8>(offset);
	}

	void write32(uint32 offset, uint32) override {
		fmt::print("PakROM/ Tried writing to ROM, offset {:08x}\n", offset);
	}

	void write16(uint32 offset, uint16) override {
		fmt::print("PakROM/ Tried writing to ROM, offset {:08x}\n", offset);
	}

	void write8(uint32 offset, uint8) override {
		fmt::print("PakROM/ Tried writing to ROM, offset {:08x}\n", offset);
	}

	std::string identify() const override {
		return "GamePak - ROM";
	}
};

class GamePak {
	PakROM m_rom;
	PakSRAM m_sram;
	WaitCtl m_waitcnt;
public:
	GamePak() = default;

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

	bool load_pak(Vector<uint8>&& rom, Vector<uint8>&& sram) {
		auto result = autodetect_flash(rom);
		BackupCartType type;
		Vector<uint8> s {std::move(sram)};

		if(!result.has_value()) {
			fmt::print("GamePak/ Backup cart type autodetection failed! Assuming FLASH 64K");
			type = BackupCartType::FLASH64K;
		} else {
			type = *result;
			fmt::print("GamePak/ Backup cart type: {}\n", type);
			switch (type) {
				case BackupCartType::FLASH64K:
					s.resize(64*kB); break;
				case BackupCartType::SRAM32K:
					s.resize(32*kB); break;
				case BackupCartType::FLASH128K:
					s.resize(128*kB); break;
				default:
					s.resize(64*kB);    //  FIXME:
			}
		}

		m_rom.from_vec(std::move(rom));
		m_sram.from_vec(std::move(s));

		return true;
	}
};
