#include "Bus/Common/BusInterface.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Debugger/Debugger.hpp"

uint8 ARM7TDMI::mem_read8(uint32 address) const {
	auto v = bus().read8(address);
	m_wait_cycles += bus().last_wait_cycles() - 1;
	debugger().on_memory_access(address, (uint8)v, false);

	return v;
}

uint16 ARM7TDMI::mem_read16(uint32 address) const {
	auto v = bus().read16(address);
	m_wait_cycles += bus().last_wait_cycles() - 1;
	debugger().on_memory_access(address, (uint16)v, false);

	return v;
}

uint32 ARM7TDMI::mem_read32(uint32 address) const {
	auto v = bus().read32(address);
	m_wait_cycles += bus().last_wait_cycles() - 1;
	debugger().on_memory_access(address, (uint32)v, false);

	return v;
}

void ARM7TDMI::mem_write8(uint32 address, uint8 val) {
	debugger().on_memory_access(address, (uint8)val, true);
	bus().write8(address, val);
	m_wait_cycles += bus().last_wait_cycles() - 1;
}

void ARM7TDMI::mem_write16(uint32 address, uint16 val) {
	debugger().on_memory_access(address, (uint16)val, true);
	bus().write16(address, val);
	m_wait_cycles += bus().last_wait_cycles() - 1;
}

void ARM7TDMI::mem_write32(uint32 address, uint32 val) {
	debugger().on_memory_access(address, (uint32)val, true);
	bus().write32(address, val);
	m_wait_cycles += bus().last_wait_cycles() - 1;
}

uint32 ARM7TDMI::mem_read_arm_opcode(uint32 address) const {
	auto v = bus().read32(address);
	debugger().on_memory_access(address, (uint32)v, false);
	m_wait_cycles += bus().last_wait_cycles() - 1;

	return v;
}

uint16 ARM7TDMI::mem_read_thumb_opcode(uint32 address) const {
	auto v = bus().read16(address);
	debugger().on_memory_access(address, (uint16)v, false);
	m_wait_cycles += bus().last_wait_cycles() - 1;

	return v;
}
