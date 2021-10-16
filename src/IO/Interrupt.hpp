#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/IOReg.hpp"

enum class IRQType : uint8 {
	VBlank      = 0,
	HBlank,
	VCounter,
	Timer0,
	Timer1,
	Timer2,
	Timer3,
	Serial,
	DMA0,
	DMA1,
	DMA2,
	DMA3,
	Keypad,
	GamePak,
	_Reserved_14,
	_Reserved_15 = 15
};

class IE final : public IOReg16<0x04000200> {
protected:
	void on_write(uint16 val) override {
		m_register = (val & ~0xc000);
	}
	uint16 on_read() override {
		return m_register & ~0xc000;
	}
};

class IF final : public IOReg16<0x04000202> {
protected:
	void on_write(uint16 val) override {
		m_register &= ~val;
	}
	uint16 on_read() override {
		return m_register & ~0xc000;
	}
};

class IME final : public IOReg32<0x04000208> {
protected:
	void on_write(uint32 new_value) override {
		this->m_register = new_value & 1u;
	}

	uint32 on_read() override {
		return this->m_register & 1u;
	}
public:
	inline bool enabled() const {
		return m_register & 1u;
	}
};

class HALTCNT final : public IOReg8<0x04000301> {
	void on_write(uint8 val) override {
		//  Halt
		if(val == 0) {
			m_halt = true;
		}
		else if (val == 0x80)
			m_stop = true;
	}
public:
	bool m_halt {false};
	bool m_stop {false};
};

class POSTFLG final : public IOReg8<0x04000300> {

};
