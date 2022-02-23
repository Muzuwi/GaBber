#pragma once

struct Config {
	unsigned volume { 60 };
	unsigned target_framerate { 60 };
	bool apu_ch1_enabled { true };
	bool apu_ch2_enabled { true };
	bool apu_ch3_enabled { true };
	bool apu_ch4_enabled { true };
	bool apu_fifo_enabled { true };
};