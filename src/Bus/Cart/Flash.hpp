#pragma once
#include <vector>
#include "Bus/Cart/BackupCart.hpp"

class Flash final : public BackupCart {
	enum class ReadState {
		Idle,
		ID,
	};

	enum class WriteState {
		Idle,
		WroteAA,
		Wrote55,
		WriteSingleByte,
		WriteBankNumber,
	};

	enum class CommandType {
		Normal,
		Erase
	};

	const uint32 m_identifier;
	const unsigned m_size;

	ReadState m_read_state { ReadState::Idle };
	WriteState m_write_state { WriteState::Idle };
	CommandType m_command_type { CommandType::Normal };
	uint8 m_bank {};
	std::vector<uint8> m_buffer {};
public:
	explicit Flash(GaBber& emu, uint32 identifier, unsigned size)
	    : BackupCart(emu)
	    , m_identifier(identifier)
	    , m_size(size) {}

	uint8 read(uint32 offset) override;
	void write(uint32 offset, uint8 value) override;

	void from_vec(std::vector<uint8>&& vector) override;
	std::vector<uint8> const& to_vec() override;
};
