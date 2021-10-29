#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"

enum class BackupCartType {
	EEPROM,
	SRAM32K,
	FLASH64K,
	FLASH128K,
};

class SRAM final : public BusDevice {
	enum class FlashChipMode {
		Idle,
		ID,
		Erase,
		Write,
		BankChange
	};

	const uint32 m_identifier {0x1cc2};
	const unsigned m_chip_size = 65536;
	const unsigned m_sector_size = 4096;
	FlashChipMode m_mode;
	uint8 m_bank;
	uint8 m_reg5555;
	uint8 m_reg2aaa;

	Vector<uint8> m_buffer;
	BackupCartType m_type;
public:
	SRAM() : BusDevice(0x0e000000, 0x0e010000) {}

	void from_vec(Vector<uint8>&& save_data, BackupCartType cart_type) {
		m_buffer = save_data;
		m_type = cart_type;
	}

	Vector<uint8>& buffer() {
		return m_buffer;
	}

	uint8 read8(uint32 offset) override;
	void write8(uint32 offset, uint8 value) override;

	uint16 read16(uint32 offset) override {
		//  FIXME: unreadable io register
		return 0xBABE;
	}
	uint32 read32(uint32 offset) override {
		//  FIXME: unreadable io register
		return 0xBABEBABE;
	}

	void write16(uint32 offset, uint16 value) override { }
	void write32(uint32 offset, uint32 value) override { }

	void reload() override { }

	unsigned int waitcycles32() const override {
		return 5;
	}

	unsigned int waitcycles16() const override {
		return 5;
	}

	unsigned int waitcycles8() const override {
		return 5;
	}
};
