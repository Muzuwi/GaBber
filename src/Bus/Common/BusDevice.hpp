#pragma once
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

class BusDevice : public Module {
	uint32 m_start;
	uint32 m_end;
public:
	BusDevice(GaBber& emu, uint32 start, uint32 end) noexcept;

	virtual ~BusDevice() = default;

	virtual unsigned waitcycles32() const { return 1; }
	virtual unsigned waitcycles16() const { return 1; }
	virtual unsigned waitcycles8() const { return 1; }

	virtual uint32 read32(uint32 offset) = 0;
	virtual uint16 read16(uint32 offset) = 0;
	virtual uint8 read8(uint32 offset) = 0;

	virtual void write32(uint32 offset, uint32 value) = 0;
	virtual void write16(uint32 offset, uint16 value) = 0;
	virtual void write8(uint32 offset, uint8 value) = 0;

	uint32 start() const { return m_start; }
	uint32 size() const { return m_end - m_start; }
	uint32 end() const { return m_end; }
	bool contains(uint32 addr) const { return addr >= start() && addr < end(); }

	virtual void reload() {}
};