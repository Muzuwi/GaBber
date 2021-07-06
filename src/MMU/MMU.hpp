#pragma once
#include <fmt/format.h>
#include "Headers/StdTypes.hpp"

class BusDevice;

class MMU {
	friend class BusDevice;
	friend class Debugger;
	friend class TestHarness;

	static Vector<BusDevice*> s_devices;

	BusDevice* find_device(uint32 address);

	uint8 peek(uint32 address);

	uint64 cache_hit {0};
	uint64 cache_miss {0};
	static bool register_device(BusDevice&);
public:

	template<typename... Args>
	static void log(const char* format, const Args& ...args) {
//		return;

		fmt::print("\u001b[93mMMU/ ");
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}

	uint32 read32(uint32 address);
	uint16 read16(uint32 address);
	uint8 read8(uint32 address);

	void write32(uint32 address, uint32 value);
	void write16(uint32 address, uint16 value);
	void write8(uint32 address, uint8 value);

	void poke(uint32 address, uint8 val);

	uint64 hits() const { return cache_hit; }
	uint64 misses() const { return cache_miss; }

	void reload_all();

	void debug();
};