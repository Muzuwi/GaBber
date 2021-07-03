#pragma once
#include <bit>
#include "Headers/StdTypes.hpp"

namespace ARM {
	enum class InstructionType {
		BX  = 0,
		BBL,
		ALU,
		MUL,
		MLL,
		SDT,
		HDT,
		BDT,
		SWP,
		SWI,
		UD,
		_End
	};

	#define PATTERN(mask, pattern) ((arm_opcode & mask) == pattern)
	static inline InstructionType opcode_decode(uint32 arm_opcode) {
		if (PATTERN(0x0f000000, 0x0f000000)) return InstructionType::SWI;
		if (PATTERN(0x0e000000, 0x0a000000)) return InstructionType::BBL;
		if (PATTERN(0x0e000000, 0x08000000)) return InstructionType::BDT;
		if (PATTERN(0x0c000000, 0x04000000)) return InstructionType::SDT;
		if (PATTERN(0x0ffffff0, 0x12fff10)) return InstructionType::BX;
		if (PATTERN(0xfb00ff0, 0x01000090)) return InstructionType::SWP;
		if (PATTERN(0x0fc000f0, 0x90)) return InstructionType::MUL;
		if (PATTERN(0x0e400f90, 0x90)) return InstructionType::HDT;
		if (PATTERN(0x0e400f90, 0x400090)) return InstructionType::HDT;
		if (PATTERN(0x0f8000f0, 0x00800090)) return InstructionType::MLL;
		if (PATTERN(0x0c000000, 0)) return InstructionType::ALU;

		return InstructionType::UD;
	}
	#undef PATTERN

	enum class InstructionCondition : uint8 {
		EQ = 0b0000,
		NE = 0b0001,
		CS = 0b0010,
		CC = 0b0011,
		MI = 0b0100,
		PL = 0b0101,
		VS = 0b0110,
		VC = 0b0111,
		HI = 0b1000,
		LS = 0b1001,
		GE = 0b1010,
		LT = 0b1011,
		GT = 0b1100,
		LE = 0b1101,
		AL = 0b1110,
		_Reserved = 0b1111
	};

	enum class ShiftType : uint8 {
		LogicalLeft  = 0b00,
		LogicalRight = 0b01,
		ArithmeticRight = 0b10,
		RotateRight  = 0b11
	};

	class Instruction {
	public:
		uint32 m_data;
	public:
		Instruction(uint32 data)
		: m_data(data) {}

		InstructionCondition condition() const {
			const auto c = static_cast<InstructionCondition>((m_data & 0xf0000000u) >> 28u);
			return c;
		}
	};

	class BXInstruction : public Instruction {
	public:
		union Reg {
			struct {
				uint8 reg : 4;
				uint32 _dummy : 24;
				InstructionCondition condition : 4;
			} __attribute__((packed));
			uint32 _opcode;
		};

		BXInstruction(uint32 data)
		: Instruction(data) {}

		BXInstruction(Reg reg)
		: Instruction(reg._opcode) {}

		uint8 reg() const {
			return m_data & 0b1111;
		}
	};

	class MultInstruction : public Instruction {
	public:
		union Reg {
			struct {
				uint8 multiplicand_reg : 4;
				uint8 _dummy1 : 4;
				uint8 source_reg : 4;
				uint8 accumulate_reg : 4;
				uint8 target_reg : 4;
				uint32 _dummy2 : 6;
				bool set_condition_flag : 1;
				bool accumulate_flag : 1;
				InstructionCondition condition : 4;
			} __attribute__((packed));
			uint32 _opcode;
		};

		MultInstruction(uint32 data)
		: Instruction(data) {}

		MultInstruction(Reg reg)
		: Instruction(reg._opcode) {}

		uint8 multiplicand_reg() const {
			return m_data & 0b1111;
		}

		uint8 source_reg() const {
			return (m_data >> 8u) & 0b1111;
		}

		uint8 accumulate_reg() const {
			return (m_data >> 12u) & 0b1111;
		}

		uint8 destination_reg() const {
			return (m_data >> 16u) & 0b1111;
		}

		bool should_set_condition() const {
			return (m_data) & (1u << 20u);
		}

		bool should_accumulate() const {
			return (m_data) & (1u << 21u);
		}
	};

	class BInstruction : public Instruction {
	public:
		union Reg {
			struct {
				uint32 offset : 24;
				bool link_flag : 1;
				uint32 _dummy : 3;
				InstructionCondition condition : 4;
			} __attribute__((packed));
			uint32 _opcode;
		};

		BInstruction(uint32 data)
		: Instruction(data) {}

		BInstruction(Reg reg)
		: Instruction(reg._opcode) {}

		bool is_link() const {
			return m_data & (1u << 24u);
		}

		int32 offset() const {
			auto u = ((m_data & (0x00FFFFFF)) << 2);
			return ((signed int)(u << 6)) >> 6;
		}
	};

	class SWPInstruction : public Instruction {
	public:
		union Reg {
			struct {
				uint8 source_reg : 4;
				uint8 _bitseq4 : 4;
				uint8 _bitseq3 : 4;
				uint8 destination_reg : 4;
				uint8 base_reg : 4;
				uint8 _bitseq2 : 2;
				bool quantity : 1;
				uint8 _bitseq1 : 5;
				InstructionCondition condition : 4;
			} __attribute__((packed));
			uint32 _opcode;
		};

		SWPInstruction(uint32 data)
		: Instruction(data) {}

		SWPInstruction(Reg data)
		: Instruction(data._opcode) {}

		bool swap_byte() const {
			return m_data & (1u << 22u);
		}

		uint8 source_reg() const {
			return m_data & 0b1111;
		}

		uint8 destination_reg() const {
			return (m_data >> 12u) & 0b1111;
		}

		uint8 base_reg() const {
			return (m_data >> 16u) & 0b1111;
		}
	};

	class DataProcessInstruction : public Instruction {
	public:
		union Reg {
			struct {
				uint16 operand2 : 12;
				uint8 destination_reg : 4;
				uint8 operand1_reg : 4;
				bool set_condition : 1;
				uint8 opcode : 4;
				bool immediate_operand : 1;
				uint8 _bitseq1 : 2;
				InstructionCondition condition : 4;
			} __attribute__((packed));
			uint32 _opcode;
		};

		DataProcessInstruction(uint32 data)
		: Instruction(data) {}

		DataProcessInstruction(Reg data)
		: Instruction(data._opcode) {}

		bool immediate_is_value() const {
			return m_data & (1u << 25u);
		}

		bool should_set_condition() const {
			return (m_data & (1u << 20u));
		}

		uint8 opcode() const {
			return (m_data >> 21u) & 0b1111u;
		}

		uint8 operand1_reg() const {
			return (m_data >> 16u) & 0b1111u;
		}

		uint8 destination_reg() const {
			return (m_data >> 12u) & 0b1111u;
		}

		/*
		 *  When I = 1
		 */
		bool is_shift_reg() const {
			return (m_data & (1u << 4u));
		}

		ShiftType shift_type() const {
			return static_cast<ShiftType>(((m_data >> 5u) & 0b11));
		}

		uint8 shift_amount_or_reg() const {
			if(is_shift_reg())
				return (m_data >> 8u) & 0b1111;     //  Register number encoded in instruction
			else
				return (m_data >> 7u) & 0b11111;    //  Immediate encoded in instruction
		}

		uint8 operand2_reg() const {
			return (m_data & 0b1111);
		}

		/*
		 *  When I = 0
		 */
		uint8 rotate() const {
			return (m_data >> 8u) & 0b1111;
		}

		uint8 immediate() const {
			return (m_data & 0xFF);
		}
	};

	class SDTInstruction : public Instruction {
	public:
		SDTInstruction(uint32 data)
		: Instruction(data) {}

		bool immediate_is_offset() const {
			return !(m_data & (1u << 25u));
		}

		bool preindex() const {
			return (m_data & (1u << 24u));
		}

		bool add_offset_to_base() const {
			return (m_data & (1u << 23u));
		}

		bool quantity_in_bytes() const {
			return (m_data & (1u << 22u));
		}

		bool writeback() const {
			return (m_data & (1u << 21u));
		}

		bool load_from_memory() const {
			return (m_data & (1u << 20u));
		}

		uint8 base_reg() const {
			return (m_data >> 16u) & 0b1111;
		}

		uint8 target_reg() const {
			return (m_data >> 12u) & 0b1111;
		}

		uint16 offset() const {
			return m_data & 0xFFF;
		}

	};

	class BDTInstruction : public Instruction {
	public:
		BDTInstruction(uint32 data)
		: Instruction(data) {}

		bool preindex() const {
			return (m_data & (1u << 24u));
		}

		bool add_offset_to_base() const {
			return (m_data & (1u << 23u));
		}

		bool PSR() const {
			return (m_data & (1u << 22u));
		}

		bool writeback() const {
			return (m_data & (1u << 21u));
		}

		bool load_from_memory() const {
			return (m_data & (1u << 20u));
		}

		uint8 base_reg() const {
			return (m_data >> 16u) & 0b1111u;
		}

		bool is_register_in_list(uint8 reg) const {
			if(reg > 15) return false;
			else return m_data & (1u << reg);
		}

		bool is_rlist_empty() const {
			return (m_data & 0xFFFF) == 0;
		}

		bool is_register_first_in_rlist(uint8 reg) const {
			return std::countr_zero(m_data & 0xFFFFu) == reg;
		}

		int32 total_offset() const {
			int32 offset = 0;
			for(unsigned i = 0; i < 16; ++i) {
				if(!is_register_in_list(i)) continue;
				offset += add_offset_to_base() ? 4 : -4;
			}
			return offset;
		}
	};

	class SWIInstruction : public Instruction {
	public:
		SWIInstruction(uint32 data)
		: Instruction(data) {}

		uint32 comment() const {
			return m_data & 0x00ffffffu;
		}
	};

	class MultLongInstruction : public Instruction {
	public:
		MultLongInstruction(uint32 data)
		: Instruction(data) {}

		bool is_signed() const {
			return m_data & (1u << 22u);
		}

		bool should_accumulate() const {
			return m_data & (1u << 21u);
		}

		bool should_set_condition() const {
			return m_data & (1u << 20u);
		}

		uint8 destHi_reg() const {
			return (m_data >> 16u) & 0b1111u;
		}

		uint8 destLo_reg() const {
			return (m_data >> 12u) & 0b1111u;
		}

		uint8 operand1_reg() const {
			return (m_data >> 8u) & 0b1111u;
		}

		uint8 operand2_reg() const {
			return m_data & 0b1111u;
		}

	};

	class HDTInstruction : public Instruction {
	public:
		HDTInstruction(uint32 data)
		: Instruction(data) {}

		bool preindex() const {
			return m_data & (1u << 24u);
		}

		bool add_offset_to_base() const {
			return m_data & (1u << 23u);
		}

		bool is_offset_immediate() const {
			return m_data & (1u << 22u);
		}

		bool writeback() const {
			return m_data & (1u << 21u);
		}

		bool load_from_memory() const {
			return m_data & (1u << 20u);
		}

		uint8 base_reg() const {
			return (m_data >> 16u) & 0xF;
		}

		uint8 target_reg() const {
			return (m_data >> 12u) & 0xF;
		}


		uint8 offset_reg_or_immediate_low() const {
			return m_data & 0xF;
		}

		uint8 immediate_high() const {
			return (m_data >> 8u) & 0xF;
		}

		uint8 immediate() const {
			return (offset_reg_or_immediate_low() & 0xF) | (immediate_high() << 0xF);
		}

		uint8 opcode() const {
			return (m_data >> 5u) & 0b11;
		}
	};
}