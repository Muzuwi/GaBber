#include "Debugger/Debugger.hpp"

void Debugger::draw_debugger_contents() {
	m_mem_editor.draw();
	m_screen.draw();
	m_registers.draw();
}

Debugger::Debugger(GaBber& v)
: emu(v),
  m_mem_editor(v),
  m_screen(v),
  m_registers(v)
{

}
