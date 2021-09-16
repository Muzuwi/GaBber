#pragma once
#include <SDL2/SDL.h>
#include "GL/gl3w.h"
#include "Debugger/Debugger.hpp"
#include "Headers/ARM7TDMI.hpp"
#include "MMU/BusInterface.hpp"
#include "MMU/MemoryLayout.hpp"
#include "PPU/PPU.hpp"
#include "Tests/Harness.hpp"
#include "Sound/GBASound.hpp"


class GaBber {
	std::string m_rom_filename;

	Debugger m_debugger;
	BusInterface m_mmu;
	ARM7TDMI m_cpu;
	PPU m_ppu;
    TestHarness m_test_harness;
    MemoryLayout m_mem;
	GBASound m_sound;
	bool m_test_mode;

	void toggle_debug_mode();

	bool m_closed;
	SDL_Window* m_gabberWindow;
	SDL_GLContext m_gabberGLContext;
	unsigned m_window_scale {5};
	GLuint m_fb, m_gba_texture;

	void _disp_create_gl_state();
	void _disp_poll_events();
	void _disp_draw_gba_screen();
	void _disp_draw_debugger();
	void _disp_draw_common();
	void _disp_draw_frame();

	void display_update();
	bool display_initialize();
	bool display_is_closed() const { return m_closed; }

	void emulator_reset();
	void emulator_loop();
	void clock_cycle();

	GaBber()
	: m_debugger(*this), m_mmu(), m_cpu(m_mmu, m_debugger, m_mem.io), m_ppu(m_cpu, m_mem), m_test_harness(*this) {}
public:
	static GaBber& instance() {
		static GaBber emu;
		return emu;
	}

	void parse_args(int argc, char** argv);
    int start();

    BusInterface& mmu() { return m_mmu; }
	ARM7TDMI& cpu() { return m_cpu; }
	PPU& ppu() { return m_ppu; }
	Debugger& debugger() { return m_debugger; }
	MemoryLayout& mem() { return m_mem; }
	GBASound& sound() { return m_sound; }

	unsigned window_scale() const { return m_window_scale; }
	GLuint gba_texture() const { return m_gba_texture; }
};