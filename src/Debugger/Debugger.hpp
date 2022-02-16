#pragma once
#include <string>
#include <functional>
#include <Headers/StdTypes.hpp>
#include <imgui.h>
#include "Debugger/WindowDefinitions.hpp"
#include "Debugger/Breakpoint.hpp"

class GaBber;
class Debugger {
	friend class BreakpointControl;
	GaBber& emu;

	MemEditor m_mem_editor;
	Screen m_screen;
	GPRs m_registers;
	IORegisters m_io_registers;
	BreakpointControl m_breakpoint_control;
	Stacktrace m_stacktrace;

	static bool match_breakpoint(Breakpoint const& breakpoint, MemoryEvent event);
	Vector<Breakpoint> m_breakpoints;
	bool m_break_on_undefined {false};
	bool m_debug_mode {false};
public:
	Debugger(GaBber& v);

	void draw_debugger_contents();

	bool is_debug_mode() const { return m_debug_mode; }
	void set_debug_mode(bool v) { m_debug_mode = v; }

	void on_memory_access(uint32 address, uint32 val, bool write);
	void on_memory_access(uint32 address, uint16 val, bool write);
	void on_memory_access(uint32 address, uint8  val, bool write);
	void on_execute_opcode(uint32 address);
	void on_undefined_access(uint32 address);
};

