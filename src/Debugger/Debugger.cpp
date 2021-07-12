#include "Debugger/Debugger.hpp"

void Debugger::draw_debugger_contents() {
	m_mem_editor.draw();
	m_screen.draw();
	m_registers.draw();
	m_io_registers.draw();
	ImGui::ShowDemoWindow();
}

Debugger::Debugger(GaBber& v)
: emu(v),
  m_mem_editor(v),
  m_screen(v),
  m_registers(v),
  m_io_registers(v)
{

}
