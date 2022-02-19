#pragma once
#include <optional>
#include <SDL.h>
#include <string>
#include <tuple>
#include <vector>
#include "Emulator/Module.hpp"

class AudioOptions : Module {
	std::vector<std::tuple<unsigned, std::string>> m_audio_devices;
	std::optional<unsigned> m_current_audio_device {};
	bool m_audio_devices_loaded { false };
	int m_volume {};
public:
	AudioOptions(GaBber& emu)
	    : Module(emu) {}

	void draw();
};