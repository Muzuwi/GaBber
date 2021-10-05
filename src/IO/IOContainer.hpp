#pragma once
#include "IO/BG.hpp"
#include "IO/DebugBackdoor.hpp"
#include "IO/DMA.hpp"
#include "IO/Interrupt.hpp"
#include "IO/Keypad.hpp"
#include "IO/PPU.hpp"
#include "IO/Timer.hpp"
#include "IO/Sound.hpp"
#include "PPU/BG.hpp"

struct IOContainer {
	DISPCNT dispcnt;        //	4000000h  2    R/W  DISPCNT   LCD Control
	GreenSwap green_swap;   //	4000002h  2    R/W  -         Undocumented - Green Swap
	DISPSTAT dispstat;  	//	4000004h  2    R/W  DISPSTAT  General LCD Status (STAT,LYC)
	VCOUNT vcount;          //	4000006h  2    R    VCOUNT    Vertical Counter (LY)


	//	4000008h  2    R/W  BG0CNT    BG0 Control
	//	400000Ah  2    R/W  BG1CNT    BG1 Control
	//	400000Ch  2    R/W  BG2CNT    BG2 Control
	//	400000Eh  2    R/W  BG3CNT    BG3 Control
	//	4000010h  2    W    BG0HOFS   BG0 X-Offset
	//	4000012h  2    W    BG0VOFS   BG0 Y-Offset
	//	4000014h  2    W    BG1HOFS   BG1 X-Offset
	//	4000016h  2    W    BG1VOFS   BG1 Y-Offset
	//	4000018h  2    W    BG2HOFS   BG2 X-Offset
	//	400001Ah  2    W    BG2VOFS   BG2 Y-Offset
	//	400001Ch  2    W    BG3HOFS   BG3 X-Offset
	//	400001Eh  2    W    BG3VOFS   BG3 Y-Offset
	//	4000020h  2    W    BG2PA     BG2 Rotation/Scaling Parameter A (dx)
	//	4000022h  2    W    BG2PB     BG2 Rotation/Scaling Parameter B (dmx)
	//	4000024h  2    W    BG2PC     BG2 Rotation/Scaling Parameter C (dy)
	//	4000026h  2    W    BG2PD     BG2 Rotation/Scaling Parameter D (dmy)
	//	4000028h  4    W    BG2X      BG2 Reference Point X-Coordinate
	//	400002Ch  4    W    BG2Y      BG2 Reference Point Y-Coordinate
	//	4000030h  2    W    BG3PA     BG3 Rotation/Scaling Parameter A (dx)
	//	4000032h  2    W    BG3PB     BG3 Rotation/Scaling Parameter B (dmx)
	//	4000034h  2    W    BG3PC     BG3 Rotation/Scaling Parameter C (dy)
	//	4000036h  2    W    BG3PD     BG3 Rotation/Scaling Parameter D (dmy)
	//	4000038h  4    W    BG3X      BG3 Reference Point X-Coordinate
	//	400003Ch  4    W    BG3Y      BG3 Reference Point Y-Coordinate
	BG<0> bg0;
	BG<1> bg1;
	BG<2> bg2;
	BG<3> bg3;


	//	4000040h  2    W    WIN0H     Window 0 Horizontal Dimensions
	//	4000042h  2    W    WIN1H     Window 1 Horizontal Dimensions
	//	4000044h  2    W    WIN0V     Window 0 Vertical Dimensions
	//	4000046h  2    W    WIN1V     Window 1 Vertical Dimensions
	//	4000048h  2    R/W  WININ     Inside of Window 0 and 1
	//	400004Ah  2    R/W  WINOUT    Inside of OBJ Window & Outside of Windows
	//	400004Ch  2    W    MOSAIC    Mosaic Size
	//	400004Eh       -    -         Not used
	//	4000050h  2    R/W  BLDCNT    Color Special Effects Selection
	//	4000052h  2    R/W  BLDALPHA  Alpha Blending Coefficients
	//	4000054h  2    W    BLDY      Brightness (Fade-In/Out) Coefficient
	//	4000056h       -    -         Not used
	//	4000060h  2  R/W  SOUND1CNT_L Channel 1 Sweep register       (NR10)
	//	4000062h  2  R/W  SOUND1CNT_H Channel 1 Duty/Length/Envelope (NR11, NR12)
	//	4000064h  2  R/W  SOUND1CNT_X Channel 1 Frequency/Control    (NR13, NR14)
	//	4000066h     -    -           Not used
	//	4000068h  2  R/W  SOUND2CNT_L Channel 2 Duty/Length/Envelope (NR21, NR22)
	//	400006Ah     -    -           Not used
	//	400006Ch  2  R/W  SOUND2CNT_H Channel 2 Frequency/Control    (NR23, NR24)
	//	400006Eh     -    -           Not used
	//	4000070h  2  R/W  SOUND3CNT_L Channel 3 Stop/Wave RAM select (NR30)
	//	4000072h  2  R/W  SOUND3CNT_H Channel 3 Length/Volume        (NR31, NR32)
	//	4000074h  2  R/W  SOUND3CNT_X Channel 3 Frequency/Control    (NR33, NR34)
	//	4000076h     -    -           Not used
	//	4000078h  2  R/W  SOUND4CNT_L Channel 4 Length/Envelope      (NR41, NR42)
	//	400007Ah     -    -           Not used
	//	400007Ch  2  R/W  SOUND4CNT_H Channel 4 Frequency/Control    (NR43, NR44)
	//	400007Eh     -    -           Not used
	//	4000080h  2  R/W  SOUNDCNT_L  Control Stereo/Volume/Enable   (NR50, NR51)
	//	4000082h  2  R/W  SOUNDCNT_H  Control Mixing/DMA Control
	//	4000084h  2  R/W  SOUNDCNT_X  Control Sound on/off           (NR52)
	//	4000086h     -    -           Not used
	//	4000088h  2  BIOS SOUNDBIAS   Sound PWM Control
	//	400008Ah  ..   -    -         Not used
	//	4000090h 2x10h R/W  WAVE_RAM  Channel 3 Wave Pattern RAM (2 banks!!)
	//	40000A0h  4    W    FIFO_A    Channel A FIFO, Data 0-3
	//	40000A4h  4    W    FIFO_B    Channel B FIFO, Data 0-3
	//	40000A8h       -    -         Not used
	//  TODO:
	IOReg32<0x04000150> serialthing;

	Sound1CtlH ch1ctlH;
	Sound1CtlL ch1ctlL;
	Sound1CtlX ch1ctlX;
	Sound2CtlL ch2ctlL;
	Sound2CtlH ch2ctlH;
	Sound3CtlL ch3ctlL;
	Sound3CtlH ch3ctlH;
	Sound3CtlX ch3ctlX;
	Sound3Bank ch3bank;
	IOReg16<0x0400006a> reg6a;
	SoundCtlL soundcntL;

	IOReg16<0x04000082> soundcntH;
	IOReg16<0x04000088> soundbias;


	//	40000B0h  4    W    DMA0SAD   DMA 0 Source Address
	//	40000B4h  4    W    DMA0DAD   DMA 0 Destination Address
	//	40000B8h  2    W    DMA0CNT_L DMA 0 Word Count
	//	40000BAh  2    R/W  DMA0CNT_H DMA 0 Control
	//	40000BCh  4    W    DMA1SAD   DMA 1 Source Address
	//	40000C0h  4    W    DMA1DAD   DMA 1 Destination Address
	//	40000C4h  2    W    DMA1CNT_L DMA 1 Word Count
	//	40000C6h  2    R/W  DMA1CNT_H DMA 1 Control
	//	40000C8h  4    W    DMA2SAD   DMA 2 Source Address
	//	40000CCh  4    W    DMA2DAD   DMA 2 Destination Address
	//	40000D0h  2    W    DMA2CNT_L DMA 2 Word Count
	//	40000D2h  2    R/W  DMA2CNT_H DMA 2 Control
	//	40000D4h  4    W    DMA3SAD   DMA 3 Source Address
	//	40000D8h  4    W    DMA3DAD   DMA 3 Destination Address
	//	40000DCh  2    W    DMA3CNT_L DMA 3 Word Count
	//	40000DEh  2    R/W  DMA3CNT_H DMA 3 Control
	DMAx<0> dma0;
	DMAx<1> dma1;
	DMAx<2> dma2;
	DMAx<3> dma3;


	IOReg16<0x040000E0> regE0;      //	40000E0h       -    -         Not used


	//	4000100h  2    R/W  TM0CNT_L  Timer 0 Counter/Reload
	//	4000102h  2    R/W  TM0CNT_H  Timer 0 Control
	//	4000104h  2    R/W  TM1CNT_L  Timer 1 Counter/Reload
	//	4000106h  2    R/W  TM1CNT_H  Timer 1 Control
	//	4000108h  2    R/W  TM2CNT_L  Timer 2 Counter/Reload
	//	400010Ah  2    R/W  TM2CNT_H  Timer 2 Control
	//	400010Ch  2    R/W  TM3CNT_L  Timer 3 Counter/Reload
	//	400010Eh  2    R/W  TM3CNT_H  Timer 3 Control
	Timer<0> timer0;
	Timer<1> timer1;
	Timer<2> timer2;
	Timer<3> timer3;


	//	4000110h       -    -         Not used
	//	4000120h  4    R/W  SIODATA32 SIO Data (Normal-32bit Mode; shared with below)
	//	4000120h  2    R/W  SIOMULTI0 SIO Data 0 (Parent)    (Multi-Player Mode)
	//	4000122h  2    R/W  SIOMULTI1 SIO Data 1 (1st Child) (Multi-Player Mode)
	//	4000124h  2    R/W  SIOMULTI2 SIO Data 2 (2nd Child) (Multi-Player Mode)
	//	4000126h  2    R/W  SIOMULTI3 SIO Data 3 (3rd Child) (Multi-Player Mode)
	//	4000128h  2    R/W  SIOCNT    SIO Control Register
	//	400012Ah  2    R/W  SIOMLT_SEND SIO Data (Local of MultiPlayer; shared below)
	//	400012Ah  2    R/W  SIODATA8  SIO Data (Normal-8bit and UART Mode)
	//	400012Ch       -    -         Not used


	Keypad keyinput;	//	4000130h  2    R    KEYINPUT  Key Status
	KeypadCnt keycnt;	//	4000132h  2    R/W  KEYCNT    Key Interrupt Control


	//	4000134h  2    R/W  RCNT      SIO Mode Select/General Purpose Data
	//	4000136h  -    -    IR        Ancient - Infrared Register (Prototypes only)
	//	4000138h       -    -         Not used
	//	4000140h  2    R/W  JOYCNT    SIO JOY Bus Control
	//	4000142h       -    -         Not used
	//	4000150h  4    R/W  JOY_RECV  SIO JOY Bus Receive Data
	//	4000154h  4    R/W  JOY_TRANS SIO JOY Bus Transmit Data
	//	4000158h  2    R/?  JOYSTAT   SIO JOY Bus Receive Status
	//	400015Ah       -    -         Not used


	IE ie;                          //	4000200h  2    R/W  IE        Interrupt Enable Register
	IF if_;                         //	4000202h  2    R/W  IF        Interrupt Request Flags / IRQ Acknowledge
	IOReg16<0x04000204> waitcnt;    //	4000204h  2    R/W  WAITCNT   Game Pak Waitstate Control
	IOReg16<0x04000206> reg206;     //	4000206h       -    -         Not used
	IME ime;                        //	4000208h  2    R/W  IME       Interrupt Master Enable Register
	IOReg16<0x0400020A> reg20A;     //	400020Ah       -    -         Not used
	POSTFLG postflg;                //	4000300h  1    R/W  POSTFLG   Undocumented - Post Boot Flag
	HALTCNT haltcnt;                //	4000301h  1    W    HALTCNT   Undocumented - Power Down Control
	IOReg16<0x04000302> reg302;     //	4000302h       -    -         Not used FIXME: ?
	IOReg16<0x04000410> reg410;     //	4000410h  ?    ?    ?         Undocumented - Purpose Unknown / Bug ??? 0FFh
									//	4000411h       -    -         Not used
	IOReg32<0x04000800> memctl;     //	4000800h  4    R/W  ?         Undocumented - Internal Memory Control (R/W)
	IOReg32<0x04000804> reg804;     //	4000804h       -    -         Not used
									//	4xx0800h  4    R/W  ?         Mirrors of 4000800h (repeated each 64K)

	Backdoor backdoor;



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
		if constexpr (x == 0)
			return dma0;
		else if constexpr(x == 1)
			return dma1;
		else if constexpr(x == 2)
			return dma2;
		else
			return dma3;
	}

};