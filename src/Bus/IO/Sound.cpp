#include "Sound.hpp"
#include "APU/APU.hpp"
#include "Bus/Common/MemoryLayout.hpp"
#include "Emulator/GaBber.hpp"

uint16 SoundCtlL::on_read() {
	return this->m_register & 0xFF77u;
}

void SoundCtlL::on_write(unsigned short new_value) {
	this->m_register = new_value & 0xFF77u;
}

uint16 SoundCtlH::on_read() {
	return m_register & readable_mask;
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

uint32 SoundCtlX::on_read() {
	auto& gbasound = GaBber::instance().sound();
	return (m_register & readable_mask) | (gbasound.m_square1.running ? 0b0001 : 0) |
	       (gbasound.m_square2.running ? 0b0010 : 0) | (0)//  FIXME: Implement
	       | (gbasound.m_wave.running ? 0b1000 : 0);
}

void SoundCtlX::on_write(uint32 new_value) {
	if(!(new_value & (1u << 7u))) {
		//  TODO: PSG/FIFO reset
	}
	m_register = new_value & writeable_mask;
}

uint32 SoundBias::on_read() {
	return m_register & readable_mask;
}

void SoundBias::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}

uint16 Sound1CtlL::on_read() {
	return this->m_register & ~0xFF80u;
}

void Sound1CtlL::on_write(uint16 new_value) {
	this->m_register = new_value & ~0xFF80u;
}

uint16 Sound1CtlH::on_read() {
	return this->m_register & ~0x3F;
}

void Sound1CtlH::on_write(uint16 new_value) {
	this->m_register = new_value;
	GaBber::instance().sound().m_square1.length_counter = 64 - (m_register & 0b1111u);
}

uint32 Sound1CtlX::on_read() {
	//  FIXME: What was this?
	return 0x4000;
}

void Sound1CtlX::on_write(uint32 new_value) {
	this->m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_square1();
	}
}

uint32 Sound2CtlL::on_read() {
	return this->m_register & 0x0000FFC0;
}

void Sound2CtlL::on_write(uint32 new_value) {
	this->m_register = new_value & 0x0000FFFF;
	GaBber::instance().sound().m_square2.length_counter = 64 - (m_register & 0b1111u);
}

uint32 Sound2CtlH::on_read() {
	//  FIXME: What was this?
	return 0x4000;
}

void Sound2CtlH::on_write(uint32 new_value) {
	this->m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_square2();
	}
}

uint16 Sound3CtlL::on_read() {
	return m_register & readable_mask;
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

uint16 Sound3CtlH::on_read() {
	return m_register & readable_mask;
}

void Sound3CtlH::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;
}

uint32 Sound3CtlX::on_read() {
	return m_register & readable_mask;
}

void Sound3CtlX::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
	if(new_value & (1u << 15u)) {
		GaBber::instance().sound().reload_wave();
	}
}

ReaderArray<16>& Sound3Bank::current_bank() {
	return GaBber::instance().mem().io.ch3ctlL->bank ? m_bank0 : m_bank1;
}

uint8 Sound3Bank::read8(uint32 offset) {
	return current_bank().read8(offset);
}

uint16 Sound3Bank::read16(uint32 offset) {
	return current_bank().read16(offset);
}

uint32 Sound3Bank::read32(uint32 offset) {
	return current_bank().read32(offset);
}

void Sound3Bank::write8(uint32 offset, uint8 value) {
	current_bank().write8(offset, value);
}

void Sound3Bank::write16(uint32 offset, uint16 value) {
	current_bank().write16(offset, value);
}

void Sound3Bank::write32(uint32 offset, uint32 value) {
	current_bank().write32(offset, value);
}

uint32 Sound4CtlL::on_read() {
	return m_register & readable_mask;
}

void Sound4CtlL::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}

uint32 Sound4CtlH::on_read() {
	return m_register & readable_mask;
}

void Sound4CtlH::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}

uint32 SoundFifoA::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABEBABE;
}

void SoundFifoA::on_write(uint32 new_value) {
	auto add_sample = [this](uint8 sample) { m_fifo.push_back(sample); };

	add_sample(Bits::byte_le<0>(new_value));
	add_sample(Bits::byte_le<1>(new_value));
	add_sample(Bits::byte_le<2>(new_value));
	add_sample(Bits::byte_le<3>(new_value));
}

uint32 SoundFifoB::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABEBABE;
}

void SoundFifoB::on_write(uint32 new_value) {
	auto add_sample = [this](uint8 sample) { m_fifo.push_back(sample); };

	add_sample(Bits::byte_le<0>(new_value));
	add_sample(Bits::byte_le<1>(new_value));
	add_sample(Bits::byte_le<2>(new_value));
	add_sample(Bits::byte_le<3>(new_value));
}
