#pragma once
#include "Bus/Common/BusDevice.hpp"
#include "Bus/Common/ReaderArray.hpp"
#include "Emulator/StdTypes.hpp"

class VRAM final : public BusDevice {
	ReaderArray<96 * kB> m_vram;

	static constexpr inline uint32 mirror(uint32 address) { return address % (128 * kB); }

	static constexpr inline uint32 offset_in_mirror(uint32 address) {
		uint32 mirrored = mirror(address);
		if(mirrored >= 96 * kB) {
			return (64 * kB) + mirrored % (32 * kB);
		}

		return mirrored;
	}
public:
	VRAM(GaBber& emu)
	    : BusDevice(emu, 0x06000000, 0x07000000)
	    , m_vram() {}

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32 offset, uint8 value) override;
	void write16(uint32 offset, uint16 value) override;
	void write32(uint32 offset, uint32 value) override;

	template<typename T>
	T readT(uint32 offset) {
		return m_vram.template readT<T>(offset_in_mirror(offset));
	}

	void reload() override;

	unsigned int waitcycles32() const override { return 2; }
	unsigned int waitcycles16() const override { return 1; }
	unsigned int waitcycles8() const override { return 1; }
};
