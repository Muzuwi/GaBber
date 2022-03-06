#pragma once
#include <fmt/format.h>
#include <vector>
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

class BusDevice;

class BusInterface : Module {
	friend class BusDevice;
	friend class TestHarness;

	std::vector<BusDevice*> m_devices;
	bool register_device(BusDevice&);

	unsigned m_last_wait_cycles;
	BusDevice* find_device(uint32 address, size_t size);
public:
	BusInterface(GaBber&);
	template<typename... Args>
	static void log(const char* format, const Args&... args) {
		return;

		fmt::print("\u001b[93mMMU/ ");
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}

	unsigned last_wait_cycles() const { return m_last_wait_cycles; }

	uint32 read32(uint32 address);
	uint16 read16(uint32 address);
	uint8 read8(uint32 address);

	void write32(uint32 address, uint32 value);
	void write16(uint32 address, uint16 value);
	void write8(uint32 address, uint8 value);

	uint8 peek(uint32 address);
	void poke(uint32 address, uint8 val);

	void reload();

	void debug();
};