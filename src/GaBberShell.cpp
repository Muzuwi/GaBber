#include "Headers/GaBber.hpp"

void GaBber::_shell_draw_options_audio() {
	ImGui::SliderFloat("Volume", &m_config.volume, 0.0f, 1.0f, "");
}
