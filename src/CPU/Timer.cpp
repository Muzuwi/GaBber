#include "IO/Timer.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Headers/GaBber.hpp"


void ARM7TDMI::timers_cycle_all(size_t n) {
	timers_cycle_n(io.timer0, n);
	timers_cycle_n(io.timer1, n);
	timers_cycle_n(io.timer2, n);
	timers_cycle_n(io.timer3, n);
}


template<unsigned int timer_num>
void ARM7TDMI::timers_cycle_n(Timer<timer_num>& timer, size_t n) {
	if(!timer.m_ctl->timer_enable) {
		timer.m_previous_cycle_was_running = false;
		return;
	}
	if(timer.m_ctl->count_up)
		return;

	//  On changing start bit 0 -> 1, copy the reload value to the counter
	if(!timer.m_previous_cycle_was_running) {
		*timer.m_reload_and_current = timer.m_reload_and_current.reload_value();
		timer.m_previous_cycle_was_running = true;
	}

	timer.m_timer_cycles += n;
	auto cycles_per_tick = Timer<timer_num>::cycle_count_from_prescaler(timer.m_ctl->prescaler);
	while(timer.m_timer_cycles >= cycles_per_tick) {
		timer.m_timer_cycles -= cycles_per_tick;
		timers_increment(timer);
	}
}


template<unsigned int timer_num>
void ARM7TDMI::timers_increment(Timer<timer_num>& timer) {
	//  Timer overflow
	if(*timer.m_reload_and_current == 0xFFFF) {
		GaBber::instance().sound().on_timer_overflow(timer_num);

		*timer.m_reload_and_current = timer.m_reload_and_current.reload_value();
		if(timer.m_ctl->irq_enable)
			raise_irq(Timer<timer_num>::irq_for_timer());

		if constexpr(timer_num != 3) {
			Timer<timer_num+1>& other_timer = io.template timer_for_num<timer_num+1>();
			if(other_timer.m_ctl->count_up)
				timers_increment<timer_num+1>(other_timer);
		}

	} else {
		(*timer.m_reload_and_current)++;
	}
}
