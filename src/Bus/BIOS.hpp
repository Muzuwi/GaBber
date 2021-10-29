#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class BIOS final : public BusDevice {
	ReaderArray<0x4000> m_bios;
public:
	BIOS() : BusDevice(0x00000000, 0x00004000), m_bios() {}

	void from_vec(Vector<uint8> const& vec) {
		assert(vec.size() == 0x4000);
		std::memcpy(&m_bios.array()[0], &vec[0], 0x4000);
	}

	uint8 read8(uint32 offset) override {
		if(offset >= m_bios.size()) {
			//  FIXME: unreadable I/O register
			return 0xBA;
		}
		return m_bios.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		if(offset >= m_bios.size() || offset+1 >= m_bios.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABE;
		}
		return m_bios.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		if(offset >= m_bios.size() || offset+3 >= m_bios.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABEBABE;
		}
		return m_bios.read32(offset);
	}

	void write8(uint32, uint8) override {}
	void write16(uint32, uint16) override {}
	void write32(uint32, uint32) override {}

	void reload() override {}

	unsigned int waitcycles32() const override {
		return 1;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}
};