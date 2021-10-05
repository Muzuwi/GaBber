#pragma once
#include "MMU/ArrayMem.hpp"

//  FIXME: Locking down the BIOS in userland
class SystemBIOS final : public ArrayMem<0x0, 0x4000, 0x4000> {
public:
	SystemBIOS()
	: ArrayMem<0, 16384, 16384>() {}

	void load_from_vec(Vector<uint8> const& v) {
		for(unsigned i = 0; i < v.size() && i < 0x4000; ++i) {
			m_buffer[i] = v[i];
		}
	}

	IOAccess access32() const override { return IOAccess::R; }
	IOAccess access16() const override { return IOAccess::R; }
	IOAccess access8() const override { return IOAccess::R; }

	std::string identify() const override {
		return "BIOS";
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
