#include "Debugger/Debugger.hpp"
#include "Headers/GaBber.hpp"

void Debugger::draw_debugger_contents() {
	m_mem_editor.draw();
	m_screen.draw();
	m_registers.draw();
	m_io_registers.draw();
	m_memimg.draw();
	m_breakpoint_control.draw();
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
  m_breakpoint_control(v),
  m_stacktrace(v)
{

}

bool Debugger::match_breakpoint(Breakpoint const& breakpoint, MemoryEvent event) {
	const bool not_overlapping =
			breakpoint.start+breakpoint.size < event.address ||
			event.address+event.size < breakpoint.start;
	const bool matched_flags = (breakpoint.type & event.type) != 0;

	return !not_overlapping && matched_flags;
}

void Debugger::on_memory_access(uint32 address, uint32 val, bool write) {
	auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [address, val, write](auto& v) {
		return match_breakpoint(v, {address, val, 4, write ? BreakWrite : BreakRead});
	});
	if(it == m_breakpoints.end()) {
		return;
	}

	fmt::print("Debugger/ Reached breakpoint at {:08x}\n", address);
	emu.enter_debug_mode();
}

void Debugger::on_memory_access(uint32 address, uint16 val, bool write) {
	auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [address, val, write](auto& v) {
		return match_breakpoint(v, {address, val, 2, write ? BreakWrite : BreakRead});
	});
	if(it == m_breakpoints.end()) {
		return;
	}

	fmt::print("Debugger/ Reached breakpoint at {:08x}\n", address);
	emu.enter_debug_mode();
}

void Debugger::on_memory_access(uint32 address, uint8 val, bool write) {
	auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [address, val, write](auto& v) {
		return match_breakpoint(v, {address, val, 1, write ? BreakWrite : BreakRead});
	});
	if(it == m_breakpoints.end()) {
		return;
	}

	fmt::print("Debugger/ Reached breakpoint at {:08x}\n", address);
	emu.enter_debug_mode();
}

void Debugger::on_execute_opcode(uint32 address) {
	auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [address](auto& v) {
		return match_breakpoint(v, {address, 0, 1, BreakExec});
	});
	if(it == m_breakpoints.end()) {
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
