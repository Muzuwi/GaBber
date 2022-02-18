#include "Module.hpp"
#include "Bus/Common/MemoryLayout.hpp"
#include "GaBber.hpp"

Module::Module(GaBber& emu)
    : m_emu(emu) {}

MemoryLayout& Module::mem() const {
	return m_emu.mem();
}

IOContainer& Module::io() const {
	return m_emu.mem().io;
}

Debugger& Module::debugger() const {
	return m_emu.debugger();
}

ARM7TDMI& Module::cpu() const {
	return m_emu.cpu();
}

APU& Module::apu() const {
	return m_emu.sound();
}

PPU& Module::ppu() const {
	return m_emu.ppu();
}

BusInterface& Module::bus() const {
	return m_emu.mmu();
}
