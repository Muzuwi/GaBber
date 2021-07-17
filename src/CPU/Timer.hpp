#pragma once
#include "CPU/Unions.hpp"
#include "MMU/IOReg.hpp"

template<unsigned x>
class TimerReload final : public IOReg16<0x04000100 + x*4> {
	uint16 m_reload {0};

	void on_write(uint16 val) override {
		m_reload = val;
	}
public:
	uint16 reload_value() const { return m_reload; }
};

template<unsigned x>
class TimerCtl final : public IOReg16<0x04000102 + x*4> {
public:
	TimerReg* operator->() {
		return this->template as<TimerReg>();
	}

	TimerReg const* operator->() const {
		return this->template as<TimerReg>();
	}
};


template<unsigned x>
struct Timer {
	static_assert(x < 4, "Invalid timer number");

	unsigned m_timer_cycles {0};
	TimerReload<x> m_reload_and_current;
	TimerCtl<x> m_ctl;
};

class ARM7TDMI;

class Timers {
	ARM7TDMI& m_cpu;

	Timer<0> m_timer0;
	Timer<1> m_timer1;
	Timer<2> m_timer2;
	Timer<3> m_timer3;

	template<unsigned timer_num>
	Timer<timer_num>& timer_for_num() {
		if constexpr(timer_num == 0)
			return m_timer0;
		else if constexpr(timer_num == 1)
			return m_timer1;
		else if constexpr(timer_num == 2)
			return m_timer2;
		else
			return m_timer3;
	}

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