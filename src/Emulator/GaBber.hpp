#pragma once
#include <array>
#include <memory>
#include "Emulator/Config.hpp"

class Debugger;
class BusInterface;
class ARM7TDMI;
class PPU;
class TestHarness;
class MemoryLayout;
class APU;
class Renderer;

class GaBber {
	friend class Module;
	std::string m_rom_filename;
	Config m_config;

	std::shared_ptr<TestHarness> m_test_harness;
	std::shared_ptr<MemoryLayout> m_mem;
	std::shared_ptr<BusInterface> m_mmu;
	std::shared_ptr<Debugger> m_debugger;
	std::shared_ptr<ARM7TDMI> m_cpu;
	std::shared_ptr<PPU> m_ppu;
	std::shared_ptr<APU> m_sound;
	std::shared_ptr<Renderer> m_renderer;
	bool m_test_mode;
	bool m_running { true };
	bool m_do_step { false };

	bool m_closed;
	unsigned m_current_sample;
	std::array<unsigned, 10000> m_cycle_samples;

	void emulator_reset();
	void emulator_loop();
	void emulator_next_state();
	void emulator_close();

	GaBber();
public:
	static GaBber& instance() {
		static GaBber emu;
		return emu;
	}

	void parse_args(int argc, char** argv);
	int start();

	BusInterface& mmu() { return *m_mmu; }
	ARM7TDMI& cpu() { return *m_cpu; }
	PPU& ppu() { return *m_ppu; }
	Debugger& debugger() { return *m_debugger; }
	MemoryLayout& mem() { return *m_mem; }
	APU& sound() { return *m_sound; }
	Config& config() { return m_config; }
	Renderer& renderer() { return *m_renderer; }

	void toggle_debug_mode();
	void enter_debug_mode();

	void single_step() { m_do_step = true; }
	void resume() { m_running = true; }
	void close() { m_closed = true; }

	std::array<unsigned, 10000> const& cycle_samples() const { return m_cycle_samples; }
};