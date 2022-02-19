#include "Bus/PakSRAM.hpp"
#include <fmt/format.h>

void PakSRAM::set_cart(std::unique_ptr<BackupCart>&& cart) {
	m_cart = std::move(cart);
}

uint8 PakSRAM::read8(uint32 offset) {
	offset = mirror(offset);
	if(!m_cart) {
		//  TODO: No cart?
		return 0xFF;
	}

	return m_cart->read(offset);
}

void PakSRAM::write8(uint32 offset, uint8 value) {
	offset = mirror(offset);
	if(!m_cart) {
		//  TODO: No cart?
		return;
	}

	m_cart->write(offset, value);
}

uint16 PakSRAM::read16(uint32 offset) {
	offset = mirror(offset);
	fmt::print("PakSRAM/ 16-bit read from offset {:x}\n", offset);
	return read8(offset) * 0x0101u;
}

uint32 PakSRAM::read32(uint32 offset) {
	offset = mirror(offset);
	fmt::print("PakSRAM/ 32-bit read from offset {:x}\n", offset);
	return read8(offset) * 0x01010101u;
}

void PakSRAM::write16(uint32 offset, uint16 value) {
	offset = mirror(offset);
	const uint8 byte = (value >> ((offset % 2) * 8)) & 0xFFu;
	write8(offset, byte);
	fmt::print("PakSRAM/ 16-bit write of value {:4x} to offset {:x} [written byte={:02x}]\n", value, offset, byte);
}

void PakSRAM::write32(uint32 offset, uint32 value) {
	offset = mirror(offset);
	const uint8 byte = (value >> ((offset % 4) * 8)) & 0xFFu;
	write8(offset, byte);
	fmt::print("PakSRAM/ 32-bit write of value {:8x} to offset {:x} [written byte={:02x}]\n", value, offset, byte);
}

void PakSRAM::reload() {
	//  TODO: Implement
}
