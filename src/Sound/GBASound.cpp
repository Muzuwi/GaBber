#include <cmath>
#include "GBASound.hpp"
#include "Headers/GaBber.hpp"

Array<uint8, 16>& Sound3Bank::current_bank() {
	return GaBber::instance().mem().io.ch3ctlL->bank ? m_bank1 : m_bank0;
}

void GBASound::init() {
	SDL_AudioSpec request {};
	std::memset(&request, 0, sizeof(request));

	request.freq = output_sample_rate;
	request.format = AUDIO_F32;
	request.channels = 1;
	request.samples = output_sample_count;
	request.callback = nullptr;

	m_device = SDL_OpenAudioDevice(nullptr, 0, &request, &m_device_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if(m_device == 0) {
		fmt::print("Sound/ Failed opening audio device: {}\n", SDL_GetError());
		return;
	}

	fmt::print("Got device spec: samplerate={}, samples={}, channels={}\n",
				m_device_spec.freq, m_device_spec.samples, m_device_spec.channels);
	SDL_PauseAudioDevice(m_device, 0);
}

void GBASound::cycle() {
	//  "The PSG channels 1-4 are internally generated at 262.144kHz"
	//  One internal sample generated every 64 CPU cycles (main clock 16MHz)
	m_cycles++;
	if((m_cycles % 64) != 0)
		return;

	const float ch1 = generate_sample_ch1();
	const float ch2 = generate_sample_ch2();
	const float ch3 = generate_sample_ch3();
	const float ch4 = generate_sample_ch4();

	m_internal_samples[m_current_sample] = ch1 + ch2 + ch3 + ch4;

	if(m_current_sample != m_internal_samples.size() - 1) {
		m_current_sample++;
		return;
	}
	m_current_sample = 0;

	const unsigned queued_size = SDL_GetQueuedAudioSize(m_device) / sizeof(float);
	const unsigned half_buffer_size = output_sample_count / 2;
	if(queued_size < half_buffer_size) {
//		fmt::print("Sound/ Lagging behind! Increasing sound rate\n");
		m_speed_scale += 1;
	} else if(queued_size >= half_buffer_size) {
//		fmt::print("Sound/ Buffer is more than half full, decreasing sound rate\n");
		m_speed_scale = (m_speed_scale > 1 ? m_speed_scale - 1 : 1);
	}

	assert(
			SDL_BuildAudioCVT(&m_out_converter, AUDIO_F32, 1, psg_sample_rate, AUDIO_F32, 1, output_sample_rate)
			> 0
	);
	std::vector<uint8> output;
	output.resize(m_internal_samples.size() * sizeof(float) * m_out_converter.len_mult);
	std::memcpy(&output[0], &m_internal_samples[0], m_internal_samples.size() * sizeof(float));

	m_out_converter.len = m_internal_samples.size() * sizeof(float);
	m_out_converter.buf = &output[0];

	if(SDL_ConvertAudio(&m_out_converter) != 0) {
		fmt::print("Sound/ Format conversion failed: {}\n", SDL_GetError());
		return;
	}

//	fmt::print("Sound/ Samples in queue: {}\n", SDL_GetQueuedAudioSize(m_device) / sizeof(float));
	auto rc = SDL_QueueAudio(m_device, &output[0], m_out_converter.len_cvt);
	if(rc != 0) {
		fmt::print("Sound/ Queue failed: {}\n", SDL_GetError());
	}
}

float GBASound::generate_sample_ch1() {
	static unsigned s_last_freq {0};
	static float s_last_phi {0.0f};
	static float s_ch1_phi {0.0f};
	static constexpr const float pi = M_PI;

	auto& io = GaBber::instance().mem().io;
	auto& ch1 = io.ch1ctlX;

	if(!(io.soundcntL->channel_enable_l & 0b0001) && !(io.soundcntL->channel_enable_r & 0b0001)) {
		return 0.0f;
	}

	const float volume = 0.2;

	const unsigned sample_rate = 262144;    //  in Hz
	const unsigned sample_number = m_cycles / 64;
	const unsigned freq = 131072u / (2048u - ch1->frequency);
	const auto samples_per_period = (unsigned)(((float)sample_rate / (float)freq));

	//  Asin(wt + phi)
	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);
	const float wt = 2 * pi * ft;

	if(s_last_freq != freq) {
		const float delta = ft - s_last_phi;
		const float phi = s_ch1_phi + 2 * pi * delta;
		s_ch1_phi = phi;
	}

	const float phi = s_ch1_phi;
	const float sample = volume * std::sin(wt - phi);

	s_last_phi = ft;
	s_last_freq = freq;

	return sample;
}

float GBASound::generate_sample_ch2() {
	static unsigned s_last_freq {0};
	static float s_last_phi {0.0f};
	static float s_ch2_phi {0.0f};
	static constexpr const float pi = M_PI;

	auto& io = GaBber::instance().mem().io;
	auto& ch2 = io.ch2ctlH;

	if(!(io.soundcntL->channel_enable_l & 0b0010) && !(io.soundcntL->channel_enable_r & 0b0010)) {
		return 0.0f;
	}

	const float volume = 0.2;

	const unsigned sample_rate = 262144;    //  in Hz
	const unsigned sample_number = m_cycles / 64;
	const unsigned freq = 131072u / (2048u - ch2->frequency);
	const auto samples_per_period = (unsigned)(((float)sample_rate / (float)freq));

	//  Asin(wt + phi)
	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);
	const float wt = 2 * pi * ft;

	if(s_last_freq != freq) {
		const float delta = ft - s_last_phi;
		const float phi = s_ch2_phi + 2 * pi * delta;
		s_ch2_phi = phi;
	}

	const float phi = s_ch2_phi;
	const float sample = volume * sin(wt - phi);

	s_last_phi = ft;
	s_last_freq = freq;

	return sample;
}

float GBASound::generate_sample_ch3() {
	static unsigned s_which_digit {0};
	auto& io = GaBber::instance().mem().io;

	if(!(io.soundcntL->channel_enable_l & 0b0100) && !(io.soundcntL->channel_enable_r & 0b0100)) {
		return 0.0f;
	}

	if(!io.ch3ctlL->enabled) {
		return 0.0f;
	}

	const float volume = 0.2;
	const unsigned sample_rate = 4194304 / (32 * (2048 - io.ch3ctlX->rate));



	return 0.0f;

}

float GBASound::generate_sample_ch4() {
	return 0;
}
