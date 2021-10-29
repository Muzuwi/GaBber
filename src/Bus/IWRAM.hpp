#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class IWRAM final : public BusDevice {
	ReaderArray<32 * kB> m_iwram;

	static inline uint32 mirror(uint32 address) {
		return address & 0x7fffu;
	}
public:
	IWRAM() : BusDevice(0x03000000, 0x04000000), m_iwram() {}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBA;
		}
		return m_iwram.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size() || offset+1 >= m_iwram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABE;
		}
		return m_iwram.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size() || offset+3 >= m_iwram.size()) {
			//  FIXME: unreadable I/O register
			return 0xBABEBABE;
		}
		return m_iwram.read32(offset);
	}


	void write8(uint32 offset, uint8 value) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size()) {
			return;
		}
		m_iwram.write8(offset, value);
	}

	void write16(uint32 offset, uint16 value) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size() || offset+1 >= m_iwram.size()) {
			return;
		}
		m_iwram.write16(offset, value);
	}

	void write32(uint32 offset, uint32 value) override {
		offset = mirror(offset);

		if(offset >= m_iwram.size() || offset+3 >= m_iwram.size()) {
			return;
		}
		m_iwram.write32(offset, value);
	}

	void reload() override {
		std::memset(&m_iwram.array()[0], 0x0, m_iwram.size());
	}

	unsigned int waitcycles32() const override {
		return 1;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}

	Array<uint8, 32 * kB>& buffer() {
		return m_iwram.array();
	}
};
