#pragma once
#include "IO/BG.hpp"
#include "IO/DebugBackdoor.hpp"
#include "IO/DMA.hpp"
#include "IO/Interrupt.hpp"
#include "IO/Keypad.hpp"
#include "IO/PPU.hpp"
#include "IO/Sound.hpp"
#include "IO/Timer.hpp"
#include "PPU/BG.hpp"

struct IOContainer {
	DISPCNT dispcnt;
	GreenSwap green_swap;
	DISPSTAT dispstat;
	VCOUNT vcount;
	BLDCNT bldcnt;
	BLDALPHA bldalpha;
	BLDY bldy;

	WindowH<0> win0h;
	WindowH<1> win1h;
	WindowV<0> win0v;
	WindowV<1> win1v;
	WindowInside winin;
	WindowOutside winout;
	Mosaic mosaic;

	BG<0> bg0;
	BG<1> bg1;
	BG<2> bg2;
	BG<3> bg3;

	Sound1CtlH ch1ctlH;
	Sound1CtlL ch1ctlL;
	Sound1CtlX ch1ctlX;
	Sound2CtlL ch2ctlL;
	Sound2CtlH ch2ctlH;
	Sound3CtlL ch3ctlL;
	Sound3CtlH ch3ctlH;
	Sound3CtlX ch3ctlX;
	Sound3Bank ch3bank;
	Sound4CtlL ch4ctlL;
	Sound4CtlH ch4ctlH;
	SoundCtlL soundctlL;
	SoundCtlH soundctlH;
	SoundCtlX soundctlX;
	SoundBias soundbias;
	SoundFifoA fifoA;
	SoundFifoB fifoB;

	DMAx<0> dma0;
	DMAx<1> dma1;
	DMAx<2> dma2;
	DMAx<3> dma3;

	Timer<0> timer0;
	Timer<1> timer1;
	Timer<2> timer2;
	Timer<3> timer3;

	Keypad keyinput;
	KeypadCnt keycnt;

	IE ie;
	IF if_;
	WaitControl waitctl;
	IME ime;
	POSTFLG postflg;
	HALTCNT haltcnt;
	MemCtl memctl;
	Backdoor backdoor;

	EmptyReg<0x0400008C> reg8C;
	EmptyReg<0x040000E0> regE0;
	EmptyReg<0x04000110> reg110;
	IOReg16<0x04000410> reg410;
	IOReg32<0x04000804> reg804;

	template<unsigned timer_num>
	Timer<timer_num>& timer_for_num() {
		if constexpr(timer_num == 0)
			return timer0;
		else if constexpr(timer_num == 1)
			return timer1;
		else if constexpr(timer_num == 2)
			return timer2;
		else
			return timer3;
	}

	template<unsigned x>
	constexpr DMAx<x>& dma_for_num() {
		if constexpr(x == 0)
			return dma0;
		else if constexpr(x == 1)
			return dma1;
		else if constexpr(x == 2)
			return dma2;
		else
			return dma3;
	}
};