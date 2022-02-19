#pragma once
#include <array>
#include <deque>
#include <SDL_audio.h>
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

struct SoundData {
	bool running;
	unsigned length_counter;
	unsigned volume_counter;
	unsigned frequency;
	unsigned envelope_counter;
	unsigned envelope_step;
};

struct WaveData : public SoundData {
	unsigned cycles;
	unsigned current_digit;
};

struct FifoData {
	std::deque<uint8> samples;
};

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

	static constexpr const float duty_lookup[4] = { 0.125f, 0.25f, 0.5f, 0.75f };

	SDL_AudioSpec m_device_spec;
	SDL_AudioDeviceID m_device;
	SDL_AudioCVT m_out_converter;

	std::array<float, psg_sample_count> m_internal_samples;
	unsigned m_current_sample;
	int m_speed_scale { 1 };

	uint64 m_cycles;
	SoundData m_square1;
	SoundData m_square2;
	WaveData m_wave;
	FifoData m_soundA;
	FifoData m_soundB;

	void push_sample_fifoA(uint8 value, unsigned sample_rate);
	void push_sample_fifoB(uint8 value, unsigned sample_rate);
	void push_samples(float left, float right);

	int16 generate_sample_square1();
	int16 generate_sample_square2();
	int16 generate_sample_noise();
	int16 generate_sample_wave();
	int16 generate_sample_fifoA();
	int16 generate_sample_fifoB();

	void timer_tick_length();
	void timer_tick_sweep();
	void timer_tick_envelope();
	void timer_tick_wave();
public:
	APU(GaBber&);
	void init();
	void cycle();

	std::array<float, psg_sample_count> const& internal_samples() const { return m_internal_samples; }

	void reload_square1();
	void reload_square2();
	void reload_wave();
	void set_wave_running(bool running);

	int speed_scale() const { return m_speed_scale; }

	void on_timer_overflow(unsigned timer_num);

	bool switch_audio_device(char const*);
};