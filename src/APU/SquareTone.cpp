#include "APU/SquareTone.hpp"
#include "Bus/IO/IOContainer.hpp"
#include "Bus/IO/Sound.hpp"

void SquareTone::tick() {
	m_cycles++;

	//  One sample every 64 ticks
	if((m_cycles % 64 == 0) && m_frequency_counter != 0) {
		m_frequency_counter--;
		if(m_frequency_counter == 0) {
			reload_frequency();
		}
	}

	if((m_cycles % 65536 == 0) && m_length_counter != 0) {
		m_length_counter--;
		if(m_length_counter == 0) {
			m_running = false;
		}
	}

	if((m_cycles % 262144 == 0) && io().ch2ctlL->envelope_step != 0) {
		if(m_envelope_counter != 0) {
			m_envelope_counter--;
		} else {
			m_envelope_counter = io().ch2ctlL->envelope_step;

			if(io().ch2ctlL->envelope_inc && m_volume_counter < 15) {
				m_volume_counter++;
			} else if(!io().ch2ctlL->envelope_inc && m_volume_counter > 0) {
				m_volume_counter--;
			}
		}
	}
}

int16 SquareTone::generate_sample() {
	static constexpr const float duty_lookup[4] = { 0.125f, 0.25f, 0.5f, 0.75f };
	if(!m_running) {
		return 0;
	}

	const unsigned freq = m_cycle_frequency;
	const auto samples_per_period = samples_per_frequency_cycle(freq);
	const unsigned sample_number = samples_per_period - m_frequency_counter;

	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);
	const float duty = duty_lookup[io().ch2ctlL->duty];
	const auto envelope = static_cast<int16>(m_volume_counter);

	const int16 isample = (ft >= duty) ? (int16)0 : (int16)(envelope);
	return isample;
}

void SquareTone::trigger() {
	reload_length();
	reload_frequency();
	reload_envelope();
	m_running = true;
}

void SquareTone::reload_frequency() {
	const unsigned freq = 131072 / (2048 - io().ch2ctlH->frequency);
	m_cycle_frequency = freq;
	m_frequency_counter = samples_per_frequency_cycle(freq);
}

void SquareTone::reload_envelope() {
	m_volume_counter = io().ch2ctlL->envelope_vol;
	m_envelope_counter = io().ch2ctlL->envelope_step;
}

void SquareTone::reload_length() {
	if(io().ch2ctlH->length_flag) {
		m_length_counter = 64 - io().ch2ctlL->length;
	} else {
		m_length_counter = 0;
	}
}
