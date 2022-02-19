#include "Emulator/AudioOptions.hpp"
#include <imgui.h>
#include "APU/APU.hpp"
#include "Emulator/Config.hpp"

void AudioOptions::draw() {
	if(!m_audio_devices_loaded) {
		const auto device_count = SDL_GetNumAudioDevices(0);
		for(unsigned i = 0; i < device_count; ++i) {
			auto dev = SDL_GetAudioDeviceName(i, 0);
			std::tuple<unsigned, std::string> tup = std::make_tuple(i, std::string { dev });
			m_audio_devices.push_back(tup);
		}
		m_audio_devices_loaded = true;
	}

	auto get_current_str = [this]() -> std::string {
		if(!m_current_audio_device.has_value()) {
			return "[system default]";
		}
		if(*m_current_audio_device >= m_audio_devices.size()) {
			return "";
		}
		auto [id, name] = m_audio_devices[*m_current_audio_device];
		return name;
	};

	auto draw_device_selector = [this](char const* name, std::optional<unsigned> which, bool selected) {
		if(ImGui::Selectable(name, selected)) {
			const bool success = apu().switch_audio_device(name);
			if(success) {
				m_current_audio_device = which;
			}
		}
		if(selected) {
			ImGui::SetItemDefaultFocus();
		}
	};

	if(ImGui::BeginCombo("Output device", get_current_str().c_str())) {
		if(ImGui::Selectable("[system default]", !m_current_audio_device.has_value())) {
			const bool success = apu().switch_audio_device(nullptr);
			if(success) {
				m_current_audio_device = std::nullopt;
			}
		}
		if(!m_current_audio_device.has_value()) {
			ImGui::SetItemDefaultFocus();
		}

		for(unsigned i = 0; i < m_audio_devices.size(); ++i) {
			const auto [id, name] = m_audio_devices[i];
			const bool selected = (m_current_audio_device.has_value() && i == *m_current_audio_device);
			draw_device_selector(name.c_str(), { id }, selected);
		}

		ImGui::EndCombo();
	}

	ImGui::SliderFloat("Volume", &config().volume, 0.0f, 1.0f, "");
}
