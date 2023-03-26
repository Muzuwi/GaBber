#pragma once
#include "../condition.hpp"
#include "../internal/common.hpp"
#include "../shift.hpp"
#include <bit>

namespace disarmv4t::arm::instr {
    class Instruction {
    public:
        constexpr Instruction(uint32 data)
            : m_data(data)
        {
        }

        constexpr InstructionCondition condition() const
        {
            return static_cast<InstructionCondition>((m_data & 0xf0000000u) >> 28u);
        }

    protected:
        uint32 m_data;
    };

    class BXInstruction final : public Instruction {
    public:
        constexpr BXInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr uint8 reg() const
        {
            return m_data & 0b1111;
        }
    };

    class MultInstruction final : public Instruction {
    public:
        constexpr MultInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr uint8 multiplicand_reg() const
        {
            return m_data & 0b1111;
        }

        constexpr uint8 source_reg() const
        {
            return (m_data >> 8u) & 0b1111;
        }

        constexpr uint8 accumulate_reg() const
        {
            return (m_data >> 12u) & 0b1111;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 16u) & 0b1111;
        }

        constexpr bool should_set_condition() const
        {
            return (m_data) & (1u << 20u);
        }

        constexpr bool should_accumulate() const
        {
            return (m_data) & (1u << 21u);
        }
    };

    class BInstruction final : public Instruction {
    public:
        constexpr BInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool is_link() const
        {
            return m_data & (1u << 24u);
        }

        constexpr int32 offset() const
        {
            auto u = ((m_data & (0x00FFFFFF)) << 2);
            return ((signed int)(u << 6)) >> 6;
        }
    };

    class SWPInstruction final : public Instruction {
    public:
        constexpr SWPInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool swap_byte() const
        {
            return m_data & (1u << 22u);
        }

        constexpr uint8 source_reg() const
        {
            return m_data & 0b1111;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 12u) & 0b1111;
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 16u) & 0b1111;
        }
    };

    class DataProcessInstruction final : public Instruction {
    public:
        constexpr DataProcessInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool immediate_is_value() const
        {
            return m_data & (1u << 25u);
        }

        constexpr bool should_set_condition() const
        {
            return (m_data & (1u << 20u));
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 21u) & 0b1111u;
        }

        constexpr uint8 operand1_reg() const
        {
            return (m_data >> 16u) & 0b1111u;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 12u) & 0b1111u;
        }

        /*
         *  When I = 1
         */
        constexpr bool is_shift_reg() const
        {
            return (m_data & (1u << 4u));
        }

        constexpr ShiftType shift_type() const
        {
            return static_cast<ShiftType>(((m_data >> 5u) & 0b11));
        }

        constexpr uint8 shift_amount_or_reg() const
        {
            if (is_shift_reg()) {
                return (m_data >> 8u) & 0b1111; //  Register number encoded in instruction
            } else {
                return (m_data >> 7u) & 0b11111; //  Immediate encoded in instruction
            }
        }

        constexpr uint8 operand2_reg() const
        {
            return (m_data & 0b1111);
        }

        /*
         *  When I = 0
         */
        constexpr uint8 rotate() const
        {
            return (m_data >> 8u) & 0b1111;
        }

        constexpr uint8 immediate() const
        {
            return (m_data & 0xFF);
        }
    };

    class SDTInstruction final : public Instruction {
    public:
        constexpr SDTInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool immediate_is_offset() const
        {
            return !(m_data & (1u << 25u));
        }

        constexpr bool preindex() const
        {
            return (m_data & (1u << 24u));
        }

        constexpr bool add_offset_to_base() const
        {
            return (m_data & (1u << 23u));
        }

        constexpr bool quantity_in_bytes() const
        {
            return (m_data & (1u << 22u));
        }

        constexpr bool writeback() const
        {
            return (m_data & (1u << 21u));
        }

        constexpr bool load_from_memory() const
        {
            return (m_data & (1u << 20u));
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 16u) & 0b1111;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data >> 12u) & 0b1111;
        }

        constexpr uint16 offset() const
        {
            return m_data & 0xFFF;
        }
    };

    class BDTInstruction final : public Instruction {
    public:
        constexpr BDTInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool preindex() const
        {
            return (m_data & (1u << 24u));
        }

        constexpr bool add_offset_to_base() const
        {
            return (m_data & (1u << 23u));
        }

        constexpr bool PSR() const
        {
            return (m_data & (1u << 22u));
        }

        constexpr bool writeback() const
        {
            return (m_data & (1u << 21u));
        }

        constexpr bool load_from_memory() const
        {
            return (m_data & (1u << 20u));
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 16u) & 0b1111u;
        }

        constexpr bool is_register_in_list(uint8 reg) const
        {
            if (reg > 15) {
                return false;
            } else {
                return m_data & (1u << reg);
            }
        }

        constexpr bool is_rlist_empty() const
        {
            return (m_data & 0xFFFF) == 0;
        }

        constexpr bool is_register_first_in_rlist(uint8 reg) const
        {
            return std::countr_zero(m_data & 0xFFFFu) == reg;
        }

        constexpr int32 total_offset() const
        {
            int32 offset = 0;
            for (unsigned i = 0; i < 16; ++i) {
                if (!is_register_in_list(i)) {
                    continue;
                }
                offset += add_offset_to_base() ? 4 : -4;
            }
            return offset;
        }
    };

    class SWIInstruction final : public Instruction {
    public:
        constexpr SWIInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr uint32 comment() const
        {
            return m_data & 0x00ffffffu;
        }
    };

    class MultLongInstruction final : public Instruction {
    public:
        constexpr MultLongInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool is_signed() const
        {
            return m_data & (1u << 22u);
        }

        constexpr bool should_accumulate() const
        {
            return m_data & (1u << 21u);
        }

        constexpr bool should_set_condition() const
        {
            return m_data & (1u << 20u);
        }

        constexpr uint8 destHi_reg() const
        {
            return (m_data >> 16u) & 0b1111u;
        }

        constexpr uint8 destLo_reg() const
        {
            return (m_data >> 12u) & 0b1111u;
        }

        constexpr uint8 operand1_reg() const
        {
            return (m_data >> 8u) & 0b1111u;
        }

        constexpr uint8 operand2_reg() const
        {
            return m_data & 0b1111u;
        }
    };

    class HDTInstruction final : public Instruction {
    public:
        constexpr HDTInstruction(uint32 data)
            : Instruction(data)
        {
        }

        constexpr bool preindex() const
        {
            return m_data & (1u << 24u);
        }

        constexpr bool add_offset_to_base() const
        {
            return m_data & (1u << 23u);
        }

        constexpr bool is_offset_immediate() const
        {
            return m_data & (1u << 22u);
        }

        constexpr bool writeback() const
        {
            return m_data & (1u << 21u);
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 20u);
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 16u) & 0xF;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data >> 12u) & 0xF;
        }

        constexpr uint8 offset_reg_or_immediate_low() const
        {
            return m_data & 0xF;
        }

        constexpr uint8 immediate_high() const
        {
            return (m_data >> 8u) & 0xF;
        }

        constexpr uint8 immediate() const
        {
            return (offset_reg_or_immediate_low() & 0xF) | (immediate_high() << 4u);
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 5u) & 0b11;
        }
    };
} // namespace disarmv4t::arm::instr
