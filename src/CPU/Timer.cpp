#include "CPU/Timer.hpp"
#include "Headers/ARM7TDMI.hpp"

void Timers::cycle() {
	cycle_timer(m_timer0);
	cycle_timer(m_timer1);
	cycle_timer(m_timer2);
	cycle_timer(m_timer3);
}

template<unsigned int timer_num>
void Timers::cycle_timer(Timer<timer_num>& timer) {
	if(!timer.m_ctl.reg().timer_enable)
		return;
	if(timer.m_ctl.reg().count_up)
		return;

	timer.m_timer_cycles++;
	auto cycles_per_tick = cycle_count_from_prescaler(timer.m_ctl.reg().prescaler);
	while(timer.m_timer_cycles >= cycles_per_tick) {
		timer.m_timer_cycles -= cycles_per_tick;
		increment_timer(timer);
	}
}

template<unsigned n>
IRQType irq_for_timer() {
	if constexpr(n == 0)
		return IRQType::Timer0;
	else if constexpr(n == 1)
		return IRQType::Timer1;
	else if constexpr(n == 2)
		return IRQType::Timer2;
	else
		return IRQType::Timer3;
}

template<unsigned int timer_num>
void Timers::increment_timer(Timer<timer_num>& timer) {
	//  Timer overflow
	if(timer.m_reload_and_current.raw() == 0xFFFF) {
		timer.m_reload_and_current.raw() = timer.m_reload_and_current.m_reload;
		if(timer.m_ctl.reg().irq_enable)
			m_cpu.raise_irq(irq_for_timer<timer_num>());

		if constexpr(timer_num != 3) {
			Timer<timer_num+1>& other_timer = timer_for_num<timer_num+1>();
			if(other_timer.m_ctl.reg().count_up)
				increment_timer<timer_num+1>(other_timer);
		}

	} else {
		timer.m_reload_and_current.raw()++;
	}
}
