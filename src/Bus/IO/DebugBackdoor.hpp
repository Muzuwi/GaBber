#pragma once
#include <string>
#include "Bus/Common/IOReg.hpp"
#include "Bus/Common/ReaderArray.hpp"

class DebugBackdoor final : public BusDevice {
	static constexpr const unsigned bufsize = 0x60;
	ReaderArray<bufsize> m_buffer;
public:
	DebugBackdoor()
	    : BusDevice(0x04fff600, 0x04fff600 + bufsize)
	    , m_buffer() {}

	uint8 read8(uint32 offset) override;
	uint16 read16(uint32 offset) override;
	uint32 read32(uint32 offset) override;

	void write8(uint32 offset, uint8 value) override;
	void write16(uint32 offset, uint16 value) override;
	void write32(uint32 offset, uint32 value) override;

	void reload() override;

	std::string to_string();
};

class Backdoor final : public IOReg16<0x04fff700> {
	DebugBackdoor m_backdoor;
public:
	void on_write(uint16) override;
};