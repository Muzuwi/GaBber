#pragma once
#include <vector>
#include "Bus/Cart/BackupCart.hpp"

class SRAM : public BackupCart {
	std::vector<uint8> m_sram;
public:
	SRAM(GaBber& emu)
	    : BackupCart(emu) {}

	~SRAM() override = default;

	uint8 read(uint32 offset) override;
	void write(uint32 offset, uint8 value) override;

	void from_vec(std::vector<uint8>&& vector) override;
	std::vector<uint8> const& to_vec() override;
};