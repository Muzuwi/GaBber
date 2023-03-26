#pragma once
#include "../condition.hpp"
#include "../internal/common.hpp"
#include "../shift.hpp"
#include <bit>

namespace disarmv4t::thumb::instr {
    class Instruction {
    public:
        constexpr Instruction(uint32 data)
            : m_data(data)
        {
        }

    protected:
        uint32 m_data;
    };

    class InstructionFormat1 final : public Instruction {
    public:
        constexpr InstructionFormat1(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint16 raw() const
        {
            return m_data;
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 11u) & 0b11;
        }

        constexpr uint8 immediate() const
        {
            return (m_data >> 6u) & 0b11111;
        }

        constexpr uint8 source_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat2 final : public Instruction {
    public:
        constexpr InstructionFormat2(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool immediate_is_value() const
        {
            return m_data & (1u << 10u);
        }

        constexpr bool subtract() const
        {
            return m_data & (1u << 9u);
        }

        constexpr uint8 immediate() const
        {
            return (m_data >> 6u) & 0b111;
        }

        constexpr uint8 source_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat3 final : public Instruction {
    public:
        constexpr InstructionFormat3(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 11u) & 0b11;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data >> 8u) & 0b111;
        }

        constexpr uint8 immediate() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat4 final : public Instruction {
    public:
        constexpr InstructionFormat4(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 6u) & 0xF;
        }

        constexpr uint8 source_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 target_reg() const
        {
            return m_data & 0b111;
        }
    };

    class InstructionFormat5 final : public Instruction {
    public:
        constexpr InstructionFormat5(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 8u) & 0b11;
        }

        constexpr bool MSBd() const
        {
            return m_data & (1u << 7u);
        }

        constexpr bool MSBs() const
        {
            return m_data & (1u << 6u);
        }

        constexpr uint8 source_reg() const
        {
            return ((m_data >> 3u) & 0b111) | (MSBs() ? 0x8 : 0);
        }

        constexpr uint8 destination_reg() const
        {
            return ((m_data)&0b111) | (MSBd() ? 0x8 : 0);
        }
    };

    class InstructionFormat6 final : public Instruction {
    public:
        constexpr InstructionFormat6(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 8u) & 0b111u;
        }

        constexpr uint8 immediate() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat7 final : public Instruction {
    public:
        constexpr InstructionFormat7(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr bool quantity_in_bytes() const
        {
            return m_data & (1u << 10u);
        }

        constexpr uint8 offset_reg() const
        {
            return (m_data >> 6u) & 0b111;
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat8 final : public Instruction {
    public:
        constexpr InstructionFormat8(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool h_flag() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint8 opcode() const
        {
            return (m_data >> 10u) & 0b11u;
        }

        constexpr bool operand_sign_extended() const
        {
            return m_data & (1u << 10u);
        }

        constexpr uint8 offset_reg() const
        {
            return (m_data >> 6u) & 0b111;
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat9 final : public Instruction {
    public:
        constexpr InstructionFormat9(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr bool quantity_in_bytes() const
        {
            return m_data & (1u << 12u);
        }

        constexpr uint8 offset() const
        {
            return (m_data >> 6u) & 0b11111;
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat10 final : public Instruction {
    public:
        constexpr InstructionFormat10(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint8 offset() const
        {
            return (m_data >> 6u) & 0b11111;
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 3u) & 0b111;
        }

        constexpr uint8 target_reg() const
        {
            return (m_data)&0b111;
        }
    };

    class InstructionFormat11 final : public Instruction {
    public:
        constexpr InstructionFormat11(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 8u) & 0b111;
        }

        constexpr uint8 immediate() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat12 final : public Instruction {
    public:
        constexpr InstructionFormat12(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool source_is_sp() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint8 destination_reg() const
        {
            return (m_data >> 8u) & 0b111;
        }

        constexpr uint8 immediate() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat13 final : public Instruction {
    public:
        constexpr InstructionFormat13(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool offset_is_negative() const
        {
            return m_data & (1u << 7u);
        }

        constexpr uint8 offset() const
        {
            return m_data & 0b1111111;
        }
    };

    class InstructionFormat14 final : public Instruction {
    public:
        constexpr InstructionFormat14(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr bool store_lr_load_pc() const
        {
            return m_data & (1u << 8u);
        }

        constexpr bool is_register_in_list(uint8 reg)
        {
            if (reg > 7) {
                return false;
            } else {
                return m_data & (1u << reg);
            }
        }
    };

    class InstructionFormat15 final : public Instruction {
    public:
        constexpr InstructionFormat15(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool load_from_memory() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint8 base_reg() const
        {
            return (m_data >> 8u) & 0b111;
        }

        constexpr bool is_register_in_list(uint8 reg) const
        {
            if (reg > 7)
                return false;
            else
                return m_data & (1u << reg);
        }

        constexpr bool is_rlist_empty() const
        {
            return (m_data & 0xFF) == 0;
        }

        constexpr bool is_register_first_in_rlist(uint8 reg) const
        {
            return std::countr_zero(m_data & 0xFFu) == reg;
        }

        constexpr uint8 total_offset() const
        {
            uint8 total = 0;
            for (unsigned i = 0; i < 8; ++i) {
                if (!is_register_in_list(i))
                    continue;
                total += 4;
            }
            return total;
        }
    };

    class InstructionFormat16 final : public Instruction {
    public:
        constexpr InstructionFormat16(uint16 data)
            : Instruction(data)
        {
        }

        constexpr InstructionCondition condition() const
        {
            return static_cast<InstructionCondition>((m_data >> 8u) & 0b1111u);
        }

        constexpr int8 offset() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat17 final : public Instruction {
    public:
        constexpr InstructionFormat17(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint8 comment() const
        {
            return m_data & 0xFF;
        }
    };

    class InstructionFormat18 final : public Instruction {
    public:
        constexpr InstructionFormat18(uint16 data)
            : Instruction(data)
        {
        }

        constexpr uint16 offset() const
        {
            return m_data & 0x7ff;
        }
    };

    class InstructionFormat19 final : public Instruction {
    public:
        constexpr InstructionFormat19(uint16 data)
            : Instruction(data)
        {
        }

        constexpr bool low() const
        {
            return m_data & (1u << 11u);
        }

        constexpr uint16 offset() const
        {
            return m_data & 0x7ff;
        }
    };
} // namespace disarmv4t::thumb::instr
