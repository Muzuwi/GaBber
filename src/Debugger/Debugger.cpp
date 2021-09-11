#include "Debugger/Debugger.hpp"

void Debugger::draw_debugger_contents() {
	m_mem_editor.draw();
	m_screen.draw();
	m_registers.draw();
	m_io_registers.draw();
	m_memimg.draw();
	ImGui::ShowDemoWindow();
}

Debugger::Debugger(GaBber& v)
: emu(v),
  m_mem_editor(v),
  m_screen(v),
  m_registers(v),
  m_io_registers(v),
  m_memimg(v)
{

}

void Debugger::on_memory_access(uint32 address, uint32 val, bool write) {

}

void Debugger::on_memory_access(uint32 address, uint16 val, bool write) {

}

void Debugger::on_memory_access(uint32 address, uint8 val, bool write) {

}
