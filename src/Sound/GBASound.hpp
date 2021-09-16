#pragma once
#include <SDL2/SDL_audio.h>
#include "Headers/StdTypes.hpp"

class GBASound {
	SDL_AudioSpec m_device_spec;
	SDL_AudioDeviceID m_device;
	Array<float, 262144/4> m_internal_samples;
//	Array<float, 4096> m_internal_samples;
	unsigned m_current_sample;
	uint64 m_cycles;
	SDL_AudioCVT m_out_converter;

	unsigned m_last_freq;
	float m_ch2_phi {0.0f};
public:
	void init();
	void cycle();

//	Array<float, 4096> const& internal_samples() const {
//		return m_internal_samples;
//	}

	Array<float, 262144/4> const& internal_samples() const {
		return m_internal_samples;
	}
};