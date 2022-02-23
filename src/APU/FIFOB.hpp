#pragma once
#include <deque>
#include "Emulator/Module.hpp"

class FIFOB : Module {
	std::deque<uint8> m_audio_samples;
	std::deque<uint8> m_raw_queue;
	void push_sample(uint8 sample, unsigned sample_rate);
public:
	FIFOB(GaBber& emu)
	    : Module(emu) {}

	int16 generate_sample();
	void push_raw(uint8 sample);
	void move_data_to_sound_circuit(unsigned sample_rate);
	void clear_raw();
};