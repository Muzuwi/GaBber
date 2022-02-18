#pragma once
#include "Emulator/StdTypes.hpp"

class GaBber;
class Debugger;
class ARM7TDMI;
class APU;
class PPU;
class BusInterface;
struct MemoryLayout;
struct IOContainer;

class Module {
protected:
	GaBber& m_emu;
public:
	Module(GaBber&);

	MemoryLayout& mem() const;
	IOContainer& io() const;
	Debugger& debugger() const;
	ARM7TDMI& cpu() const;
	APU& apu() const;
	PPU& ppu() const;
	BusInterface& bus() const;
};