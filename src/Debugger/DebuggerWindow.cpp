#include "Debugger/DebuggerWindow.hpp"

void DebuggerWindow::draw() {
	ImGui::Begin(m_name.c_str(), &m_is_open, m_flags);
	draw_window();
	ImGui::End();
}
