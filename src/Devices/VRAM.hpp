#pragma once
#include "MMU/ArrayMem.hpp"


class OAM final : public ArrayMem<0x07000000, 0x08000000, 1*kB> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::R; }

	uint32 handle_offset(uint32 offset, size_t) const override {
		return offset & 0x3ff;
	}

	std::string identify() const override {
		return "OAM";
	}

	unsigned int waitcycles32() const override {
		return 1;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}
};


class PaletteRAM final : public ArrayMem<0x05000000, 0x06000000, 1*kB> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::R; }

	uint32 handle_offset(uint32 offset, size_t) const override {
		return offset & 0x3ff;
	}

	std::string identify() const override {
		return "PaletteRAM";
	}

	unsigned int waitcycles32() const override {
		return 2;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}
};


//  FIXME: Mirroring
class VRAM final : public ArrayMem<0x06000000, 0x07000000, 96*kB> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::R; }

	uint32 handle_offset(uint32 offset, size_t) const override {
		return offset % (96*kB);
	}

	std::string identify() const override {
		return "VRAM";
	}

	unsigned int waitcycles32() const override {
		return 2;
	}

	unsigned int waitcycles16() const override {
		return 1;
	}

	unsigned int waitcycles8() const override {
		return 1;
	}
};
