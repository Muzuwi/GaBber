#pragma once
#include <algorithm>
#include "MMU/IOReg.hpp"
#include "MMU/ReaderArray.hpp"

class DebugBackdoor final : public BusDevice {
	static constexpr const unsigned bufsize = 0x60;
	ReaderArray<bufsize> m_buffer;
public:
	DebugBackdoor()
	    : BusDevice(0x04fff600, 0x04fff600 + bufsize)
	    , m_buffer() {}

	uint32 read32(uint32 offset) override {
		return m_buffer.read32(offset);
	}

	uint16 read16(uint32 offset) override {
		return m_buffer.read16(offset);
	}

	uint8 read8(uint32 offset) override {
		return m_buffer.read8(offset);
	}

	void write32(uint32 offset, uint32 value) override {
		m_buffer.write32(offset, value);
	}

	void write16(uint32 offset, uint16 value) override {
		m_buffer.write16(offset, value);
	}

	void write8(uint32 offset, uint8 value) override {
		m_buffer.write8(offset, value);
	}

	void reload() override {
		std::fill_n(m_buffer.array().begin(), bufsize, 0x0);
	}

	std::string to_string() {
		std::string str;
		for(unsigned i = 0; i < 0x60; ++i) {
			if(m_buffer.array()[i] == '\0')
				break;
			str.push_back(m_buffer.array()[i]);
		}
		return str;
	}
};

class Backdoor final : public IOReg16<0x04fff700> {
	DebugBackdoor m_backdoor;
public:
	void on_write(uint16) override {
		fmt::print("Backdoor: \u001b[96m{}", m_backdoor.to_string());
		fmt::print("\u001b[0m\n");
	}
};