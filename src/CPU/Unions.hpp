#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/IOReg.hpp"

union TimerReg {
	struct {
		uint8 prescaler     : 2;
		bool  count_up      : 1;
		uint8 _unused       : 3;
		bool  irq_enable    : 1;
		bool  timer_enable  : 1;
		uint8 _unused_2     : 8;
	} __attribute__((packed)) m_reg;

	uint16 m_raw;
};

union IEReg {
	struct {
		bool lcd_vblank  : 1;
		bool lcd_hblank  : 1;
		bool lcd_vcounter : 1;
		bool timer0_ov   : 1;
		bool timer1_ov   : 1;
		bool timer2_ov   : 1;
		bool timer3_ov   : 1;
		bool serial      : 1;
		bool dma0        : 1;
		bool dma1        : 1;
		bool dma2        : 1;
		bool dma3        : 1;
		bool keypad      : 1;
		bool gamepak     : 1;
		bool _unused     : 2;
	} __attribute__((packed)) m_reg;

	uint16 m_raw;
};


union IFReg {
	struct {
		bool lcd_vblank  : 1;
		bool lcd_hblank  : 1;
		bool lcd_vcounter : 1;
		bool timer0_ov   : 1;
		bool timer1_ov   : 1;
		bool timer2_ov   : 1;
		bool timer3_ov   : 1;
		bool serial      : 1;
		bool dma0        : 1;
		bool dma1        : 1;
		bool dma2        : 1;
		bool dma3        : 1;
		bool keypad      : 1;
		bool gamepak     : 1;
		bool _unused     : 2;
	} __attribute__((packed)) m_reg;

	uint16 m_raw;
};
