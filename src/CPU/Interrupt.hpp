#pragma once
#include "Headers/StdTypes.hpp"
#include "CPU/Unions.hpp"
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

class IE final : public IOReg<0x04000200, IEReg, IOAccess::RW> {
public:
	virtual void on_write(uint16 val) override {
		m_register.m_raw = (val & ~0xc000);
	}
};

class IF final : public IOReg<0x04000202, IFReg, IOAccess::RW> {
	virtual void on_write(uint16 val) override {
		m_register.m_raw &= ~val;
	}
};

class IME final : public IOReg<0x04000208, _DummyReg<uint16>, IOAccess::RW> {
};

class HALTCNT final : public IOReg<0x04000301, _DummyReg<uint8>, IOAccess::RW> {
	virtual void on_write(uint8 val) override {
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

class POSTFLG final : public IOReg<0x04000300, _DummyReg<uint8>, IOAccess::RW> {
};