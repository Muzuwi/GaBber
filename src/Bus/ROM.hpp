#pragma once
#include <vector>
#include "Bus/Common/BusDevice.hpp"
#include "Emulator/StdTypes.hpp"

class ROM final : public BusDevice {
	std::vector<uint8> m_rom {};

	static constexpr inline uint32 mirror(uint32 offset) { return offset % 0x02000000; }
public:
	ROM()
	    : BusDevice(0x08000000, 0x0e000000)
	    , m_rom() {}

	void from_vec(std::vector<uint8>&& vec);

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;
	void write8(uint32, uint8) override;
	void write16(uint32, uint16) override;
	void write32(uint32, uint32) override;

	unsigned int waitcycles32() const override { return 8; }
	unsigned int waitcycles16() const override { return 5; }
	unsigned int waitcycles8() const override { return 5; }
};