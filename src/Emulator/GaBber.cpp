#include "GaBber.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>
#include "APU/APU.hpp"
#include "Bus/Common/BusInterface.hpp"
#include "Bus/Common/IOReg.hpp"
#include "Bus/Common/MemoryLayout.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Debugger/Debugger.hpp"
#include "Emulator/Renderer.hpp"
#include "PPU/PPU.hpp"

GaBber::GaBber() {
	m_mmu = std::make_shared<BusInterface>(*this);
	m_mem = std::make_shared<MemoryLayout>(*this);
	m_debugger = std::make_shared<Debugger>(*this);
	m_cpu = std::make_shared<ARM7TDMI>(*this);
	m_ppu = std::make_shared<PPU>(*this);
	m_sound = std::make_shared<APU>(*this);
	m_renderer = std::make_shared<Renderer>(*this);
}

std::optional<std::vector<uint8>> load_from_file(const std::string& path) {
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
	std::vector<uint8> rom(temp.begin(), temp.end());

	return { rom };
}

bool GaBber::parse_args(int argc, char** argv) {
	if(argc < 2) {
		fmt::print("Usage: <executable> [options] <path-to-rom>\n");
		fmt::print("Available options:\n");
		fmt::print("\t--debug\t\tStart the emulator in debug mode\n");
		fmt::print("\t--bios <path>\t\tUse the specified file as the BIOS\n");
		fmt::print("\t--save <path>\t\tUse the specified save file\n");
		fmt::print("\t--test\t\tRun emulator tests\n");
		return false;
	}

	std::vector<std::string> arguments { argv + 1, argv + argc };

	auto it = arguments.begin();

	auto skip = [&arguments, &it](unsigned count) { std::ranges::advance(it, count, arguments.end()); };

	auto peek = [&arguments, &it]() -> std::optional<std::string> {
		auto current = it;
		std::ranges::advance(current, 1, arguments.end());
		if(current == arguments.end()) {
			return { std::nullopt };
		}
		return { *current };
	};

	bool save_name_passed = false;
	bool rom_name_passed = false;

	while(it != arguments.end()) {
		if(*it == "--debug") {
			m_debugger->set_debug_mode(true);
			skip(2);
		} else if(*it == "--bios") {
			auto name = peek();
			if(name.has_value()) {
				m_bios_filename = { *name };
				skip(2);
			} else {
				fmt::print("Missing file path for argument '--bios'\n");
				return false;
			}
		} else if(*it == "--save") {
			auto path = peek();
			if(path.has_value()) {
				save_name_passed = true;
				m_save_filename = { *path };
				skip(2);
			} else {
				fmt::print("Missing file path for argument '--save'\n");
				return false;
			}
		} else if(!(*it).empty() && (*it)[0] != '-') {
			rom_name_passed = true;
			m_rom_filename = { *it };
			skip(1);
		}
	}

	if(!rom_name_passed) {
		fmt::print("No ROM file path provided\n");
		return false;
	}

	if(!save_name_passed) {
		m_save_filename = m_rom_filename + ".sav";
	}

	return true;
}

int GaBber::start() {
	auto bios_image = load_from_file(m_bios_filename);
	if(!bios_image.has_value()) {
		fmt::print("Failed loading BIOS from file '{}'\n", m_bios_filename);
		return 1;
	}
	m_mem->bios.from_vec(*bios_image);

	auto rom = load_from_file(m_rom_filename);
	if(!rom.has_value()) {
		fmt::print("Failed loading ROM from file '{}'\n", m_rom_filename);
		return 1;
	}

	std::vector<uint8> save = {};
	auto maybe_save = load_from_file(m_save_filename);
	if(maybe_save.has_value()) {
		save = *maybe_save;
	} else {
		fmt::print("Failed loading save file from file '{}'\n", m_save_filename);
	}

	m_mem->pak.load_pak(std::move(*rom), std::move(save));

	emulator_reset();

	if(!m_renderer->initialize_platform()) {
		fmt::print("Failed initializing platform renderer\n");
		return 1;
	}
	m_sound->initialize_platform();

	if(m_debugger->is_debug_mode()) {
		enter_debug_mode();
	}

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

		if(m_ppu->frame_ready() || !m_running) {
			m_renderer->update();
			m_ppu->clear_frame_ready();
		}
	}
}

void GaBber::emulator_next_state() {
	const unsigned cycles = m_cpu->run_next_instruction();
	assert(cycles > 0 && "Trying to emulate zero cycles!");
	for(unsigned i = 0; i < cycles; ++i) {
		m_ppu->cycle();
		m_sound->cycle();
	}

	m_cycle_samples[m_current_sample++] = cycles;
	if(mem().io.haltcnt.m_halt) {
		m_cycle_samples[m_current_sample - 1] += 1000;
	}
	if(m_current_sample == 10000) {
		m_current_sample = 0;
	}
}

void GaBber::toggle_debug_mode() {
	if(m_debugger->is_debug_mode()) {
		m_debugger->set_debug_mode(false);
		m_running = true;
		m_renderer->resize_to_normal();
	} else {
		enter_debug_mode();
	}
}

void GaBber::enter_debug_mode() {
	m_debugger->set_debug_mode(true);
	m_running = false;
	m_renderer->resize_to_debugger();
}

void GaBber::emulator_reset() {
	m_cpu->reset();
	m_mmu->reload();
}

void GaBber::emulator_close() {
	std::ofstream save_file { m_save_filename, std::ios_base::binary };
	if(!save_file.good()) {
		fmt::print("Failed opening save file '{}' for writing\n", m_save_filename);
		return;
	}

	auto* cart = mem().pak.sram.cart();
	if(!cart) {
		return;
	}

	auto const& buffer = cart->to_vec();
	save_file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());
	save_file.close();
}
