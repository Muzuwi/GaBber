#include <cmath>
#include "GBASound.hpp"
#include "Headers/GaBber.hpp"

void GBASound::init() {
	SDL_AudioSpec request {};
	std::memset(&request, 0, sizeof(request));

	request.freq = 48000;
	request.format = AUDIO_F32;
	request.channels = 1;
	request.samples = 16384;
	request.callback = nullptr;

	m_device = SDL_OpenAudioDevice(nullptr, 0, &request, &m_device_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if(m_device == 0) {
		fmt::print("Sound/ Failed opening audio device: {}\n", SDL_GetError());
		return;
	}

	fmt::print("Got device spec: samplerate={}, samples={}, channels={}\n",
				m_device_spec.freq, m_device_spec.samples, m_device_spec.channels);

	auto rc = SDL_BuildAudioCVT(&m_out_converter, AUDIO_F32, 1, 262144, AUDIO_F32, 1, 48000);
	if(rc <= 0) {
		fmt::print("Sound/ Failed creating audio converter: {}\n", SDL_GetError());
		return;
	}

	fmt::print("Sound/ Len={}, lenMult={}\n", (int)m_out_converter.len, (int)m_out_converter.len_mult);
	SDL_PauseAudioDevice(m_device, 0);
}

void GBASound::cycle() {
	static std::ofstream debugfile {"output.raw", std::ios::binary};
	static float s_last_phi {0.0f};
	static constexpr const float pi = M_PI;

	//  "The PSG channels 1-4 are internally generated at 262.144kHz"
	//  One internal sample generated every 64 CPU cycles (main clock 16MHz)
	m_cycles++;
	if((m_cycles % 64) != 0)
		return;

	auto& io = GaBber::instance().mem().io;
	auto& ch2 = io.ch2ctlH;

	if(!(io.soundcntL->channel_enable_l & 2) && !(io.soundcntL->channel_enable_r & 2)) {
		return;
	}

	const float volume = 0.2;

	const unsigned sample_rate = 262144;    //  in Hz
	const unsigned sample_number = m_cycles / 64;
	const unsigned freq = 131072u / (2048u - ch2->frequency);
 	const auto samples_per_period = (unsigned)(((float)sample_rate / (float)freq));

	//  Asin(wt + phi)
	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);
	const float wt = 2 * pi * ft;

	if(m_last_freq != freq) {
		const float delta = ft - s_last_phi;
		const float phi = m_ch2_phi + 2 * pi * delta;
		m_ch2_phi = phi;
//		fmt::print("Sound/ Frequency transition {}Hz -> {}Hz\n", m_last_freq, freq);
//		fmt::print("Sound/ ft({}Hz)={}\n", m_last_freq, s_last_phi);
//		fmt::print("Sound/ ft({}Hz)={}\n", freq, ft);
//		fmt::print("Sound/ delta={}\n", delta);
//		fmt::print("Sound/ phi={}\n\n", phi);
	}

	const float phi = m_ch2_phi;
	const float sample = volume * sin(wt - phi);

	s_last_phi = ft;
	m_last_freq = freq;

	m_internal_samples[m_current_sample] = sample;


	if(m_current_sample == m_internal_samples.size() - 1 && m_device != 0) {
		debugfile.write(reinterpret_cast<char*>(&m_internal_samples[0]), m_internal_samples.size() * sizeof(float));

		assert(
				SDL_BuildAudioCVT(&m_out_converter, AUDIO_F32, 1, 262144, AUDIO_F32, 1, 48000)
				> 0
		);
		std::vector<uint8> output;
		output.resize(m_internal_samples.size() * sizeof(float) * m_out_converter.len_mult);
		std::memcpy(&output[0], &m_internal_samples[0], m_internal_samples.size() * sizeof(float));

		m_out_converter.len = m_internal_samples.size() * sizeof(float);
		m_out_converter.buf = &output[0];

		if(SDL_ConvertAudio(&m_out_converter) != 0) {
			fmt::print("Sound/ Format conversion failed: {}\n", SDL_GetError());
		} else {
			fmt::print("Sound/ Bytes in queue: {}\n", 	SDL_GetQueuedAudioSize(m_device));

			auto rc = SDL_QueueAudio(m_device, &output[0], m_out_converter.len_cvt);
			if(rc != 0) {
				fmt::print("Sound/ Queue failed: {}\n", SDL_GetError());
			}
		}

		m_current_sample = 0;
		return;
	}

	m_current_sample++;
}
