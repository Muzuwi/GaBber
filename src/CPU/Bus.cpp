#include "Headers/ARM7TDMI.hpp"
#include "MMU/BusInterface.hpp"
#include "Debugger/Debugger.hpp"

uint8 ARM7TDMI::mem_read8(uint32 address) const {
	auto v = m_mmu.read8(address);
	m_debugger.on_memory_access(address, (uint8)v, false);

	return v;
}

uint16 ARM7TDMI::mem_read16(uint32 address) const {
	auto v = m_mmu.read16(address);
	m_debugger.on_memory_access(address, (uint16)v, false);

	return v;
}

uint32 ARM7TDMI::mem_read32(uint32 address) const {
	auto v = m_mmu.read32(address);
	m_debugger.on_memory_access(address, (uint32)v, false);

	return v;
}

void ARM7TDMI::mem_write8(uint32 address, uint8 val) {
	m_debugger.on_memory_access(address, (uint8)val, true);
	m_mmu.write8(address, val);
}

void ARM7TDMI::mem_write16(uint32 address, uint16 val) {
	m_debugger.on_memory_access(address, (uint16)val, true);
	m_mmu.write16(address, val);
}

void ARM7TDMI::mem_write32(uint32 address, uint32 val) {
	m_debugger.on_memory_access(address, (uint32)val, true);
	m_mmu.write32(address, val);
}
