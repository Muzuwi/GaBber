#include "Bus/OAM.hpp"
#include <cstring>

uint8 OAM::read8(uint32 offset) {
	offset = mirror(offset);
	return m_oam.read8(offset);
}

uint16 OAM::read16(uint32 offset) {
	offset = mirror(offset);
	return m_oam.read16(offset);
}

uint32 OAM::read32(uint32 offset) {
	offset = mirror(offset);
	return m_oam.read32(offset);
}

void OAM::write8(uint32, uint8) {
	//  8-bit writes are ignored
}

void OAM::write16(uint32 offset, uint16 value) {
	offset = mirror(offset);
	m_oam.write16(offset, value);
}

void OAM::write32(uint32 offset, uint32 value) {
	offset = mirror(offset);
	m_oam.write32(offset, value);
}

void OAM::reload() {
	std::memset(&m_oam.array()[0], 0x0, m_oam.size());
}
