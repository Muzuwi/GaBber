#include "SRAM.hpp"

uint8 SRAM::read(uint32 offset) {
	offset = offset % 32768;

	//  FIXME: OOB read from SRAM
	if(offset >= m_sram.size()) {
		return 0xFF;
	}
	return m_sram[offset];
}

void SRAM::write(uint32 offset, uint8 value) {
	offset = offset % 32768;

	if(offset >= m_sram.size()) {
		return;
	}
	m_sram[offset] = value;
}

void SRAM::from_vec(std::vector<uint8>&& vector) {
	m_sram = vector;
	m_sram.resize(32768);
}

std::vector<uint8> const& SRAM::to_vec() {
	return m_sram;
}
