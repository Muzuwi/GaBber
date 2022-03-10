#pragma once
#include <fmt/format.h>
#include <vector>
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

enum class AccessType {
	NonSeq,
	Seq
};

class BusDevice;

class BusInterface : Module {
	enum class Region {
		BIOS,
		IWRAM,
		IO,
		OAM,
		WRAM,
		PAL,
		VRAM,
		PAKROM0,
		PAKROM1,
		PAKROM2,
		PAKSRAM
	};

	static constexpr Region region_from_address(uint32 address) {
		if(address < 0x02000000) {
			return Region::BIOS;
		} else if(address < 0x03000000) {
			return Region::WRAM;
		} else if(address < 0x04000000) {
			return Region::IWRAM;
		} else if(address < 0x05000000) {
			return Region::IO;
		} else if(address < 0x06000000) {
			return Region::PAL;
		} else if(address < 0x07000000) {
			return Region::VRAM;
		} else if(address < 0x08000000) {
			return Region::OAM;
		} else if(address < 0x0a000000) {
			return Region::PAKROM0;
		} else if(address < 0x0c000000) {
			return Region::PAKROM1;
		} else if(address < 0x0e000000) {
			return Region::PAKROM2;
		} else {
			return Region::PAKSRAM;
		}
	}

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

	unsigned waits32(uint32 address, AccessType);
	unsigned waits16(uint32 address, AccessType);
	unsigned waits8(uint32 address, AccessType);

	void debug();
};