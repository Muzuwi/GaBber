#pragma once
#include <memory>
#include "Bus/Common/BusDevice.hpp"
#include "Bus/SRAM/BackupCart.hpp"
#include "Emulator/StdTypes.hpp"

class SRAM final : public BusDevice {
	std::unique_ptr<BackupCart> m_cart;

	static constexpr inline uint32 mirror(uint32 offset) { return offset & 0xFFFFu; }
public:
	SRAM()
	    : BusDevice(0x0e000000, 0x10000000) {}

	void set_cart(std::unique_ptr<BackupCart>&& cart);

	BackupCart* cart() const { return m_cart.get(); }

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32 offset, uint8 value) override;
	void write16(uint32 offset, uint16 value) override;
	void write32(uint32 offset, uint32 value) override;

	void reload() override;

	unsigned int waitcycles32() const override { return 5; }
	unsigned int waitcycles16() const override { return 5; }
	unsigned int waitcycles8() const override { return 5; }
};
