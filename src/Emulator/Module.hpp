#pragma once
#include "Emulator/StdTypes.hpp"

class GaBber;
class Debugger;
class ARM7TDMI;
class APU;
class PPU;
class BusInterface;
struct Config;
struct MemoryLayout;
struct IOContainer;

class Module {
protected:
	GaBber& m_emu;
public:
	Module(GaBber&);

	Config& config() const;
	MemoryLayout& mem() const;
	IOContainer& io() const;
	Debugger& debugger() const;
	ARM7TDMI& cpu() const;
	APU& apu() const;
	PPU& ppu() const;
	BusInterface& bus() const;
};