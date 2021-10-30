#include "Headers/GaBber.hpp"
#include "IO/Sound.hpp"

uint32 SoundCtlX::on_read() {
	auto& gbasound = GaBber::instance().sound();
	return (m_register & readable_mask)
		   | (gbasound.m_square1.running ? 0b0001 : 0)
	       | (gbasound.m_square2.running ? 0b0010 : 0)
	       | (0)    //  FIXME: Implement
	       | (gbasound.m_wave.running ? 0b1000 : 0);
}

void SoundCtlX::on_write(uint32 new_value) {
	if(!(new_value & (1u << 7u))) {
		//  TODO: PSG/FIFO reset
	}
	m_register = new_value & writeable_mask;
}

void Sound1CtlH::on_write(uint16 new_value) {
	this->m_register = new_value;
	GaBber::instance().sound().m_square1.length_counter = 64 - (m_register & 0b1111u);
}

void Sound1CtlX::on_write(uint32 new_value) {
	this->m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_square1();
	}
}

void Sound2CtlL::on_write(uint32 new_value) {
	this->m_register = new_value & 0x0000FFFF;
	GaBber::instance().sound().m_square2.length_counter = 64 - (m_register & 0b1111u);
}

void Sound2CtlH::on_write(uint32 new_value) {
	this->m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_square2();
	}
}

void Sound3CtlX::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_wave();
	}
}

Array<uint8, 16>& Sound3Bank::current_bank() {
	return GaBber::instance().mem().io.ch3ctlL->bank ? m_bank0 : m_bank1;
}

void Sound3CtlL::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;

	//  Start or stop playback
	if(new_value & (1u << 7u)) {
		GaBber::instance().sound().set_wave_running(true);
	} else {
		GaBber::instance().sound().set_wave_running(false);
	}
}

void SoundCtlH::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;
	if((*this)->_resetA) {
		GaBber::instance().mem().io.fifoA.fifo().clear();
	}
	if((*this)->_resetB) {
		GaBber::instance().mem().io.fifoB.fifo().clear();
	}
}
