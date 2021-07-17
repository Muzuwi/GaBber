#pragma once
#include "Headers/StdTypes.hpp"

struct TimerReg {
	uint8 prescaler     : 2;
	bool  count_up      : 1;
	uint8 _unused       : 3;
	bool  irq_enable    : 1;
	bool  timer_enable  : 1;
	uint8 _unused_2     : 8;
} __attribute__((packed));
