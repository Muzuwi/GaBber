#include "Bus/Cart/Flash.hpp"
#include <fmt/format.h>

uint8 Flash::read(uint32 offset) {
	switch(m_read_state) {
		case ReadState::Idle: {
			const auto buffer_offset = (m_bank * 0x10000 + (offset & 0xFFFFu)) % m_size;
			if(offset >= m_buffer.size()) {
				return 0xFF;
			}
			return m_buffer[buffer_offset];
		}
		case ReadState::ID: {
			switch(offset) {
				case 0x0: {
					return m_identifier & 0xFFu;
				}
				case 0x1: {
					return m_identifier >> 8u;
				}
				default: {
					fmt::print("Flash/ Read from invalid ID-mode address [{:08x}]\n", offset);
					return 0xFF;
				}
			}
		}
		default: {
			return 0xFF;
		}
	}
}

void Flash::write(uint32 offset, uint8 value) {
	switch(m_write_state) {
		case WriteState::Idle: {
			if(offset == 0x5555 && value == 0xAA) {
				m_write_state = WriteState::WroteAA;
			} else if(offset == 0x5555 && value == 0xF0) {
				fmt::print("Flash/ Terminate command after timeout\n");
				m_command_type = CommandType::Normal;
			} else {
				fmt::print("Flash/ Invalid write [{:08x}]={:02x} in Idle state\n", offset, value);
			}
			return;
		}
		case WriteState::WroteAA: {
			if(offset == 0x2AAA && value == 0x55) {
				m_write_state = WriteState::Wrote55;
			} else {
				fmt::print("Flash/ Invalid write [{:08x}]={:02x} in WroteAA state\n", offset, value);
				m_write_state = WriteState::Idle;
			}
			return;
		}
		case WriteState::Wrote55: {
			if(offset == 0x5555 && m_command_type == CommandType::Normal) {
				switch(value) {
					case 0x80: {
						fmt::print("Flash/ Enter erase command mode\n");
						m_command_type = CommandType::Erase;
						m_write_state = WriteState::Idle;
						return;
					}
					case 0x90: {
						fmt::print("Flash/ Enter ID mode\n");
						m_read_state = ReadState::ID;
						m_write_state = WriteState::Idle;
						return;
					}
					case 0xA0: {
						fmt::print("Flash/ Write single byte mode\n");
						m_write_state = WriteState::WriteSingleByte;
						return;
					}
					case 0xB0: {
						fmt::print("Flash/ Write bank number mode\n");
						m_write_state = WriteState::WriteBankNumber;
						return;
					}
					case 0xF0: {
						fmt::print("Flash/ Terminate ID mode\n");
						m_read_state = ReadState::Idle;
						m_write_state = WriteState::Idle;
						return;
					}
					default: {
						fmt::print("Flash/ Unknown command byte {:02x}\n", value);
						m_write_state = WriteState::Idle;
						return;
					}
				}
			} else if(m_command_type == CommandType::Erase) {
				m_write_state = WriteState::Idle;
				m_command_type = CommandType::Normal;

				if(offset == 0x5555 && value == 0x10) {
					fmt::print("Flash/ Erase entire chip\n");
					std::fill_n(&m_buffer[0], m_buffer.size(), 0xFF);
				} else if(value == 0x30) {
					fmt::print("Flash/ Erase sector\n");
					const auto buffer_offset = (m_bank * 0x10000 + (offset & 0xF000)) % m_size;
					std::fill_n(&m_buffer[buffer_offset], 0x1000, 0xFF);
				} else {
					fmt::print("Flash/ Invalid erase command [{:08x}]={:02x}\n", offset, value);
				}
				return;
			} else {
				fmt::print("Flash/ Invalid write [{:08x}]={:02x} in Wrote55 state\n", offset, value);
				return;
			}
		}
		case WriteState::WriteSingleByte: {
			m_write_state = WriteState::Idle;
			m_buffer[(offset + m_bank * 0x10000) % m_size] = value;
			return;
		}
		case WriteState::WriteBankNumber: {
			m_write_state = WriteState::Idle;
			if(offset == 0x0000) {
				fmt::print("Flash/ Bank change: {} -> {}\n", m_bank, value & 1);
				m_bank = value & 1;
				return;
			}
			fmt::print("Flash/ Invalid bank change write [{:08x}]={:02x}\n", offset, value);
			return;
		}
		default: {
			return;
		}
	}
}

void Flash::from_vec(std::vector<uint8>&& vector) {
	m_buffer = vector;
	m_buffer.resize(m_size, 0xFF);
}

std::vector<uint8> const& Flash::to_vec() {
	return m_buffer;
}
