#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "GL/gl3w.h"
#include "Headers/StdTypes.hpp"
#include "Debugger/DebuggerWindow.hpp"
#include "MMU/BusDevice.hpp"


class MemEditor : public DebuggerWindow {
	MemoryEditor m_editor;

	void draw_window() override;
public:
	MemEditor(GaBber& emu)
	: DebuggerWindow("RAM", emu) {}
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
	: DebuggerWindow("Screen", emu) {}
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

class MemoryImage : public DebuggerWindow {
	bool created_tex;
	GLuint64 tex;
	void draw_window() override;
public:
	MemoryImage(GaBber& emu)
	: DebuggerWindow("Memory Image", emu) {}
};

class Breakpoints : public DebuggerWindow {
	uint32 m_break_address;
	void draw_window() override;
public:
	Breakpoints(GaBber& emu)
	: DebuggerWindow("Breakpoints", emu) {}
};

class Stacktrace : public DebuggerWindow {
	MemoryEditor m_stack;
	void draw_window() override;
public:
	Stacktrace(GaBber& emu)
	: DebuggerWindow("Stacktrace", emu) {}
};
