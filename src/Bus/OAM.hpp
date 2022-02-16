#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class OAM final : public BusDevice {
	ReaderArray<1 * kB> m_oam;

	static inline uint32 mirror(uint32 address) {
		return address & 0x3ff;
	}
public:
	OAM()
	    : BusDevice(0x07000000, 0x08000000)
	    , m_oam() {}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);
		return m_oam.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		offset = mirror(offset);
		return m_oam.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		offset = mirror(offset);
		return m_oam.read32(offset);
	}

	void write8(uint32, uint8) override {
		//  8-bit writes are ignored
	}

	void write16(uint32 offset, uint16 value) override {
		offset = mirror(offset);
		m_oam.write16(offset, value);
	}

	void write32(uint32 offset, uint32 value) override {
		offset = mirror(offset);
		m_oam.write32(offset, value);
	}

	template<typename T>
	T readT(uint32 offset) {
		return m_oam.template readT<T>(offset);
	}

	void reload() override {
		std::memset(&m_oam.array()[0], 0x0, m_oam.size());
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
};
