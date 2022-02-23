#pragma once
#include "Emulator/Module.hpp"

class SquareSweep : Module {
	bool m_running { false };
	unsigned m_cycles {};

	unsigned m_cycle_frequency {};
	unsigned m_frequency_counter {};
	unsigned m_sweep_counter {};

	unsigned m_length_counter {};
	unsigned m_envelope_counter {};
	unsigned m_volume_counter {};

	void reload_length();
	void reload_frequency();

	static constexpr unsigned samples_per_frequency_cycle(unsigned frequency) {
		const unsigned sample_rate = 262144;//  in Hz
		const auto samples_per_period = (unsigned)(((float)sample_rate / (float)frequency));
		return samples_per_period;
	}
public:
	SquareSweep(GaBber& emu)
	    : Module(emu) {}

	void reload_sweep();
	void reload_envelope();
	bool running() const { return m_running; }
	void trigger();
	void tick();
	int16 generate_sample();
};