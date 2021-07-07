#include "Debugger/DebuggerWindow.hpp"
#include "Headers/GaBber.hpp"

void DebuggerWindow::draw() {
	ImGui::Begin(m_name.c_str(), &m_is_open, m_flags);
	draw_window();
	ImGui::End();
}

ARM7TDMI& DebuggerWindow::cpu() {
	return m_emu.cpu();
}
