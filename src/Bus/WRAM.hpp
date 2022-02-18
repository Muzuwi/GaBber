#pragma once
#include "Bus/Common/BusDevice.hpp"
#include "Bus/Common/ReaderArray.hpp"
#include "Emulator/StdTypes.hpp"

class WRAM final : public BusDevice {
	ReaderArray<256 * kB> m_wram;

	static constexpr inline uint32 mirror(uint32 address) { return address & 0x3ffffu; }
public:
	WRAM()
	    : BusDevice(0x02000000, 0x03000000)
	    , m_wram() {}

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32 offset, uint8 value) override;
	void write16(uint32 offset, uint16 value) override;
	void write32(uint32 offset, uint32 value) override;

	void reload() override;

	unsigned int waitcycles32() const override { return 6; }
	unsigned int waitcycles16() const override { return 3; }
	unsigned int waitcycles8() const override { return 3; }
};
