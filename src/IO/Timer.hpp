#pragma once
#include "MMU/IOReg.hpp"
#include "IO/Interrupt.hpp"

struct TimerReg {
	uint8 prescaler     : 2;
	bool  count_up      : 1;
	uint8 _unused       : 3;
	bool  irq_enable    : 1;
	bool  timer_enable  : 1;
	uint8 _unused_2     : 8;
} __attribute__((packed));


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
	bool m_previous_cycle_was_running;

	static unsigned cycle_count_from_prescaler(uint8 prescaler) {
		const unsigned val[4] { 1, 64, 256, 1024 };

		if(prescaler > 3) return 1;
		else              return val[prescaler];
	}

	static constexpr IRQType irq_for_timer() {
		if constexpr(x == 0)
			return IRQType::Timer0;
		else if constexpr(x == 1)
			return IRQType::Timer1;
		else if constexpr(x == 2)
			return IRQType::Timer2;
		else
			return IRQType::Timer3;
	}
};
