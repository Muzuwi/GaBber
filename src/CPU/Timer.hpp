#pragma once
#include "CPU/Unions.hpp"


template<unsigned base_address>
struct TimerReload final : public IOReg<base_address, _DummyReg<uint16>, IOAccess::RW> {
	uint16 m_reload {0};

	void on_write(uint16 val) override {
		m_reload = val;
	}
};

template<unsigned timer_number>
struct Timer {
	static_assert(timer_number < 4, "Invalid timer number");

	unsigned m_timer_cycles {0};
	TimerReload<0x04000100 + timer_number*4> m_reload_and_current;
	IOReg<0x04000102 + timer_number*4, TimerReg, IOAccess::RW> m_ctl;
};

class ARM7TDMI;

class Timers {
	ARM7TDMI& m_cpu;

	Timer<0> m_timer0;
	Timer<1> m_timer1;
	Timer<2> m_timer2;
	Timer<3> m_timer3;

	template<unsigned timer_num>
	void cycle_timer(Timer<timer_num>& timer);

	template<unsigned timer_num>
	void increment_timer(Timer<timer_num>& timer);

	unsigned cycle_count_from_prescaler(uint8 prescaler) {
		const unsigned val[4] { 1, 64, 256, 1024 };

		if(prescaler > 3) return 1;
		else              return val[prescaler];
	}
public:
	Timers(ARM7TDMI& v)
	: m_cpu(v) {}

	void cycle();
};