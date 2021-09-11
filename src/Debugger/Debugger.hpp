#pragma once
#include <string>
#include <functional>
#include <Headers/StdTypes.hpp>
#include "imgui/imgui.h"
#include "Debugger/WindowDefinitions.hpp"

class GaBber;
class Debugger {
	GaBber& emu;

	MemEditor m_mem_editor;
	Screen m_screen;
	GPRs m_registers;
	IORegisters m_io_registers;
	MemoryImage m_memimg;

	bool m_debug_mode {false};
	bool m_step {false};
	bool m_continue {false};
public:
	Debugger(GaBber& v);

	void draw_debugger_contents();

	bool is_step() const { return m_step; }
	void set_step(bool v) { m_step = v; }

	bool continue_mode() const { return m_continue; }
	void set_continue_mode(bool v) { m_continue = v; }

	bool is_debug_mode() const { return m_debug_mode; }
	void set_debug_mode(bool v) { m_debug_mode = v; }

	void on_memory_access(uint32 address, uint32 val, bool write);
	void on_memory_access(uint32 address, uint16 val, bool write);
	void on_memory_access(uint32 address, uint8  val, bool write);
};

