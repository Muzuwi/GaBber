#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class WRAM final : public BusDevice {
	ReaderArray<256 * kB> m_wram;

	static inline uint32 mirror(uint32 address) {
		return address & 0x3ffffu;
	}
public:
	WRAM() : BusDevice(0x02000000, 0x03000000), m_wram() {}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_wram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBA;
		}
		return m_wram.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_wram.size() || offset+1 >= m_wram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABE;
		}
		return m_wram.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_wram.size() || offset+3 >= m_wram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABEBABE;
		}
		return m_wram.read32(offset);
	}


	void write8(uint32 offset, uint8 value) override {
		offset = mirror(offset);

		if(offset >= m_wram.size()) {
			return;
		}
		m_wram.write8(offset, value);
	}

	void write16(uint32 offset, uint16 value) override {
		offset = mirror(offset);

		if(offset >= m_wram.size() || offset+1 >= m_wram.size()) {
			return;
		}
		m_wram.write16(offset, value);
	}

	void write32(uint32 offset, uint32 value) override {
		offset = mirror(offset);

		if(offset >= m_wram.size() || offset+3 >= m_wram.size()) {
			return;
		}
		m_wram.write32(offset, value);
	}

	void reload() override {
		std::memset(&m_wram.array()[0], 0x0, m_wram.size());
	}
	
	unsigned int waitcycles32() const override {
		return 6;
	}

	unsigned int waitcycles16() const override {
		return 3;
	}

	unsigned int waitcycles8() const override {
		return 3;
	}
};
