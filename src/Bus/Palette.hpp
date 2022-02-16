#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class Palette final : public BusDevice {
	ReaderArray<1 * kB> m_palette;

	static inline uint32 mirror(uint32 address) {
		return address & 0x3ff;
	}
public:
	Palette()
	    : BusDevice(0x05000000, 0x06000000)
	    , m_palette() {}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);
		return m_palette.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		offset = mirror(offset);
		return m_palette.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		offset = mirror(offset);
		return m_palette.read32(offset);
	}

	void write8(uint32 offset, uint8 value) override {
		offset = mirror(offset) & ~1u;

		//  8-bit value is written to both the upper and lower 8-bits
		//  of the nearest half-word
		m_palette.write8(offset, value);
		m_palette.write8(offset + 1, value);
	}

	void write16(uint32 offset, uint16 value) override {
		offset = mirror(offset);
		m_palette.write16(offset, value);
	}

	void write32(uint32 offset, uint32 value) override {
		offset = mirror(offset);
		m_palette.write32(offset, value);
	}

	template<typename T>
	T readT(uint32 offset) {
		return m_palette.template readT<T>(offset);
	}

	void reload() override {
		std::memset(&m_palette.array()[0], 0x0, m_palette.size());
	}

	unsigned int waitcycles32() const override {
		return 2;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}
};
