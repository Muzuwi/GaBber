#include "Bus/IWRAM.hpp"
#include <cstring>

uint8 IWRAM::read8(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_iwram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBA;
	}
	return m_iwram.read8(offset);
}

uint16 IWRAM::read16(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_iwram.size() || offset + 1 >= m_iwram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABE;
	}
	return m_iwram.read16(offset);
}

uint32 IWRAM::read32(uint32 offset) {
	offset = mirror(offset);

	if(offset >= m_iwram.size() || offset + 3 >= m_iwram.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
	return m_iwram.read32(offset);
}

void IWRAM::write8(uint32 offset, uint8 value) {
	offset = mirror(offset);

	if(offset >= m_iwram.size()) {
		return;
	}
	m_iwram.write8(offset, value);
}

void IWRAM::write16(uint32 offset, uint16 value) {
	offset = mirror(offset);

	if(offset >= m_iwram.size() || offset + 1 >= m_iwram.size()) {
		return;
	}
	m_iwram.write16(offset, value);
}

void IWRAM::write32(uint32 offset, uint32 value) {
	offset = mirror(offset);

	if(offset >= m_iwram.size() || offset + 3 >= m_iwram.size()) {
		return;
	}
	m_iwram.write32(offset, value);
}

void IWRAM::reload() {
	std::memset(&m_iwram.array()[0], 0x0, m_iwram.size());
}
