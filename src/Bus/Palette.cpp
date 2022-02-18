#include "Bus/Palette.hpp"
#include <cstring>

uint8 Palette::read8(uint32 offset) {
	offset = mirror(offset);
	return m_palette.read8(offset);
}

uint16 Palette::read16(uint32 offset) {
	offset = mirror(offset);
	return m_palette.read16(offset);
}

uint32 Palette::read32(uint32 offset) {
	offset = mirror(offset);
	return m_palette.read32(offset);
}

void Palette::write8(uint32 offset, uint8 value) {
	offset = mirror(offset) & ~1u;

	//  8-bit value is written to both the upper and lower 8-bits
	//  of the nearest half-word
	m_palette.write8(offset, value);
	m_palette.write8(offset + 1, value);
}

void Palette::write16(uint32 offset, uint16 value) {
	offset = mirror(offset);
	m_palette.write16(offset, value);
}

void Palette::write32(uint32 offset, uint32 value) {
	offset = mirror(offset);
	m_palette.write32(offset, value);
}

void Palette::reload() {
	std::memset(&m_palette.array()[0], 0x0, m_palette.size());
}
