#include "DebugBackdoor.hpp"
#include <fmt/format.h>

uint8 DebugBackdoor::read8(uint32 offset) {
	return m_buffer.read8(offset);
}

uint16 DebugBackdoor::read16(uint32 offset) {
	return m_buffer.read16(offset);
}

uint32 DebugBackdoor::read32(uint32 offset) {
	return m_buffer.read32(offset);
}

void DebugBackdoor::write8(uint32 offset, uint8 value) {
	m_buffer.write8(offset, value);
}

void DebugBackdoor::write16(uint32 offset, uint16 value) {
	m_buffer.write16(offset, value);
}

void DebugBackdoor::write32(uint32 offset, uint32 value) {
	m_buffer.write32(offset, value);
}

void DebugBackdoor::reload() {
	std::fill_n(m_buffer.array().begin(), bufsize, 0x0);
}

std::string DebugBackdoor::to_string() {
	std::string str;
	for(unsigned i = 0; i < 0x60; ++i) {
		if(m_buffer.array()[i] == '\0')
			break;
		str.push_back(m_buffer.array()[i]);
	}
	return str;
}

void Backdoor::on_write(uint16) {
	fmt::print("Backdoor: \u001b[96m{}", m_backdoor.to_string());
	fmt::print("\u001b[0m\n");
}
