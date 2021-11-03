#pragma once
#include <memory>
#include "Headers/StdTypes.hpp"
#include "MMU/BusDevice.hpp"
#include "Bus/SRAM/BackupCart.hpp"

class SRAM final : public BusDevice {
	std::unique_ptr<BackupCart> m_cart;

	static inline uint32 mirror(uint32 offset) {
		return offset & 0xFFFFu;
	}
public:
	SRAM() : BusDevice(0x0e000000, 0x10000000) {}

	void set_cart(std::unique_ptr<BackupCart>&& cart) {
		m_cart = std::move(cart);
	}

	//  Holy shit this is disgusting
	std::unique_ptr<BackupCart> const& cart() const {
		return m_cart;
	}

	uint8 read8(uint32 offset) override {
		offset = mirror(offset);
		if(!m_cart) {
			//  TODO: No cart?
			return 0xFF;
		}

		return m_cart->read(offset);
	}

	void write8(uint32 offset, uint8 value) override {
		offset = mirror(offset);
		if(!m_cart) {
			//  TODO: No cart?
			return;
		}

		m_cart->write(offset, value);
	}

	uint16 read16(uint32 offset) override {
		offset = mirror(offset);
		fmt::print("PakSRAM/ 16-bit read from offset {:x}\n", offset);
		return read8(offset) * 0x0101u;
	}
	uint32 read32(uint32 offset) override {
		offset = mirror(offset);
		fmt::print("PakSRAM/ 32-bit read from offset {:x}\n", offset);
		return read8(offset) * 0x01010101u;
	}

	void write16(uint32 offset, uint16 value) override {
		offset = mirror(offset);
		fmt::print("PakSRAM/ 16-bit write of value {:4x} to offset {:x}\n", value, offset);
	}
	void write32(uint32 offset, uint32 value) override {
		offset = mirror(offset);
		fmt::print("PakSRAM/ 32-bit write of value {:8x} to offset {:x}\n", value, offset);
	}

	void reload() override { }

	unsigned int waitcycles32() const override {
		return 5;
	}

	unsigned int waitcycles16() const override {
		return 5;
	}

	unsigned int waitcycles8() const override {
		return 5;
	}
};