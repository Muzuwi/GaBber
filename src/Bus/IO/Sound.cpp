#include "Sound.hpp"
#include <fmt/format.h>
#include "APU/APU.hpp"
#include "Bus/Common/MemoryLayout.hpp"
#include "Emulator/GaBber.hpp"

/*
 *  DMG sound output control
 */

uint16 SoundCtlL::on_read() {
	return this->m_register & 0xFF77u;
}

void SoundCtlL::on_write(unsigned short new_value) {
	this->m_register = new_value & 0xFF77u;
}

/*
 *  Direct sound output control
 */

uint16 SoundCtlH::on_read() {
	return m_register & readable_mask;
}

void SoundCtlH::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;
	if(new_value & (1u << 11u)) {
		apu().fifo_a().clear_raw();
	}
	if(new_value & (1u << 15u)) {
		apu().fifo_b().clear_raw();
	}
}

/*
 *  Master sound output control/status
 */

uint32 SoundCtlX::on_read() {
	// clang-format off
	return (m_register & readable_mask)
	       | (apu().square1().running() ? 0b0001 : 0)
	       | (apu().square2().running() ? 0b0010 : 0)
	       | (apu().wave().running()    ? 0b0100 : 0)
	       | (apu().noise().running()   ? 0b1000 : 0);
	// clang-format on
}

void SoundCtlX::on_write(uint32 new_value) {
	if(!(new_value & (1u << 7u))) {
		//  TODO: PSG/FIFO reset
		fmt::print("Sound/ Unimplemented: PSG/FIFO Reset\n");
	}
	m_register = new_value & writeable_mask;
}

/*
 *  Sound bias
 */

uint32 SoundBias::on_read() {
	return m_register & readable_mask;
}

void SoundBias::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
}

/*
 *  DMG channel 1 sweep control
 */

uint16 Sound1CtlL::on_read() {
	return this->m_register & ~0xFF80u;
}

void Sound1CtlL::on_write(uint16 new_value) {
	this->m_register = new_value & ~0xFF80u;
}

/*
 *  DMG channel 1 length, wave duty, envelope
 */

uint16 Sound1CtlH::on_read() {
	return this->m_register & ~0x3F;
}

void Sound1CtlH::on_write(uint16 new_value) {
	this->m_register = new_value;
	apu().square1().reload_envelope();
}

/*
 *  DMG channel 1 frequency, reset, loop control
 */

uint32 Sound1CtlX::on_read() {
	return m_register & 0x4000u;
}

void Sound1CtlX::on_write(uint32 new_value) {
	m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		apu().square1().trigger();
	}
}

/*
 *  DMG channel 2 length, wave duty, envelope
 */

uint32 Sound2CtlL::on_read() {
	return this->m_register & 0x0000FFC0;
}

void Sound2CtlL::on_write(uint32 new_value) {
	this->m_register = new_value & 0x0000FFFF;
	apu().square2().reload_envelope();
}

/*
 *  DMG channel 2 frequency, reset, loop control
 */

uint32 Sound2CtlH::on_read() {
	return m_register & 0x4000u;
}

void Sound2CtlH::on_write(uint32 new_value) {
	this->m_register = new_value & 0xC7FFu;
	if(new_value & (1u << 15u)) {
		apu().square2().trigger();
	}
}

/*
 *  DMG channel 3 enable, wave RAM bank control
 */

uint16 Sound3CtlL::on_read() {
	return m_register & readable_mask;
}

void Sound3CtlL::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;

	//  Start or stop playback
	if(new_value & (1u << 7u)) {
		apu().wave().resume();
	} else {
		apu().wave().pause();
	}
}

/*
 *  DMG channel 3 length, output level control
 */

uint16 Sound3CtlH::on_read() {
	return m_register & readable_mask;
}

void Sound3CtlH::on_write(uint16 new_value) {
	m_register = new_value & writeable_mask;
}

/*
 *  DMG channel 3 frequency, reset, loop control
 */

uint32 Sound3CtlX::on_read() {
	return m_register & readable_mask;
}

void Sound3CtlX::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
	if(new_value & (1u << 15u)) {
		apu().wave().trigger();
	}
}

/*
 *  DMG channel 3 wave RAM
 */

ReaderArray<16>& Sound3Bank::current_bank() {
	return io().ch3ctlL->bank ? m_bank0 : m_bank1;
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

/*
 *  DMG channel 4 length, output level, envelope control
 */

uint32 Sound4CtlL::on_read() {
	return m_register & readable_mask;
}

void Sound4CtlL::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
	apu().noise().reload_envelope();
}

/*
 *  DMG channel 4 noise parameters, reset, loop control
 */

uint32 Sound4CtlH::on_read() {
	return m_register & readable_mask;
}

void Sound4CtlH::on_write(uint32 new_value) {
	m_register = new_value & writeable_mask;
	if(new_value & (1u << 15u)) {
		apu().noise().trigger();
	}
}

uint32 SoundFifoA::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABEBABE;
}

void SoundFifoA::on_write(uint32 new_value) {
	auto add_sample = [this](uint8 sample) { apu().fifo_a().push_raw(sample); };

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
	auto add_sample = [this](uint8 sample) { apu().fifo_b().push_raw(sample); };

	add_sample(Bits::byte_le<0>(new_value));
	add_sample(Bits::byte_le<1>(new_value));
	add_sample(Bits::byte_le<2>(new_value));
	add_sample(Bits::byte_le<3>(new_value));
}
