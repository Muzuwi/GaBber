#include "Interrupt.hpp"

uint16 IE::on_read() {
	return m_register & ~0xc000;
}

void IE::on_write(uint16 val) {
	m_register = (val & ~0xc000);
}

uint16 IF::on_read() {
	return m_register & ~0xc000;
}

void IF::on_write(uint16 val) {
	m_register &= ~val;
}

uint32 IME::on_read() {
	return this->m_register & 1u;
}

void IME::on_write(uint32 new_value) {
	this->m_register = new_value & 1u;
}

void HALTCNT::on_write(uint8 val) {
	//  Halt
	if(val == 0) {
		m_halt = true;
	} else if(val == 0x80)
		m_stop = true;
}

uint32 WaitControl::on_read() {
	return m_register & readable_mask;
}

void WaitControl::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}

uint32 MemCtl::on_read() {
	return m_register & readable_mask;
}

void MemCtl::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}
