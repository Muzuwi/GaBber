#include "Wave.hpp"
#include "Bus/IO/IOContainer.hpp"

void Wave::tick() {
	m_cycles++;

	if(!m_running) {
		return;
	}

	if((m_cycles % 65536 == 0) && m_length_counter != 0) {
		m_length_counter--;
		if(m_length_counter == 0) {
			m_running = false;
		}
	}

	if(m_rate_cycles != 0) {
		m_rate_cycles--;
		return;
	}

	//  Reload the cycle counter with the amount of CPU cycles
	//  for the current sample rate
	reload_frequency();

	//  Consume 1 digit
	if(io().ch3ctlL->dimension) {
		m_current_digit = (m_current_digit + 1) % 64;
	} else {
		m_current_digit = (m_current_digit + 1) % 32;
	}
}

int16 Wave::generate_sample() {
	if(!m_running) {
		return 0;
	}

	const unsigned which_bank = ((unsigned)io().ch3ctlL->bank + (m_current_digit / 32)) % 2;
	const unsigned which_digit = m_current_digit % 32;
	auto const& bank = (which_bank == 0) ? io().ch3bank.bank0() : io().ch3bank.bank1();
	const unsigned byte = which_digit / 2;
	const bool upper = (which_digit % 2) == 0;

	uint8 digit;
	if(upper) {
		digit = (bank.read8(byte) >> 4u) & 0x0Fu;
	} else {
		digit = bank.read8(byte) & 0x0Fu;
	}

	if(io().ch3ctlH->force_volume) {
		return (int16)((digit * 3) / 4);
	}

	const unsigned shift = (io().ch3ctlH->volume == 0) ? 4 : (io().ch3ctlH->volume - 1);
	return (int16)(digit >> shift);
}

void Wave::trigger() {
	reload_frequency();
	reload_length();
	m_current_digit = 0;
	m_running = true;
}

void Wave::reload_frequency() {
	m_frequency = 2097152 / (2048 - io().ch3ctlX->rate);
	m_rate_cycles = 16 * kB * kB / m_frequency;
}

void Wave::reload_length() {
	if(io().ch3ctlX->length_flag) {
		m_length_counter = 256 - io().ch3ctlH->length;
	} else {
		m_length_counter = 0;
	}
}
