#include "Noise.hpp"
#include <fmt/format.h>
#include "Bus/IO/IOContainer.hpp"
#include "Bus/IO/Sound.hpp"

void Noise::tick() {
	m_cycles++;

	if(m_rate_counter != 0) {
		--m_rate_counter;
		if(m_rate_counter == 0) {
			advance_state();
			reload_rate();
		}
	}

	if((m_cycles % 65536 == 0) && m_length_counter != 0) {
		m_length_counter--;
		if(m_length_counter == 0) {
			m_running = false;
		}
	}

	if((m_cycles % 262144 == 0) && io().ch4ctlL->envelope_step != 0) {
		if(m_envelope_counter != 0) {
			m_envelope_counter--;
		} else {
			m_envelope_counter = io().ch4ctlL->envelope_step;

			if(io().ch4ctlL->envelope_inc && m_volume_counter < 15) {
				m_volume_counter++;
			} else if(!io().ch4ctlL->envelope_inc && m_volume_counter > 0) {
				m_volume_counter--;
			}
		}
	}
}

int16 Noise::generate_sample() const {
	if(!m_running) {
		return 0;
	}

	return m_output ? static_cast<int16>(m_volume_counter) : static_cast<int16>(0);
}

void Noise::trigger() {
	reload_length();
	reload_rate();
	reload_envelope();
	reload_state();
	m_running = true;
}

void Noise::reload_length() {
	if(io().ch4ctlH->length_flag) {
		m_length_counter = 64 - io().ch4ctlL->length;
	} else {
		m_length_counter = 0;
	}
}

void Noise::reload_rate() {
	const auto r = io().ch4ctlH->r;
	const auto s = io().ch4ctlH->s;
	unsigned frequency {};
	if(r == 0) {
		frequency = 2 * 524288 / (1 << (s + 1));
	} else {
		frequency = (524288 / r) / (1 << (s + 1));
	}
	m_rate_counter = 16 * 1024 * 1024 / frequency;
	assert(frequency < 16 * 1024 * 1024);
}

void Noise::reload_envelope() {
	m_volume_counter = io().ch4ctlL->envelope_vol;
	m_envelope_counter = io().ch4ctlL->envelope_step;
}

void Noise::reload_state() {
	if(io().ch4ctlH->counter_7bits) {
		m_lfsr = 0x40u;
	} else {
		m_lfsr = 0x4000u;
	}
}

void Noise::advance_state() {
	if(io().ch4ctlH->counter_7bits) {
		const bool carry = m_lfsr & 1;
		m_lfsr = m_lfsr >> 1u;
		if(carry) {
			m_lfsr ^= 0x60u;
		}
		m_output = carry;
	} else {
		const bool carry = m_lfsr & 1;
		m_lfsr = m_lfsr >> 1u;
		if(carry) {
			m_lfsr ^= 0x6000u;
		}
		m_output = carry;
	}
}
