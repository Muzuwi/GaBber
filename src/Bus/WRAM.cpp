#include "Bus/WRAM.hpp"
#include <cstring>

uint8 WRAM::read8(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_wram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBA;
	}
	return m_wram.read8(offset);
}

uint16 WRAM::read16(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_wram.size() || offset + 1 >= m_wram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABE;
	}
	return m_wram.read16(offset);
}

uint32 WRAM::read32(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_wram.size() || offset + 3 >= m_wram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
	return m_wram.read32(offset);
}

void WRAM::write8(uint32 offset, uint8 value) {
	offset = mirror(offset);

	if(offset >= m_wram.size()) {
		return;
	}
	m_wram.write8(offset, value);
}

void WRAM::write16(uint32 offset, uint16 value) {
	offset = mirror(offset);

	if(offset >= m_wram.size() || offset + 1 >= m_wram.size()) {
		return;
	}
	m_wram.write16(offset, value);
}

void WRAM::write32(uint32 offset, uint32 value) {
	offset = mirror(offset);

	if(offset >= m_wram.size() || offset + 3 >= m_wram.size()) {
		return;
	}
	m_wram.write32(offset, value);
}

void WRAM::reload() {
	std::memset(&m_wram.array()[0], 0x0, m_wram.size());
}
