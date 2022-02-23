#pragma once
#include "Emulator/Module.hpp"

class Noise : Module {
	bool m_running { false };
	unsigned m_cycles {};

	unsigned m_length_counter {};
	unsigned m_envelope_counter {};
	unsigned m_volume_counter {};
	unsigned m_rate_counter { 0 };

	uint16 m_lfsr {};
	bool m_output {};

	void reload_rate();
	void reload_length();
	void reload_state();

	void advance_state();
public:
	Noise(GaBber& emu)
	    : Module(emu) {}

	bool running() const { return m_running; }
	void tick();
	int16 generate_sample() const;
	void trigger();
	void reload_envelope();
};