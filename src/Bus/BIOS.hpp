#pragma once
#include <vector>
#include "Bus/Common/BusDevice.hpp"
#include "Bus/Common/ReaderArray.hpp"
#include "Emulator/StdTypes.hpp"

class BIOS final : public BusDevice {
	ReaderArray<0x4000> m_bios;
public:
	BIOS(GaBber& emu)
	    : BusDevice(emu, 0x00000000, 0x00004000)
	    , m_bios() {}

	void from_vec(std::vector<uint8> const& vec);

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32, uint8) override;
	void write16(uint32, uint16) override;
	void write32(uint32, uint32) override;

	void reload() override;

	unsigned int waitcycles32() const override { return 1; }
	unsigned int waitcycles16() const override { return 1; }
	unsigned int waitcycles8() const override { return 1; }
};