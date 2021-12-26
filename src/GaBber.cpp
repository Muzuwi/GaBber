#include <iostream>
#include <fstream>
#include "Headers/GaBber.hpp"
#include "MMU/IOReg.hpp"

Optional<Vector<uint8>> load_from_file(const std::string& path){
	std::ifstream file;
	std::vector<char> temp;
	file.open(path, std::ios::binary);
	if(!file.good()) {
		return {};
	}

	file.seekg(0, file.end);
	size_t fileSize = file.tellg();
	file.seekg(0, file.beg);
	temp.resize(fileSize);
	file.read(&temp[0], fileSize);
	file.close();
	Vector<uint8> rom(temp.begin(), temp.end());

	return {rom};
}


void GaBber::parse_args(int argc, char** argv) {
	for(int i = 1; i < argc; ++i) {
		if(std::string("--debug").compare(argv[i]) == 0) {
			m_debugger.set_debug_mode(true);
		}
		if(std::string("--test").compare(argv[i]) == 0) {
			m_test_mode = true;
		}
	}

	if(argc >= 2) {
		m_rom_filename = std::string(argv[1]);
	}
}


int GaBber::start() {
	auto bios_image = load_from_file("bios.bin");
	if(!bios_image.has_value()) {
		std::cerr << "Failed loading BIOS image!";
		return -1;
	}
	m_mem.bios.from_vec(*bios_image);

	if(!m_test_mode) {
		auto rom = load_from_file(m_rom_filename);
		if(!rom.has_value()) {
			std::cerr << "Failed loading ROM from file `" << m_rom_filename << "`\n";
			return -1;
		}

		Vector<uint8> save = {};
		auto maybe_save = load_from_file(m_rom_filename + ".sav");
		if(maybe_save.has_value()) {
			save = *maybe_save;
		} else {
			std::cerr << "Could not load save file!\n";
		}

		m_mem.pak.load_pak(std::move(*rom), std::move(save));
	}

	emulator_reset();

	if(m_test_mode) {
		m_test_harness.run_emulator_tests();
		return 0;
	}

	if(!display_initialize()) {
		std::cerr << "Failed initializing display subsystem\n";
		return -1;
	}
	m_sound.init();

	emulator_loop();
	emulator_close();

	return 0;
}


void GaBber::emulator_loop() {
	while(!m_closed) {
		if(m_running || m_do_step) {
			emulator_next_state();
			m_do_step = false;
		}

		if(m_ppu.frame_ready() || !m_running) {
			display_update();
			m_ppu.clear_frame_ready();
		}
	}
}

void GaBber::emulator_next_state() {
	const unsigned cycles = m_cpu.run_next_instruction();
	assert(cycles > 0 && "Trying to emulate zero cycles!");
	for(unsigned i = 0; i < cycles; ++i) {
		m_ppu.cycle();
		const auto count = m_sound.speed_scale();
		for(unsigned j = 0; j < (unsigned)count; ++j) {
			m_sound.cycle();
		}
	}

	m_cycle_samples[m_current_sample++] = cycles;
	if(mem().io.haltcnt.m_halt) {
		m_cycle_samples[m_current_sample-1] += 1000;
	}
	if(m_current_sample == 10000) {
		m_current_sample = 0;
	}
}

void GaBber::toggle_debug_mode() {
	if(m_debugger.is_debug_mode()) {
		m_debugger.set_debug_mode(false);
		m_running = true;

		SDL_SetWindowResizable(m_gabberWindow, SDL_FALSE);
		SDL_RestoreWindow(m_gabberWindow);
	} else {
		enter_debug_mode();
	}
}

void GaBber::enter_debug_mode() {
	m_debugger.set_debug_mode(true);
	m_running = false;

	SDL_SetWindowResizable(m_gabberWindow, SDL_TRUE);
	SDL_MaximizeWindow(m_gabberWindow);
}


void GaBber::emulator_reset() {
	m_cpu.reset();
	m_mmu.reload();
}

void GaBber::emulator_close() {
	std::ofstream save_file {m_rom_filename + ".sav", std::ios_base::binary};
	if(!save_file.good()) {
		fmt::print("Failed opening save file!\n");
		return;
	}

	auto const& cart = mem().pak.sram.cart();
	if(!cart) {
		return;
	}

	auto const& buffer = cart->to_vec();
	save_file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());
	save_file.close();
}
