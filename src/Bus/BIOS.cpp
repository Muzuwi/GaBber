#include "Bus/BIOS.hpp"
#include <cstring>

void BIOS::from_vec(std::vector<uint8> const& vec) {
	assert(vec.size() == 0x4000);
	std::memcpy(&m_bios.array()[0], &vec[0], 0x4000);
}

uint8 BIOS::read8(uint32 offset) {
	if(offset >= m_bios.size()) {
		//  FIXME: unreadable I/O register
		return 0xBA;
	}
	return m_bios.read8(offset);
}

uint16 BIOS::read16(uint32 offset) {
	if(offset >= m_bios.size() || offset + 1 >= m_bios.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABE;
	}
	return m_bios.read16(offset);
}

uint32 BIOS::read32(uint32 offset) {
	if(offset >= m_bios.size() || offset + 3 >= m_bios.size()) {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
	return m_bios.read32(offset);
}

void BIOS::write8(uint32, uint8) {}

void BIOS::write16(uint32, uint16) {}

void BIOS::write32(uint32, uint32) {}

void BIOS::reload() {
	BusDevice::reload();
}
