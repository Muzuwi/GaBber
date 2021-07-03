#pragma once
#include "Headers/StdTypes.hpp"

/*
 *  General purpose registers
 */
struct GPR {
	//  System/User instruction_mode registers
	uint32 m_base[16];

	//  R8-R14 FIQ regs
	uint32 m_gFIQ[7];

	//  R13-R14 SVC regs
	uint32 m_gSVC[2];

	//  R13-R14 ABT regs
	uint32 m_gABT[2];

	//  R13-R14 IRQ regs
	uint32 m_gIRQ[2];

	//  R13-R14 UND regs
	uint32 m_gUND[2];
};