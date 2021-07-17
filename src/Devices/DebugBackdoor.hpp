#pragma once
#include "MMU/IOReg.hpp"

class DebugBackdoor final : public ArrayMem<0x04fff600, 0x04fff600 + 0x60, 0x60> {
public:
	IOAccess access32() const override { return IOAccess::RW; }
	IOAccess access16() const override { return IOAccess::RW; }
	IOAccess access8() const override { return IOAccess::RW; }

	std::string identify() const override {
		return "Backdoor";
	}

	std::string to_string() const {
		std::string str;
		for(unsigned i = 0; i < 0x60; ++i) {
			if(m_buffer[i] == '\0')
				break;
			str.push_back(m_buffer[i]);
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