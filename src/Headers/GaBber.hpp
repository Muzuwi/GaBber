#pragma once
#include <array>
#include <GL/glew.h>
#include <SDL.h>
#include "CPU/ARM7TDMI.hpp"
#include "Debugger/Debugger.hpp"
#include "Headers/Config.hpp"
#include "Headers/ShellFlags.hpp"
#include "MMU/BusInterface.hpp"
#include "MMU/MemoryLayout.hpp"
#include "PPU/PPU.hpp"
#include "Sound/GBASound.hpp"
#include "Tests/Harness.hpp"

class GaBber {
	std::string m_rom_filename;

	Config m_config;

	Debugger m_debugger;
	BusInterface m_mmu;
	ARM7TDMI m_cpu;
	PPU m_ppu;
	TestHarness m_test_harness;
	MemoryLayout m_mem;
	GBASound m_sound;
	bool m_test_mode;
	bool m_running { true };
	bool m_do_step { false };

	bool m_closed;
	SDL_Window* m_gabberWindow;
	SDL_GLContext m_gabberGLContext;
	float m_last_frame_time { 0.001f };
	unsigned m_window_scale { 5 };
	GLuint m_fb, m_gba_texture;
	unsigned m_current_sample;
	std::array<unsigned, 10000> m_cycle_samples;
	ShellFlags m_shell_flags;

	void _disp_create_gl_state();
	void _disp_poll_events();
	void _disp_draw_gba_screen();
	void _disp_draw_debugger();
	void _disp_draw_common();
	void _disp_draw_frame();
	void _shell_draw_options_audio();

	void display_update();
	bool display_initialize();
	bool display_is_closed() const { return m_closed; }

	void emulator_reset();
	void emulator_loop();
	void emulator_next_state();
	void emulator_close();

	GaBber()
	    : m_debugger(*this)
	    , m_mmu()
	    , m_cpu(m_mmu, m_debugger, m_mem.io)
	    , m_ppu(m_cpu, m_mem)
	    , m_test_harness(*this) {}
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
	Config& config() { return m_config; }

	void toggle_debug_mode();
	void enter_debug_mode();
	unsigned window_scale() const { return m_window_scale; }
	GLuint gba_texture() const { return m_gba_texture; }
	bool running() const { return m_running; }

	void single_step() { m_do_step = true; }
	void resume() { m_running = true; }

	std::array<unsigned, 10000> const& cycle_samples() const {
		return m_cycle_samples;
	}
};