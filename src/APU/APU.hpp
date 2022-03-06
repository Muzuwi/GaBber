#pragma once
#include <array>
#include <deque>
#include <SDL_audio.h>
#include "APU/FIFOA.hpp"
#include "APU/FIFOB.hpp"
#include "APU/Noise.hpp"
#include "APU/SquareSweep.hpp"
#include "APU/SquareTone.hpp"
#include "APU/Wave.hpp"
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

class APU : Module {
	friend class SoundCtlX;
	friend class Sound2CtlL;
	friend class Sound1CtlH;
	static constexpr const unsigned psg_sample_rate = 262144;
	static constexpr const unsigned output_sample_rate = 48000;
	static constexpr const double requested_latency = 0.02;//  in seconds

	static constexpr const unsigned psg_sample_count = (unsigned)(psg_sample_rate * requested_latency) * 2;
	static constexpr const unsigned output_sample_count = (unsigned)(output_sample_rate * requested_latency) * 2;
	static_assert(psg_sample_count % 2 == 0, "Invalid PSG sample count. Should be a multiple of 2 (L+R).");
	static_assert(output_sample_count % 2 == 0, "Invalid output sample count. Should be a multiple of 2 (L+R).");

	SDL_AudioSpec m_device_spec;
	SDL_AudioDeviceID m_device;

	std::array<float, psg_sample_count> m_internal_samples;
	unsigned m_current_sample;

	uint64 m_cycles;
	SquareSweep m_square1;
	SquareTone m_square2;
	Wave m_wave;
	Noise m_noise;
	FIFOA m_fifo_a;
	FIFOB m_fifo_b;

	void push_samples(float left, float right);
public:
	APU(GaBber&);
	void initialize_platform();
	void cycle();

	std::array<float, psg_sample_count> const& internal_samples() const { return m_internal_samples; }

	void on_timer_overflow(unsigned timer_num);

	bool switch_audio_device(char const*);

	SquareSweep& square1() { return m_square1; }
	SquareTone& square2() { return m_square2; }
	Wave& wave() { return m_wave; }
	Noise& noise() { return m_noise; }
	FIFOA& fifo_a() { return m_fifo_a; }
	FIFOB& fifo_b() { return m_fifo_b; }

	float audio_latency() const;
};