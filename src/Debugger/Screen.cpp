#include "Debugger/WindowDefinitions.hpp"
#include "Emulator/GaBber.hpp"
#include "Emulator/Renderer.hpp"

void Screen::draw_window() {
	ImGui::Image((ImTextureID)(GLuint64)m_emu.renderer().gba_texture(),
	             ImVec2(240 * m_emu.renderer().window_scale(), 160 * m_emu.renderer().window_scale()));
}
