#include <algorithm>
#include "Bus/SRAM.hpp"

uint8 SRAM::read8(uint32 offset) {
	if(m_type != BackupCartType::FLASH64K && m_type != BackupCartType::FLASH128K) {
//		fmt::print("PakSRAM/ FIXME: Read unimplemented type: {}\n", m_type);
		return 0x00;
	}

	if(m_mode == FlashChipMode::ID) {
		if(offset == 0x1) {
			fmt::print("PakSRAM/ Read offset 1\n");
			return m_identifier >> 16u;
		}
		if(offset == 0x0) {
			fmt::print("PakSRAM/ Read offset 0\n");
			return m_identifier & 0xFFu;
		}
	}

	const unsigned size = m_type == BackupCartType::FLASH64K ? 65536 : 65536*2;
	const auto buffer_offset = (m_bank * 0x10000 + (offset & 0xFFFFu)) % size;

	if(offset >= m_buffer.size()) {
		return 0xFF;
	}

	return m_buffer[buffer_offset];
}

void SRAM::write8(uint32 offset, uint8 value) {
	if(m_type != BackupCartType::FLASH64K && m_type != BackupCartType::FLASH128K) {
//		fmt::print("PakSRAM/ FIXME: Write unimplemented type: {}\n", m_type);
		return;
	}

//	fmt::print("PakSRAM/ Flash write[{:04x}]={:02x}\n", offset, value);

	if(m_mode == FlashChipMode::Write) {
		const unsigned size = m_type == BackupCartType::FLASH64K ? 65536 : 65536*2;
		m_buffer[(offset + m_bank * 0x10000) % size] = value;
		m_mode = FlashChipMode::Idle;
		return;
	}

	if(offset == 0x5555) {
		if(m_reg5555 == 0xaa && m_reg2aaa == 0x55) {
			if(value == 0x90) {
				fmt::print("PakSRAM/ Enter ID mode\n");
				m_mode = FlashChipMode::ID;
			} else if(value == 0xF0) {
				fmt::print("PakSRAM/ Terminate mode\n");
				m_mode = FlashChipMode::Idle;
			} else if(value == 0x80) {
				fmt::print("PakSRAM/ Enter erase mode\n");
				m_mode = FlashChipMode::Erase;
			} else if(value == 0x10 && m_mode == FlashChipMode::Erase) {
				fmt::print("PakSRAM/ Erase entire chip\n");
				std::fill_n(&m_buffer[0], m_buffer.size(), 0xFF);
			} else if(value == 0xA0) {
				fmt::print("PakSRAM/ Enter write mode\n");
				m_mode = FlashChipMode::Write;
			} else if(value == 0xB0) {
				fmt::print("PakSRAM/ Enter bank change mode\n");
				m_mode = FlashChipMode::BankChange;
			}
			else {
				fmt::print("PakSRAM/ Flash command id={:02x}\n", value);
			}
			m_reg2aaa = 0x0;
			m_reg5555 = 0x0;
			return;
		}

		m_reg5555 = value;
		return;
	}
	if(offset == 0x2aaa) {
		m_reg2aaa = value;
		return;
	}
	if(offset == 0x0 && m_mode == FlashChipMode::BankChange) {
		fmt::print("PakSRAM/ Change bank to {}\n", m_bank);
		m_bank = value & 1;
		m_mode = FlashChipMode::Idle;
		return;
	}
	if(m_mode == FlashChipMode::Erase && value == 0x30) {
		fmt::print("PakSRAM/ Erase sector {}\n", offset / 0x1000);
		const unsigned size = m_type == BackupCartType::FLASH64K ? 65536 : 65536*2;
		const auto buffer_offset = (m_bank * 0x10000 + (offset & 0xFFFFu)) % size;
		std::fill_n(&m_buffer[buffer_offset], 0x1000, 0xFF);
		return;
	}
}
