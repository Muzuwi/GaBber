#pragma once
#include "MMU/IOReg.hpp"
#include "IO/PPU.hpp"

class Keypad final : public IOReg16<0x04000130> {
	void on_write(uint16) override {}
public:
	enum class State {
		Pressed,
		Released
	};

	void reload() override {
		m_register = 0x3ff;
	}

	bool pressed(KeypadKey key) const {
		return (m_register & (1u << (uint16)key)) == 0;
	}

	void set(KeypadKey key, State state) {
		const uint16 mask = (1u << (uint16)key);
		const uint16 val  = m_register & ~mask;
		m_register = val | (state == State::Pressed ? 0 : mask);
	}
};


class KeypadCnt final : public IOReg16<0x04000132> {
public:
	bool selected(KeypadKey key) {
		return (m_register & (1u << (uint16)key));
	}

	bool irq_enable() const {
		return (m_register & (1u << 14u));
	}

	bool irq_condition() const {
		return (m_register & (1u << 15u));
	}
};
