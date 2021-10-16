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
	request.channels = 2;
	request.samples = output_sample_count;
	request.callback = nullptr;

	m_device = SDL_OpenAudioDevice(nullptr, 0, &request, &m_device_spec, 0);
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

	auto& io = GaBber::instance().mem().io;
	const float ch1 = generate_sample_ch1();
	const float ch2 = generate_sample_ch2();
	const float ch3 = generate_sample_ch3();
	const float ch4 = generate_sample_ch4();

	const float master_volume = 1.0f;
	const float vol_l = (float)io.soundctlL->volume_l / 7.0f;
	const float vol_r = (float)io.soundctlL->volume_r / 7.0f;
	float left_sample = 0.0f;
	float right_sample = 0.0f;

	if(io.soundctlL->channel_enable_l & 0b0001) {
		left_sample += ch1 * vol_l;
	}
	if(io.soundctlL->channel_enable_r & 0b0001) {
		right_sample += ch1 * vol_r;
	}
	if(io.soundctlL->channel_enable_l & 0b0010) {
		left_sample += ch2 * vol_l;
	}
	if(io.soundctlL->channel_enable_r & 0b0010) {
		right_sample += ch2 * vol_r;
	}
	if(io.soundctlL->channel_enable_l & 0b0100) {
		left_sample += ch3 * vol_l;
	}
	if(io.soundctlL->channel_enable_r & 0b0100) {
		right_sample += ch3 * vol_r;
	}
	if(io.soundctlL->channel_enable_l & 0b1000) {
		left_sample += ch4 * vol_l;
	}
	if(io.soundctlL->channel_enable_r & 0b1000) {
		right_sample += ch4 * vol_r;
	}

	m_internal_samples[m_current_sample] = master_volume * left_sample;
	m_internal_samples[m_current_sample+1] = master_volume * right_sample;

	if(m_current_sample != m_internal_samples.size() - 2) {
		m_current_sample += 2;
		return;
	}
	m_current_sample = 0;

	const unsigned queued_size = SDL_GetQueuedAudioSize(m_device) / (sizeof(float)*2);
	const unsigned half_buffer_size = (output_sample_count/2) / 2;
	if(queued_size < half_buffer_size) {
//		fmt::print("Sound/ Lagging behind! Increasing sound rate\n");
		m_speed_scale += 1;
	} else if(queued_size >= half_buffer_size) {
//		fmt::print("Sound/ Buffer is more than half full, decreasing sound rate\n");
		m_speed_scale = (m_speed_scale > 1 ? m_speed_scale - 1 : 1);
	}

	assert(
			SDL_BuildAudioCVT(&m_out_converter, AUDIO_F32, 2, psg_sample_rate, AUDIO_F32, 2, output_sample_rate)
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
	const float sample = 0.4f * std::sin(wt - phi);

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
	const float sample = 0.4f * sin(wt - phi);

	s_last_phi = ft;
	s_last_freq = freq;

	return sample;
}

float GBASound::generate_sample_ch3() {
	static unsigned s_which_digit {0};
	auto& io = GaBber::instance().mem().io;

	if(!(io.soundctlL->channel_enable_l & 0b0100) && !(io.soundctlL->channel_enable_r & 0b0100)) {
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
