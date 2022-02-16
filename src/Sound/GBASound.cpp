#include "GBASound.hpp"
#include <cmath>
#include "Headers/GaBber.hpp"

void GBASound::push_samples(float left, float right) {
	const float master_volume = GaBber::instance().config().volume;
	m_internal_samples[m_current_sample] = master_volume * left;
	m_internal_samples[m_current_sample + 1] = master_volume * right;

	if(m_current_sample != m_internal_samples.size() - 2) {
		m_current_sample += 2;
		return;
	}
	m_current_sample = 0;

	const unsigned queued_size = SDL_GetQueuedAudioSize(m_device) / (sizeof(float) * 2);
	const unsigned half_buffer_size = (output_sample_count / 2) / 2;
	if(queued_size < half_buffer_size) {
		m_speed_scale += 1;
	} else {
		m_speed_scale = (m_speed_scale > 1 ? m_speed_scale - 1 : 1);

		//  We're generating too many samples, which
		//  will lead to significant audio lag quickly
		if(queued_size > 4 * half_buffer_size) {
			//  Approach 1 - drop entire buffer
			fmt::print("Sound/ Going too fast - dropping {} samples ({} over double buffer size)\n",
			           queued_size,
			           queued_size - 4 * half_buffer_size);
			m_current_sample = 0;
			return;
		}
	}

	assert(
	        SDL_BuildAudioCVT(&m_out_converter, AUDIO_F32, 2, psg_sample_rate, AUDIO_F32, 2, output_sample_rate) > 0);
	std::vector<uint8> output;
	output.resize(m_internal_samples.size() * sizeof(float) * m_out_converter.len_mult);
	std::memcpy(&output[0], &m_internal_samples[0], m_internal_samples.size() * sizeof(float));

	m_out_converter.len = m_internal_samples.size() * sizeof(float);
	m_out_converter.buf = &output[0];

	if(SDL_ConvertAudio(&m_out_converter) != 0) {
		fmt::print("Sound/ Format conversion failed: {}\n", SDL_GetError());
		return;
	}

	auto rc = SDL_QueueAudio(m_device, &output[0], m_out_converter.len_cvt);
	if(rc != 0) {
		fmt::print("Sound/ Queue failed: {}\n", SDL_GetError());
	}
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

	if(m_cycles % 65536 == 0) {
		timer_tick_length();
	}
	if(m_cycles % 131072 == 0) {
		timer_tick_sweep();
	}
	if(m_cycles % 262144 == 0) {
		timer_tick_envelope();
	}
	timer_tick_wave();

	if((m_cycles % 64) != 0)
		return;

	auto& io = GaBber::instance().mem().io;
	const int16 ch1 = generate_sample_square1();
	const int16 ch2 = generate_sample_square2();
	const int16 ch3 = generate_sample_noise();
	const int16 ch4 = generate_sample_wave();
	const int16 fifoA = generate_sample_fifoA();
	const int16 fifoB = generate_sample_fifoB();

	const int16 vol_l = io.soundctlL->volume_l + 1;
	const int16 vol_r = io.soundctlL->volume_r + 1;
	int16 left_sample = 0;
	int16 right_sample = 0;

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

	//  DMA mixing
	if(io.soundctlH->psg_volume != 3) {
		left_sample = left_sample / (1u << (2 - io.soundctlH->psg_volume));
		right_sample = right_sample / (1u << (2 - io.soundctlH->psg_volume));
	}

	if(io.soundctlH->enable_left_A) {
		left_sample += fifoA;
	}
	if(io.soundctlH->enable_right_A) {
		right_sample += fifoA;
	}
	if(io.soundctlH->enable_left_B) {
		left_sample += fifoB;
	}
	if(io.soundctlH->enable_right_B) {
		right_sample += fifoB;
	}

	const uint16 bias = (*io.soundbias >> 1u) & 0x1FF;
	const uint8 resolution = (*io.soundbias >> 14u) & 0b11u;

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
		capacitor = in - out * 0.999958f;
		return out;
	};
	auto capR = [](float in) -> float {
		static float capacitor = 0.0f;
		float out = 0.0f;
		out = in - capacitor;
		capacitor = in - out * 0.999958f;
		return out;
	};

	push_samples(capL(normalized_left),
	             capR(normalized_right));
}

/*
 *  Timer connected to sound length
 */
void GBASound::timer_tick_length() {
	if(m_square1.running && m_square1.length_counter != 0) {
		m_square1.length_counter--;
		if(m_square1.length_counter == 0) {
			m_square1.running = false;
		}
	}

	if(m_square2.running && m_square2.length_counter != 0) {
		m_square2.length_counter--;
		if(m_square2.length_counter == 0) {
			m_square2.running = false;
		}
	}

	if(m_wave.running && m_wave.length_counter != 0) {
		m_wave.length_counter--;
		if(m_wave.length_counter == 0) {
			m_wave.running = false;
		}
	}
}

/*
 *  Timer connected to sweep
 */
void GBASound::timer_tick_sweep() {
}

/*
 *  Timer connected to envelope
 */
void GBASound::timer_tick_envelope() {
	auto const& io = GaBber::instance().mem().io;
	auto const& ch1 = io.ch1ctlH;
	auto const& ch2 = io.ch2ctlL;

	if(m_square1.running && m_square1.envelope_step != 0) {
		if(m_square1.envelope_counter != 0) {
			m_square1.envelope_counter--;
		} else {
			m_square1.envelope_counter = m_square1.envelope_step;

			if(ch1->envelope_inc && m_square1.volume_counter < 15) {
				m_square1.volume_counter++;
			} else if(m_square1.volume_counter > 0) {
				m_square1.volume_counter--;
			}
		}
	}

	if(m_square2.running && m_square2.envelope_step != 0) {
		if(m_square2.envelope_counter != 0) {
			m_square2.envelope_counter--;
		} else {
			m_square2.envelope_counter = m_square2.envelope_step;

			if(ch2->envelope_inc && m_square2.volume_counter < 15) {
				m_square2.volume_counter++;
			} else if(m_square2.volume_counter > 0) {
				m_square2.volume_counter--;
			}
		}
	}
}

/*
 *  Timer used by the wave output channel, represents the sample rate
 *  of the output digital data.
 */
void GBASound::timer_tick_wave() {
	if(!m_wave.running) {
		return;
	}

	if(m_wave.cycles != 0) {
		m_wave.cycles--;
		return;
	}

	//  Reload the cycle counter with the amount of CPU cycles
	//  for the current sample rate
	m_wave.cycles = 16 * kB * kB / m_wave.frequency;

	auto& io = GaBber::instance().mem().io;

	//  Consume 1 digit
	if(io.ch3ctlL->dimension) {
		m_wave.current_digit = (m_wave.current_digit + 1) % 64;
	} else {
		m_wave.current_digit = (m_wave.current_digit + 1) % 32;
	}
}

int16 GBASound::generate_sample_square1() {
	auto& io = GaBber::instance().mem().io;
	auto& ch1h = io.ch1ctlH;

	if(!m_square1.running) {
		return 0;
	}

	const unsigned sample_rate = 262144;//  in Hz
	const unsigned sample_number = m_cycles / 64;
	const unsigned freq = m_square1.frequency;
	const auto samples_per_period = (unsigned)(((float)sample_rate / (float)freq));
	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);

	const float duty = duty_lookup[ch1h->duty];
	const auto envelope = static_cast<int16>(m_square1.volume_counter);
	const int16 sample = (ft >= duty) ? (int16)0 : envelope;

	return sample;
}

int16 GBASound::generate_sample_square2() {
	auto& io = GaBber::instance().mem().io;
	auto& ch2l = io.ch2ctlL;

	if(!m_square2.running) {
		return 0;
	}

	const unsigned sample_rate = 262144;//  in Hz
	const unsigned sample_number = m_cycles / 64;
	const unsigned freq = m_square2.frequency;
	const auto samples_per_period = (unsigned)(((float)sample_rate / (float)freq));
	const float ft = ((float)(sample_number % samples_per_period) / (float)samples_per_period);

	const float duty = duty_lookup[ch2l->duty];
	const auto envelope = static_cast<int16>(m_square2.volume_counter);
	const int16 sample = (ft >= duty) ? (int16)0 : envelope;

	return sample;
}

int16 GBASound::generate_sample_noise() {
	if(!m_wave.running) {
		return 0;
	}
	IOContainer const& io = GaBber::instance().mem().io;

	const unsigned which_bank =
	        ((unsigned)io.ch3ctlL->bank +
	         (m_wave.current_digit / 32)) %
	        2;
	const unsigned which_digit = m_wave.current_digit % 32;
	auto const& bank = (which_bank == 0) ? io.ch3bank.bank0() : io.ch3bank.bank1();
	const unsigned byte = which_digit / 2;
	const bool upper = (which_digit % 2) == 0;

	uint8 digit;
	if(upper) {
		digit = bank[byte] >> 4u;
	} else {
		digit = bank[byte] & 0x0Fu;
	}

	if(io.ch3ctlH->force_volume) {
		return (int16)((digit * 3) / 4);
	}

	const unsigned shift = (io.ch3ctlH->volume == 0) ? 4 : (io.ch3ctlH->volume - 1);
	return (int16)(digit >> shift);
}

int16 GBASound::generate_sample_wave() {
	return 0;
}

int16 GBASound::generate_sample_fifoA() {
	if(m_soundA.samples.empty()) {
		return 0;
	}
	IOContainer& io = GaBber::instance().mem().io;

	const int16 sample = static_cast<int16>(static_cast<int8>(m_soundA.samples.front())) / (2 - io.soundctlH->volumeA);
	m_soundA.samples.pop_front();

	return sample;
}

int16 GBASound::generate_sample_fifoB() {
	if(m_soundB.samples.empty()) {
		return 0;
	}
	IOContainer& io = GaBber::instance().mem().io;

	const int16 sample = static_cast<int16>(static_cast<int8>(m_soundB.samples.front())) / (2 - io.soundctlH->volumeB);
	m_soundB.samples.pop_front();

	return sample;
}

void GBASound::reload_square1() {
	IOContainer& io = GaBber::instance().mem().io;

	m_square1.running = true;
	m_square1.frequency = 131072 / (2048 - io.ch1ctlX->frequency);
	if(io.ch1ctlX->length_flag) {
		m_square1.length_counter = 64 - io.ch1ctlH->length;
	} else {
		m_square1.length_counter = 0;
	}
	m_square1.volume_counter = io.ch1ctlH->envelope_vol;
	m_square1.envelope_step = io.ch1ctlH->envelope_step;
	m_square1.envelope_counter = io.ch1ctlH->envelope_step;
}

void GBASound::reload_square2() {
	IOContainer& io = GaBber::instance().mem().io;

	m_square2.running = true;
	m_square2.frequency = 131072 / (2048 - io.ch2ctlH->frequency);
	if(io.ch2ctlH->length_flag) {
		m_square2.length_counter = 64 - io.ch2ctlL->length;
	} else {
		m_square2.length_counter = 0;
	}
	m_square2.volume_counter = io.ch2ctlL->envelope_vol;
	m_square2.envelope_step = io.ch2ctlL->envelope_step;
	m_square2.envelope_counter = io.ch2ctlL->envelope_step;
}

void GBASound::reload_wave() {
	IOContainer& io = GaBber::instance().mem().io;

	m_wave.running = true;
	m_wave.frequency = 2097152 / (2048 - io.ch3ctlX->rate);
	if(io.ch3ctlX->length_flag) {
		m_wave.length_counter = 256 - io.ch3ctlH->length;
	} else {
		m_wave.length_counter = 0;
	}
	m_wave.current_digit = 0;
	m_wave.cycles = 0;
}

void GBASound::set_wave_running(bool running) {
	if(!m_wave.running && running) {
		reload_wave();
	} else {
		m_wave.running = running;
	}
}

void GBASound::on_timer_overflow(unsigned timer_num) {
	auto& io = GaBber::instance().mem().io;

	//  Only Timers 0/1 can be used for DMA sound
	if(timer_num >= 2) {
		return;
	}

	const unsigned target_sample_rate = 131072 >> (3 - ((*io.soundbias >> 14u) & 0b11u));

	if((io.soundctlH->enable_left_A || io.soundctlH->enable_right_A) &&
	   io.soundctlH->timer_sel_A == timer_num) {
		std::deque<uint8>& fifo = io.fifoA.fifo();
		if(!fifo.empty()) {
			//  Output sample to audio engine
			push_sample_fifoA(fifo.front(), target_sample_rate);
			fifo.pop_front();
		}

		//  Check if FIFOs contain enough data
		if(fifo.size() == 16 || fifo.empty()) {
			GaBber::instance().cpu().dma_request_fifoA();
		}
	}

	if((io.soundctlH->enable_left_B || io.soundctlH->enable_right_B) &&
	   io.soundctlH->timer_sel_B == timer_num) {
		std::deque<uint8>& fifo = io.fifoB.fifo();
		if(!fifo.empty()) {
			//  Output sample to audio engine
			push_sample_fifoB(fifo.front(), target_sample_rate);
			fifo.pop_front();
		}

		//  Check if FIFOs contain enough data
		if(fifo.size() == 16 || fifo.empty()) {
			GaBber::instance().cpu().dma_request_fifoB();
		}
	}
}

void GBASound::push_sample_fifoA(uint8 value, unsigned sample_rate) {
	for(unsigned i = 0; i < 262144 / sample_rate; ++i) {
		m_soundA.samples.push_back(value);
	}
}

void GBASound::push_sample_fifoB(uint8 value, unsigned sample_rate) {
	for(unsigned i = 0; i < 262144 / sample_rate; ++i) {
		m_soundB.samples.push_back(value);
	}
}
