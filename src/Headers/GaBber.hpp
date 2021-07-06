#pragma once
#include <SDL2/SDL.h>
#include "GL/gl3w.h"

#include "CPU/Unions.hpp"
#include "Devices/RAM.hpp"
#include "Debugger/Debugger.hpp"
#include "Devices/GamePak.hpp"
#include "Devices/SystemBIOS.hpp"
#include "Devices/DebugBackdoor.hpp"
#include "Headers/ARM7TDMI.hpp"
#include "MMU/MMU.hpp"
#include "PPU/PPU.hpp"
#include "Tests/Harness.hpp"

class GaBber {
	friend class Debugger;

	std::string m_rom_filename;

	MMU m_mmu;
	ARM7TDMI m_cpu;
	PPU m_ppu;
    OnboardWRAM m_onboard_wram;
    OnchipWRAM m_onchip_wram;
	GamePak m_pak;
    TestHarness m_test_harness;
	SystemBIOS m_bios;
	Backdoor m_backdoor;
	bool m_test_mode;

    Debugger m_debugger;
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
public:
	GaBber()
	: m_mmu(), m_cpu(m_mmu), m_ppu(m_cpu, m_mmu), m_test_harness(*this), m_debugger(*this) {}

	void parse_args(int argc, char** argv);
    int start();

    MMU& mmu() { return m_mmu; }
	ARM7TDMI& cpu() { return m_cpu; }
	PPU& ppu() { return m_ppu; }
};