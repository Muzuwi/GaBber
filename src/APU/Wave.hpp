#pragma once
#include "Emulator/Module.hpp"

class Wave : Module {
	unsigned m_cycles {};
	unsigned m_rate_cycles {};
	bool m_running { false };
	unsigned m_frequency {};
	unsigned m_length_counter {};
	unsigned m_current_digit {};
public:
	Wave(GaBber& emu)
	    : Module(emu) {}
	~Wave() = default;

	bool running() const { return m_running; }
	void pause() { m_running = false; }
	void resume() { m_running = true; }

	void tick();
	int16 generate_sample();
	void trigger();
	void reload_length();
	void reload_frequency();
};