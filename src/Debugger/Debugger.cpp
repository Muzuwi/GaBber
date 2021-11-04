#include "Debugger/Debugger.hpp"
#include "Headers/GaBber.hpp"

void Debugger::draw_debugger_contents() {
	m_mem_editor.draw();
	m_screen.draw();
	m_registers.draw();
	m_io_registers.draw();
	m_memimg.draw();
	m_breakpoints.draw();
	m_stacktrace.draw();
	ImGui::ShowDemoWindow();
}

Debugger::Debugger(GaBber& v)
: emu(v),
  m_mem_editor(v),
  m_screen(v),
  m_registers(v),
  m_io_registers(v),
  m_memimg(v),
  m_breakpoints(v),
  m_stacktrace(v)
{

}

void Debugger::on_memory_access(uint32 address, uint32 val, bool write) {

}

void Debugger::on_memory_access(uint32 address, uint16 val, bool write) {

}

void Debugger::on_memory_access(uint32 address, uint8 val, bool write) {

}

void Debugger::on_execute_opcode(uint32 address) {
	auto it = std::find(m_execution_breakpoints.begin(), m_execution_breakpoints.end(), address);
	if(it == m_execution_breakpoints.end()) {
		return;
	}

	fmt::print("Debugger/ Reached execution breakpoint at {:08x}\n", address);
	emu.enter_debug_mode();
}

void Debugger::on_undefined_access(uint32 address) {
	if(!m_break_on_undefined) {
		return;
	}

	emu.enter_debug_mode();
}
