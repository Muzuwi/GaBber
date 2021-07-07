#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"

void Screen::draw_window() {
	ImGui::Image((ImTextureID)(GLuint64)m_emu.gba_texture(),
				 ImVec2(240 * m_emu.window_scale(), 160 * m_emu.window_scale())
				 );
}
