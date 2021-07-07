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
	m_bios.load_from_vec(*bios_image);

	if(!m_test_mode) {
		auto rom = load_from_file(m_rom_filename);
		if(!rom.has_value()) {
			std::cerr << "Failed loading ROM from file `" << m_rom_filename << "`\n";
			return -1;
		}

		m_pak.load_pak(std::move(*rom), {});
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

	emulator_loop();

	return 0;
}


void GaBber::emulator_loop() {
	while(!m_closed) {
		bool step = m_debugger.is_step(),
			 cont = m_debugger.continue_mode();
		if(m_debugger.is_debug_mode() && !step && !cont) {
			display_update();
			continue;
		}

		clock_cycle();
		if(step)
			m_debugger.set_step(false);

		if(m_ppu.frame_ready()) {
			display_update();
			m_ppu.clear_frame_ready();
		} else if(m_debugger.is_debug_mode()) {
			static unsigned cycles_without_display_update {0};
			cycles_without_display_update++;
			if((cycles_without_display_update % 1024768) == 0) {
				display_update();
			}
		}
	}
}


void GaBber::clock_cycle() {
	m_cpu.cycle();
	m_ppu.cycle();
}


void GaBber::toggle_debug_mode() {
	if(m_debugger.is_debug_mode()) {
		m_debugger.set_debug_mode(false);
		m_debugger.set_step(false);
		m_debugger.set_continue_mode(true);

		SDL_SetWindowResizable(m_gabberWindow, SDL_FALSE);
		SDL_RestoreWindow(m_gabberWindow);
	} else {
		m_debugger.set_debug_mode(true);
		m_debugger.set_step(true);
		m_debugger.set_continue_mode(false);

		SDL_SetWindowResizable(m_gabberWindow, SDL_TRUE);
		SDL_MaximizeWindow(m_gabberWindow);
	}
}

void GaBber::emulator_reset() {
	m_cpu.reset();
	m_mmu.reload_all();
}