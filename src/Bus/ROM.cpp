#include "Bus/ROM.hpp"

void ROM::from_vec(std::vector<uint8>&& vec) {
	m_rom = vec;
	if(m_rom.size() > 32 * MB) {
		m_rom.resize(32 * MB);
	}
}

uint8 ROM::read8(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_rom.size()) {
		//  FIXME: unreadable I/O register
		return 0xBA;
	}
	return m_rom[offset];
}

uint16 ROM::read16(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_rom.size() || offset + 1 >= m_rom.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABE;
	}
	return *reinterpret_cast<uint16*>(&m_rom[0] + offset);
}

uint32 ROM::read32(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_rom.size() || offset + 3 >= m_rom.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
	return *reinterpret_cast<uint32*>(&m_rom[0] + offset);
}

void ROM::write8(uint32, uint8) {}

void ROM::write16(uint32, uint16) {}

void ROM::write32(uint32, uint32) {}
