#pragma once
#include <vector>
#include "Bus/SRAM/BackupCart.hpp"

enum class FlashChipMode {
	Idle,
	ID,
	Erase,
	Write,
	BankChange
};

class Flash final : public BackupCart {
	const uint32 m_identifier { 0x1cc2u };
	const unsigned m_size;

	FlashChipMode m_mode {};
	uint8 m_bank;
	uint8 m_reg5555;
	uint8 m_reg2aaa;
	std::vector<uint8> m_buffer;
public:
	explicit Flash(unsigned size)
	    : m_size(size) {}

	uint8 read(uint32 offset) override;
	void write(uint32 offset, uint8 value) override;

	void from_vec(std::vector<uint8>&& vector) override {
		m_buffer = vector;
		m_buffer.resize(m_size, 0xFF);
	}

	std::vector<uint8> const& to_vec() override {
		return m_buffer;
	}
};
