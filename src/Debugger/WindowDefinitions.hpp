#pragma once
#include <imgui.h>
#include <imgui_memory_editor.h>
#include "Headers/StdTypes.hpp"
#include "Debugger/DebuggerWindow.hpp"
#include "MMU/BusDevice.hpp"
#include "Debugger/Breakpoint.hpp"


class MemEditor : public DebuggerWindow {
	MemoryEditor m_editor;

	void draw_window() override;
public:
	MemEditor(GaBber& emu)
	: DebuggerWindow("RAM", emu) {
		m_flags = ImGuiWindowFlags_None;
	}
};


class GPRs : public DebuggerWindow {
	void draw_window() override;
public:
	GPRs(GaBber& emu)
	: DebuggerWindow("Registers", emu) {}
};


class Screen : public DebuggerWindow {
	void draw_window() override;
public:
	Screen(GaBber& emu)
	: DebuggerWindow("Screen", emu) {
		m_flags = ImGuiWindowFlags_AlwaysAutoResize;
	}
};

class IORegisters : public DebuggerWindow {
	enum class WindowTab {
		Interrupts,
		DMA,
		PPU,
		Sound
	};

	WindowTab m_which_tab;

	void draw_window() override;
public:
	IORegisters(GaBber& emu)
	: DebuggerWindow("I/O Registers", emu) {
		m_which_tab = WindowTab::Interrupts;
	}

	void draw_interrupts();
	void draw_ppu();
	void draw_sound();
	void draw_dma();
};

class BreakpointControl : public DebuggerWindow {
	Breakpoint m_break;
	void draw_window() override;
public:
	BreakpointControl(GaBber& emu)
	: DebuggerWindow("Breakpoints", emu) {
		m_flags = ImGuiWindowFlags_AlwaysAutoResize;
	}
};

class Stacktrace : public DebuggerWindow {
	MemoryEditor m_stack;
	void draw_window() override;
public:
	Stacktrace(GaBber& emu)
	: DebuggerWindow("Stacktrace", emu) {}
};
