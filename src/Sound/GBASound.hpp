#pragma once
#include <SDL2/SDL_audio.h>
#include "Headers/StdTypes.hpp"

class GBASound {
	static constexpr const unsigned psg_sample_rate = 262144;
	static constexpr const unsigned output_sample_rate = 48000;
	static constexpr const double requested_latency = 0.02;     //  in seconds

	static constexpr const unsigned psg_sample_count = (unsigned)(psg_sample_rate * requested_latency);
	static constexpr const unsigned output_sample_count = (unsigned)(output_sample_rate * requested_latency);

	SDL_AudioSpec m_device_spec;
	SDL_AudioDeviceID m_device;
	Array<float, psg_sample_count> m_internal_samples;

	unsigned m_current_sample;
	uint64 m_cycles;
	SDL_AudioCVT m_out_converter;

	float generate_sample_ch1();
	float generate_sample_ch2();
	float generate_sample_ch3();
	float generate_sample_ch4();
public:
	void init();
	void cycle();

	Array<float, psg_sample_count> const& internal_samples() const {
		return m_internal_samples;
	}

	int m_speed_scale = 1;
};