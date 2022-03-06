#include "APU.hpp"
#include <cassert>
#include <fmt/format.h>
#include <soxr.h>
#include <vector>
#include "Bus/Common/MemoryLayout.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Emulator/GaBber.hpp"

APU::APU(GaBber& emu)
    : Module(emu)
    , m_square1(emu)
    , m_square2(emu)
    , m_wave(emu)
    , m_noise(emu)
    , m_fifo_a(emu)
    , m_fifo_b(emu) {}

void APU::push_samples(float left, float right) {
	const float user_volume = (static_cast<float>(config().volume) / 100.0f);
	const float master_volume = std::pow(user_volume, 2.0f);
	m_internal_samples[m_current_sample] = master_volume * left;
	m_internal_samples[m_current_sample + 1] = master_volume * right;

	if(m_current_sample != m_internal_samples.size() - 2) {
		m_current_sample += 2;
		return;
	}
	m_current_sample = 0;

	const unsigned queued_size = SDL_GetQueuedAudioSize(m_device) / (sizeof(float) * 2);
	const unsigned half_buffer_size = (output_sample_count / 2) / 2;
	//  Prevent rampant growth of the sample buffer (which could lead to significant audio delay) by preemptively
	//  dropping buffers when we notice that there's already too many samples queued up.
	if(queued_size > 4 * half_buffer_size) {
		fmt::print("Sound/ Going too fast - dropping {} samples ({} over double buffer size)\n", queued_size,
		           queued_size - 4 * half_buffer_size);
		m_current_sample = 0;
		return;
	}

	const double input_rate = (double)psg_sample_rate;
	const double output_rate = (double)output_sample_rate;
	//  Per channel
	const size_t input_length = psg_sample_count / 2;
	const size_t output_length = output_sample_count / 2;

	std::vector<float> output_samples {};
	output_samples.resize(output_sample_count);

	size_t actual_output_length {};
	soxr_error_t soxr_error =
	        soxr_oneshot(input_rate, output_rate, 2, &m_internal_samples[0], input_length, nullptr, &output_samples[0],
	                     output_length, &actual_output_length, nullptr, nullptr, nullptr);
	const auto byte_count = actual_output_length * 2 * sizeof(float);

	const auto error = SDL_QueueAudio(m_device, &output_samples[0], byte_count);
	if(error != 0) {
		fmt::print("Sound/ Queue failed: {}\n", SDL_GetError());
	}
}

void APU::initialize_platform() {
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

	fmt::print("Got device spec: samplerate={}, samples={}, channels={}\n", m_device_spec.freq, m_device_spec.samples,
	           m_device_spec.channels);
	SDL_PauseAudioDevice(m_device, 0);
}

void APU::cycle() {
	//  "The PSG channels 1-4 are internally generated at 262.144kHz"
	//  One internal sample generated every 64 CPU cycles (main clock 16MHz)
	m_cycles++;
	m_square1.tick();
	m_square2.tick();
	m_wave.tick();
	m_noise.tick();

	if((m_cycles % 64) != 0)
		return;

	int16 ch1 = m_square1.generate_sample();
	int16 ch2 = m_square2.generate_sample();
	int16 ch3 = m_wave.generate_sample();
	int16 ch4 = m_noise.generate_sample();
	int16 fifoA = m_fifo_a.generate_sample();
	int16 fifoB = m_fifo_b.generate_sample();
	if(!config().apu_ch1_enabled) {
		ch1 = 0;
	}
	if(!config().apu_ch2_enabled) {
		ch2 = 0;
	}
	if(!config().apu_ch3_enabled) {
		ch3 = 0;
	}
	if(!config().apu_ch4_enabled) {
		ch4 = 0;
	}
	if(!config().apu_fifo_enabled) {
		fifoA = 0;
		fifoB = 0;
	}

	const int16 vol_l = io().soundctlL->volume_l + 1;
	const int16 vol_r = io().soundctlL->volume_r + 1;
	int16 left_sample = 0;
	int16 right_sample = 0;

	if(io().soundctlL->channel_enable_l & 0b0001) {
		left_sample += ch1 * vol_l;
	}
	if(io().soundctlL->channel_enable_r & 0b0001) {
		right_sample += ch1 * vol_r;
	}
	if(io().soundctlL->channel_enable_l & 0b0010) {
		left_sample += ch2 * vol_l;
	}
	if(io().soundctlL->channel_enable_r & 0b0010) {
		right_sample += ch2 * vol_r;
	}
	if(io().soundctlL->channel_enable_l & 0b0100) {
		left_sample += ch3 * vol_l;
	}
	if(io().soundctlL->channel_enable_r & 0b0100) {
		right_sample += ch3 * vol_r;
	}
	if(io().soundctlL->channel_enable_l & 0b1000) {
		left_sample += ch4 * vol_l;
	}
	if(io().soundctlL->channel_enable_r & 0b1000) {
		right_sample += ch4 * vol_r;
	}

	//  DMA mixing
	if(io().soundctlH->psg_volume != 3) {
		left_sample = left_sample / (1u << (2 - io().soundctlH->psg_volume));
		right_sample = right_sample / (1u << (2 - io().soundctlH->psg_volume));
	}

	if(io().soundctlH->enable_left_A) {
		left_sample += fifoA;
	}
	if(io().soundctlH->enable_right_A) {
		right_sample += fifoA;
	}
	if(io().soundctlH->enable_left_B) {
		left_sample += fifoB;
	}
	if(io().soundctlH->enable_right_B) {
		right_sample += fifoB;
	}

	const uint16 bias = (*io().soundbias >> 1u) & 0x1FF;
	const uint8 resolution = (*io().soundbias >> 14u) & 0b11u;

	left_sample += bias;
	right_sample += bias;

	left_sample = std::clamp(left_sample, (int16)0, (int16)0x3FF);
	right_sample = std::clamp(right_sample, (int16)0, (int16)0x3FF);

	//  Change bit depth
	left_sample = left_sample / (1u << (resolution + 1));
	right_sample = right_sample / (1u << (resolution + 1));

	const unsigned max = 0x400 / (1u << (resolution));
	const float normalized_left = ((float)left_sample / max);
	const float normalized_right = ((float)right_sample / max);

	auto capL = [](float in) -> float {
		static float capacitor = 0.0f;
		float out = 0.0f;
		out = in - capacitor;
		capacitor = in - out * 0.997315553f;
		return out;
	};
	auto capR = [](float in) -> float {
		static float capacitor = 0.0f;
		float out = 0.0f;
		out = in - capacitor;
		capacitor = in - out * 0.997315553f;
		return out;
	};

	push_samples(capL(normalized_left), capR(normalized_right));
}

void APU::on_timer_overflow(unsigned timer_num) {
	//  Only Timers 0/1 can be used for DMA sound
	if(timer_num >= 2) {
		return;
	}

	constexpr const auto clock_frequency = 16777216;
	constexpr const unsigned prescaler_divisors[4] { 1, 64, 256, 1024 };

	if(timer_num == 1) {
		//  Do not support count-up timing for now
		assert(!io().timer1.m_ctl->count_up);
	}
	const uint16 reload = (timer_num == 0) ? io().timer0.m_reload_and_current.reload_value()
	                                       : io().timer1.m_reload_and_current.reload_value();
	const auto ticks = 0x10000 - reload;
	const auto divisor =
	        prescaler_divisors[(timer_num == 0) ? io().timer0.m_ctl->prescaler : io().timer1.m_ctl->prescaler];
	const auto prescaler_frequency = clock_frequency / divisor;
	const unsigned sample_rate = prescaler_frequency / ticks;

	if(io().soundctlH->timer_sel_A == timer_num) {
		m_fifo_a.move_data_to_sound_circuit(sample_rate);
	}
	if(io().soundctlH->timer_sel_B == timer_num) {
		m_fifo_b.move_data_to_sound_circuit(sample_rate);
	}
}

bool APU::switch_audio_device(char const* device_name) {
	SDL_AudioSpec request {};
	std::memset(&request, 0, sizeof(request));

	request.freq = output_sample_rate;

	request.format = AUDIO_F32;
	request.channels = 2;
	request.samples = output_sample_count;
	request.callback = nullptr;

	SDL_AudioSpec response {};
	const SDL_AudioDeviceID id = SDL_OpenAudioDevice(device_name, 0, &request, &response, SDL_FALSE);
	if(id == 0) {
		return false;
	}

	SDL_CloseAudioDevice(m_device);
	SDL_PauseAudioDevice(id, 0);

	m_device = id;
	m_device_spec = response;
	return true;
}

float APU::audio_latency() const {
	const unsigned queued_size = SDL_GetQueuedAudioSize(m_device) / (sizeof(float) * 2);
	return static_cast<float>(queued_size) / static_cast<float>(output_sample_rate);
}
