#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"

class ROM final : public BusDevice {
	Vector<uint8> m_rom {};

	static inline uint32 mirror(uint32 offset) {
		return offset % 0x02000000;
	}
public:
	ROM() : BusDevice(0x08000000, 0x0e000000), m_rom() {}

	void from_vec(Vector<uint8>&& vec) {
		m_rom = vec;
		if(m_rom.size() > 32*MB) {
			m_rom.resize(32*MB);
		}
	}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_rom.size()) {
			//  FIXME: unreadable I/O register
			return 0xBA;
		}
		return m_rom[offset];
	}
	uint16 read16(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_rom.size() || offset + 1 >= m_rom.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABE;
		}
		return *reinterpret_cast<uint16*>(&m_rom[0] + offset);
	}
	uint32 read32(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_rom.size() || offset + 3 >= m_rom.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABEBABE;
		}
		return *reinterpret_cast<uint32*>(&m_rom[0] + offset);
	}

	void write8(uint32, uint8) override {}
	void write16(uint32, uint16) override {}
	void write32(uint32, uint32) override {}

	unsigned int waitcycles32() const override {
		return 8;
	}

	unsigned int waitcycles16() const override {
		return 5;
	}

	unsigned int waitcycles8() const override {
		return 5;
	}
};