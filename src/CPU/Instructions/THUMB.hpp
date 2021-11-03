#pragma once
#include "Headers/StdTypes.hpp"
#include "ARM.hpp"

namespace THUMB {
	enum class InstructionType {
		FMT1 = 0,
		FMT2,
		FMT3,
		FMT4,
		FMT5,
		FMT6,
		FMT7,
		FMT8,
		FMT9,
		FMT10,
		FMT11,
		FMT12,
		FMT13,
		FMT14,
		FMT15,
		FMT16,
		FMT17,
		FMT18,
		FMT19,
		UD,
		_End
	};

	#define PATTERN(mask, pattern) ((thumb_opcode & mask) == pattern)
	static inline InstructionType opcode_decode(uint16 thumb_opcode) {
		if (PATTERN(0xf800, 0x1800)) return InstructionType::FMT2;
		if (PATTERN(0xe000, 0x0)) return InstructionType::FMT1;
		if (PATTERN(0xe000, 0x2000)) return InstructionType::FMT3;
		if (PATTERN(0xfc00, 0x4000)) return InstructionType::FMT4;
		if (PATTERN(0xfc00, 0x4400)) return InstructionType::FMT5;
		if (PATTERN(0xf800, 0x4800)) return InstructionType::FMT6;
		if (PATTERN(0xf200, 0x5000)) return InstructionType::FMT7;
		if (PATTERN(0xf200, 0x5200)) return InstructionType::FMT8;
		if (PATTERN(0xe000, 0x6000)) return InstructionType::FMT9;
		if (PATTERN(0xf000, 0x8000)) return InstructionType::FMT10;
		if (PATTERN(0xf000, 0x9000)) return InstructionType::FMT11;
		if (PATTERN(0xf000, 0xa000)) return InstructionType::FMT12;
		if (PATTERN(0xff00, 0xb000)) return InstructionType::FMT13;
		if (PATTERN(0xf600, 0xb400)) return InstructionType::FMT14;
		if (PATTERN(0xf000, 0xc000)) return InstructionType::FMT15;
		if (PATTERN(0xff00, 0xdf00)) return InstructionType::FMT17;
		if (PATTERN(0xf000, 0xd000)) return InstructionType::FMT16;
		if (PATTERN(0xf800, 0xe000)) return InstructionType::FMT18;
		if (PATTERN(0xf000, 0xf000)) return InstructionType::FMT19;

		return InstructionType::UD;
	}
	#undef PATTERN

	class InstructionFormat1 {
		uint16 m_data;
	public:
		InstructionFormat1(uint16 data)
		: m_data(data) {}

		uint16 raw() const {
			return m_data;
		}

		uint8 opcode() const {
			return (m_data >> 11u) & 0b11;
		}

		uint8 immediate() const {
			return (m_data >> 6u) & 0b11111;
		}

		uint8 source_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 destination_reg() const {
			return (m_data) & 0b111;
		}
	};

	class InstructionFormat2 {
		uint16 m_data;
	public:
		InstructionFormat2(uint16 data)
				: m_data(data) {}

		bool immediate_is_value() const {
			return m_data & (1u << 10u);
		}

		bool subtract() const {
			return m_data & (1u << 9u);
		}

		uint8 immediate() const {
			return (m_data >> 6u) & 0b111;
		}

		uint8 source_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 destination_reg() const {
			return (m_data) & 0b111;
		}
	};

	class InstructionFormat3 {
		uint16 m_data;
	public:
		InstructionFormat3(uint16 data)
		: m_data(data) {}

		uint8 opcode() const {
			return (m_data >> 11u) & 0b11;
		}

		uint8 target_reg() const {
			return (m_data >> 8u) & 0b111;
		}

		uint8 immediate() const {
			return m_data & 0xFF;
		}
	};

	class InstructionFormat4 {
		uint16 m_data;
	public:
		InstructionFormat4(uint16 data)
		: m_data(data) {}

		uint8 opcode() const {
			return (m_data >> 6u) & 0xF;
		}

		uint8 source_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 target_reg() const {
			return m_data & 0b111;
		}
	};

	class InstructionFormat5 {
		uint16 m_data;
	public:
		InstructionFormat5(uint16 data)
		: m_data(data) {}

		uint8 opcode() const {
			return (m_data >> 8u) & 0b11;
		}

		bool MSBd() const {
			return m_data & (1u << 7u);
		}

		bool MSBs() const {
			return m_data & (1u << 6u);
		}

		uint8 source_reg() const {
			return ((m_data >> 3u) & 0b111) | (MSBs() ? 0x8 : 0);
		}

		uint8 destination_reg() const {
			return ((m_data) & 0b111) | (MSBd() ? 0x8 : 0);
		}
	};

	class InstructionFormat6 {
		uint16 m_data;
	public:
		InstructionFormat6(uint16 data)
		: m_data(data) {}

		uint8 destination_reg() const {
			return (m_data >> 8u) & 0b111u;
		}

		uint8 immediate() const {
			return m_data & 0xFF;
		}

	};

	class InstructionFormat7 {
		uint16 m_data;
	public:
		InstructionFormat7(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		bool quantity_in_bytes() const {
			return m_data & (1u << 10u);
		}

		uint8 offset_reg() const {
			return (m_data >> 6u) & 0b111;
		}

		uint8 base_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 target_reg() const {
			return (m_data) & 0b111;
		}

	};

	class InstructionFormat8 {
		uint16 m_data;
	public:
		InstructionFormat8(uint16 data)
		: m_data(data) {}

		bool h_flag() const {
			return m_data & (1u << 11u);
		}

		uint8 opcode() const {
			return (m_data >> 10u) & 0b11u;
		}

		bool operand_sign_extended() const {
			return m_data & (1u << 10u);
		}

		uint8 offset_reg() const {
			return (m_data >> 6u) & 0b111;
		}

		uint8 base_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 destination_reg() const {
			return (m_data) & 0b111;
		}

	};

	class InstructionFormat9 {
		uint16 m_data;
	public:
		InstructionFormat9(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		bool quantity_in_bytes() const {
			return m_data & (1u << 12u);
		}

		uint8 offset() const {
			return (m_data >> 6u) & 0b11111;
		}

		uint8 base_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 target_reg() const {
			return (m_data) & 0b111;
		}
	};

	class InstructionFormat10 {
		uint16 m_data;
	public:
		InstructionFormat10(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		uint8 offset() const {
			return (m_data >> 6u) & 0b11111;
		}

		uint8 base_reg() const {
			return (m_data >> 3u) & 0b111;
		}

		uint8 target_reg() const {
			return (m_data) & 0b111;
		}
	};

	class InstructionFormat11 {
		uint16 m_data;
	public:
		InstructionFormat11(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		uint8 destination_reg() const {
			return (m_data >> 8u) & 0b111;
		}

		uint8 immediate() const {
			return m_data & 0xFF;
		}
	};

	class InstructionFormat12 {
		uint16 m_data;
	public:
		InstructionFormat12(uint16 data)
		: m_data(data) {}

		bool source_is_sp() const {
			return m_data & (1u << 11u);
		}

		uint8 destination_reg() const {
			return (m_data >> 8u) & 0b111;
		}

		uint8 immediate() const {
			return m_data & 0xFF;
		}
	};

	class InstructionFormat13 {
		uint16 m_data;
	public:
		InstructionFormat13(uint16 data)
		: m_data(data) {}

		bool offset_is_negative() const {
			return m_data & (1u << 7u);
		}

		uint8 offset() const {
			return m_data & 0b1111111;
		}
	};

	class InstructionFormat14 {
		uint16 m_data;
	public:
		InstructionFormat14(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		bool store_lr_load_pc() const {
			return m_data & (1u << 8u);
		}

		bool is_register_in_list(uint8 reg) {
			if(reg > 7) return false;
			else return m_data & (1u << reg);
		}
	};

	class InstructionFormat15 {
		uint16 m_data;
	public:
		InstructionFormat15(uint16 data)
		: m_data(data) {}

		bool load_from_memory() const {
			return m_data & (1u << 11u);
		}

		uint8 base_reg() const {
			return (m_data >> 8u) & 0b111;
		}

		bool is_register_in_list(uint8 reg) const {
			if(reg > 7) return false;
			else return m_data & (1u << reg);
		}

		bool is_rlist_empty() const {
			return (m_data & 0xFF) == 0;
		}

		bool is_register_first_in_rlist(uint8 reg) const {
			return std::countr_zero(m_data & 0xFFu) == reg;
		}

		uint8 total_offset() const {
			uint8 total = 0;
			for(unsigned i = 0; i < 8; ++i) {
				if(!is_register_in_list(i)) continue;
				total += 4;
			}
			return total;
		}
	};

	class InstructionFormat16 {
		uint16 m_data;
	public:
		InstructionFormat16(uint16 data)
		: m_data(data) {}

		ARM::InstructionCondition condition() const {
			return static_cast<ARM::InstructionCondition>((m_data >> 8u) & 0b1111u);
		}

		int8 offset() const {
			return m_data & 0xFF;
		}
	};

	class InstructionFormat17 {
		uint16 m_data;
	public:
		InstructionFormat17(uint16 data)
		: m_data(data) {}

		uint8 comment() const {
			return m_data & 0xFF;
		}
	};

	class InstructionFormat18 {
		uint16 m_data;
	public:
		InstructionFormat18(uint16 data)
		: m_data(data) {}

		uint16 offset() const {
			return m_data & 0x7ff;
		}
	};

	class InstructionFormat19 {
		uint16 m_data;
	public:
		InstructionFormat19(uint16 data)
		: m_data(data) {}

		bool low() const {
			return m_data & (1u << 11u);
		}

		uint16 offset() const {
			return m_data & 0x7ff;
		}
	};
}
