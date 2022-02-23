#include "SquareSweep.hpp"
#include "Bus/IO/IOContainer.hpp"
#include "Bus/IO/Sound.hpp"

void SquareSweep::tick() {
	m_cycles++;

	//  One sample every 64 ticks
	if((m_cycles % 64 == 0) && m_frequency_counter != 0) {
		m_frequency_counter--;
		if(m_frequency_counter == 0) {
			reload_frequency();
		}
	}

	if((m_cycles % 131072) == 0 && m_sweep_counter != 0) {
		m_sweep_counter--;
		if(m_sweep_counter == 0) {
			m_sweep_counter = io().ch1ctlL->sweep_time;

			const auto old_frequency = io().ch1ctlX->frequency;
			const auto change = old_frequency / (1 << io().ch1ctlL->sweep_shift);
			if(io().ch1ctlL->sweep_decreases) {
				if(old_frequency < change) {
					m_sweep_counter = 0;
				} else {
					io().ch1ctlX->frequency -= change;
				}
			} else {
				if(static_cast<uint32>(old_frequency) + static_cast<uint32>(change) >= 2048) {
					m_sweep_counter = 0;
					m_running = false;
				} else {
					io().ch1ctlX->frequency += change;
				}
			}
		}
	}

	if((m_cycles % 65536 == 0) && m_length_counter != 0) {
		m_length_counter--;
		if(m_length_counter == 0) {
			m_running = false;
		}
	}

	//  Every 1/64 seconds
	if((m_cycles % 262144 == 0) && io().ch1ctlH->envelope_step != 0) {
		if(m_envelope_counter != 0) {
			m_envelope_counter--;
		} else {
			m_envelope_counter = io().ch1ctlH->envelope_step;

			if(io().ch1ctlH->envelope_inc && m_volume_counter < 15) {
				m_volume_counter++;
			} else if(!io().ch1ctlH->envelope_inc && m_volume_counter > 0) {
				m_volume_counter--;
			}
		}
	}
}

int16 SquareSweep::generate_sample() {
	static constexpr const float duty_lookup[4] = { 0.125f, 0.25f, 0.5f, 0.75f };
	if(!m_running) {
		return 0;
	}

	const unsigned freq = m_cycle_frequency;
	const auto samples_per_period = samples_per_frequency_cycle(freq);
	const unsigned sample_number = samples_per_period - m_frequency_counter;

	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);
	const float duty = duty_lookup[io().ch1ctlH->duty];
	const auto envelope = static_cast<int16>(m_volume_counter);

	const int16 isample = (ft >= duty) ? (int16)0 : (int16)(envelope);
	return isample;
}

void SquareSweep::trigger() {
	reload_length();
	reload_frequency();
	reload_envelope();
	reload_sweep();
	m_running = true;
}

void SquareSweep::reload_frequency() {
	const unsigned freq = 131072 / (2048 - io().ch1ctlX->frequency);
	m_cycle_frequency = freq;
	m_frequency_counter = samples_per_frequency_cycle(freq);
}

void SquareSweep::reload_envelope() {
	m_volume_counter = io().ch1ctlH->envelope_vol;
	m_envelope_counter = io().ch1ctlH->envelope_step;
}

void SquareSweep::reload_length() {
	if(io().ch1ctlX->length_flag) {
		m_length_counter = 64 - io().ch1ctlH->length;
	} else {
		m_length_counter = 0;
	}
}

void SquareSweep::reload_sweep() {
	m_sweep_counter = io().ch1ctlL->sweep_time;
}
