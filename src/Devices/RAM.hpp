#pragma once
#include "MMU/ArrayMem.hpp"


class OnboardWRAM final : public ArrayMem<0x02000000, 0x03000000, 256 * kB> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::RW; }

	uint32 handle_offset(uint32 offset, size_t) const override {
		return offset & 0x3ffffu;
	}

	std::string identify() const override {
		return "On-board WRAM";
	}

	unsigned int waitcycles32() const override {
		return 6;
	}

	unsigned int waitcycles16() const override {
		return 3;
	}

	unsigned int waitcycles8() const override {
		return 3;
	}
};


class OnchipWRAM final : public ArrayMem<0x03000000, 0x04000000, 32 * kB> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::RW; }

	uint32 handle_offset(uint32 offset, size_t) const override {
		return offset & 0x7fffu;
	}

	std::string identify() const override {
		return "On-Chip WRAM";
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
