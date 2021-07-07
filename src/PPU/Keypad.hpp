#pragma once
#include "MMU/IOReg.hpp"
#include "PPU/Unions.hpp"

class Keypad : public IOReg<0x04000130, _DummyReg<uint16>, IOAccess::R> {
public:
	enum class State {
		Pressed,
		Released
	};

	void reload() override {
		raw() = 0x3ff;
	}

	bool pressed(KeypadKey key) const {
		return (raw() & (1u << (uint16)key)) == 0;
	}

	void set(KeypadKey key, State state) {
		const uint16 mask = (1u << (uint16)key);
		const uint16 val  = raw() & ~mask;
		raw() = val | (state == State::Pressed ? 0 : mask);
	}
};

class KeypadCnt : public IOReg<0x04000132, KEYCNTReg, IOAccess::RW> {
public:
	bool selected(KeypadKey key) {
		return (raw() & (1u << (uint16)key));
	}
};
