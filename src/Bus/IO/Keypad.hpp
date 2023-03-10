#pragma once
#include "Bus/Common/IOReg.hpp"
#include "PPU.hpp"

enum class KeypadKey {
	A = 0,
	B,
	Sel,
	Start,
	Right,
	Left,
	Up,
	Down,
	R,
	L = 9
};

class Keypad final : public IOReg16<0x04000130> {
	void on_write(uint16) override {}
public:
	Keypad(GaBber& emu)
	    : IOReg16<67109168>(emu) {}

	enum class State {
		Pressed,
		Released
	};

	void reload() override { m_register = 0x3ff; }

	bool pressed(KeypadKey key) const { return (m_register & (1u << (uint16)key)) == 0; }

	void set(KeypadKey key, State state) {
		const uint16 mask = (1u << (uint16)key);
		const uint16 val = m_register & ~mask;
		m_register = val | (state == State::Pressed ? 0 : mask);
	}
};

class KeypadCnt final : public IOReg16<0x04000132> {
	void on_write(uint16 new_value) override { m_register = new_value & ~0x3c00; }
	uint16 on_read() override { return m_register & ~0x3c00; }
public:
	KeypadCnt(GaBber& emu)
	    : IOReg16<67109170>(emu) {}

	bool selected(KeypadKey key) { return (m_register & (1u << (uint16)key)); }

	bool irq_enable() const { return (m_register & (1u << 14u)); }

	bool irq_condition() const { return (m_register & (1u << 15u)); }
};
