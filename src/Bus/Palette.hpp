#pragma once
#include "Bus/Common/BusDevice.hpp"
#include "Bus/Common/ReaderArray.hpp"
#include "Emulator/StdTypes.hpp"

class Palette final : public BusDevice {
	ReaderArray<1 * kB> m_palette;

	static constexpr inline uint32 mirror(uint32 address) { return address & 0x3ff; }
public:
	Palette(GaBber& emu)
	    : BusDevice(emu, 0x05000000, 0x06000000)
	    , m_palette() {}

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32 offset, uint8 value) override;
	void write16(uint32 offset, uint16 value) override;
	void write32(uint32 offset, uint32 value) override;

	template<typename T>
	T readT(uint32 offset) {
		return m_palette.template readT<T>(offset);
	}

	void reload() override;

	unsigned int waitcycles32() const override { return 2; }
	unsigned int waitcycles16() const override { return 1; }
	unsigned int waitcycles8() const override { return 1; }
};
