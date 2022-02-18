#include "Bus/VRAM.hpp"
#include <cstring>

uint8 VRAM::read8(uint32 offset) {
	offset = offset_in_mirror(offset);
	return m_vram.read8(offset);
}

uint16 VRAM::read16(uint32 offset) {
	offset = offset_in_mirror(offset);
	return m_vram.read16(offset);
}

uint32 VRAM::read32(uint32 offset) {
	offset = offset_in_mirror(offset);
	return m_vram.read32(offset);
}

void VRAM::write8(uint32 offset, uint8 value) {
	offset = offset_in_mirror(offset) & ~1u;

	//  8-bit writes ignored to OBJ
	//  TODO: Also ignore writes to 0x0014000-0x0017FFF when in bitmap modes
	if(offset >= 0x0010000 && offset <= 0x0017FFF) {
		return;
	}

	//  8-bit value is written to both the upper and lower 8-bits
	//  of the nearest half-word
	m_vram.write8(offset, value);
	m_vram.write8(offset + 1, value);
}

void VRAM::write16(uint32 offset, uint16 value) {
	offset = offset_in_mirror(offset);
	m_vram.write16(offset, value);
}

void VRAM::write32(uint32 offset, uint32 value) {
	offset = offset_in_mirror(offset);
	m_vram.write32(offset, value);
}

void VRAM::reload() {
	std::memset(&m_vram.array()[0], 0x0, m_vram.size());
}
