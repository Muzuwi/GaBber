#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "MMU/ReaderArray.hpp"

class VRAM final : public BusDevice {
	ReaderArray<96 * kB> m_vram;

	static inline uint32 mirror(uint32 address) {
		return address % (128 * kB);
	}
	static inline uint32 offset_in_mirror(uint32 address) {
		uint32 mirrored = mirror(address);
		if(mirrored >= 96 * kB) {
			return (64 * kB) + mirrored % (32 * kB);
		}

		return mirrored;
	}
public:
	VRAM()
	    : BusDevice(0x06000000, 0x07000000)
	    , m_vram() {}

	uint8 read8(uint32 offset) override {
		offset = offset_in_mirror(offset);
		return m_vram.read8(offset);
	}

	uint16 read16(uint32 offset) override {
		offset = offset_in_mirror(offset);
		return m_vram.read16(offset);
	}

	uint32 read32(uint32 offset) override {
		offset = offset_in_mirror(offset);
		return m_vram.read32(offset);
	}

	void write8(uint32 offset, uint8 value) override {
		offset = offset_in_mirror(offset) & ~1u;

		//  8-bit writes ignored to OBJ
		//  TODO: Also ignore writes to 0x0014000-0x0017FFF when in bitmap modes
		if(offset >= 0x0010000 && offset <= 0x0017FFF) {
			return;
		}

		//  8-bit value is written to both the upper and lower 8-bits
		//  of the nearest half-word
		m_vram.write8(offset, value);
		m_vram.write8(offset + 1, value);
	}

	void write16(uint32 offset, uint16 value) override {
		offset = offset_in_mirror(offset);
		m_vram.write16(offset, value);
	}

	void write32(uint32 offset, uint32 value) override {
		offset = offset_in_mirror(offset);
		m_vram.write32(offset, value);
	}

	template<typename T>
	T readT(uint32 offset) {
		offset = offset_in_mirror(offset);
		return m_vram.template readT<T>(offset);
	}

	void reload() override {
		std::memset(&m_vram.array()[0], 0x0, m_vram.size());
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
