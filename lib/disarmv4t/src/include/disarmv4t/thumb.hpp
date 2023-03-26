#pragma once
#include "condition.hpp"
#include "instructions/thumb.hpp"
#include "internal/common.hpp"

namespace disarmv4t::thumb {
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
        UD9,
        BKPT,
        BLX9,
        _end
    };

    constexpr InstructionType decode(uint16 thumb_opcode)
    {
        auto masked_pattern_match = [thumb_opcode](uint16 mask, uint16 pattern) {
            return (thumb_opcode & mask) == pattern;
        };

        if (masked_pattern_match(0xf800, 0x1800)) {
            return InstructionType::FMT2;
        }
        if (masked_pattern_match(0xe000, 0x0)) {
            return InstructionType::FMT1;
        }
        if (masked_pattern_match(0xe000, 0x2000)) {
            return InstructionType::FMT3;
        }
        if (masked_pattern_match(0xfc00, 0x4000)) {
            return InstructionType::FMT4;
        }
        if (masked_pattern_match(0xfc00, 0x4400)) {
            return InstructionType::FMT5;
        }
        if (masked_pattern_match(0xf800, 0x4800)) {
            return InstructionType::FMT6;
        }
        if (masked_pattern_match(0xf200, 0x5000)) {
            return InstructionType::FMT7;
        }
        if (masked_pattern_match(0xf200, 0x5200)) {
            return InstructionType::FMT8;
        }
        if (masked_pattern_match(0xe000, 0x6000)) {
            return InstructionType::FMT9;
        }
        if (masked_pattern_match(0xf000, 0x8000)) {
            return InstructionType::FMT10;
        }
        if (masked_pattern_match(0xf000, 0x9000)) {
            return InstructionType::FMT11;
        }
        if (masked_pattern_match(0xf000, 0xa000)) {
            return InstructionType::FMT12;
        }
        if (masked_pattern_match(0xff00, 0xb000)) {
            return InstructionType::FMT13;
        }
        if (masked_pattern_match(0xff00, 0xbe00)) {
            return InstructionType::BKPT;
        }
        if (masked_pattern_match(0xf600, 0xb400)) {
            return InstructionType::FMT14;
        }
        if (masked_pattern_match(0xf000, 0xc000)) {
            return InstructionType::FMT15;
        }
        if (masked_pattern_match(0xff00, 0xdf00)) {
            return InstructionType::FMT17;
        }
        if (masked_pattern_match(0xff00, 0xde00)) {
            return InstructionType::UD9;
        }
        if (masked_pattern_match(0xf000, 0xd000)) {
            return InstructionType::FMT16;
        }
        if (masked_pattern_match(0xf800, 0xe000)) {
            return InstructionType::FMT18;
        }
        if (masked_pattern_match(0xf801, 0xe800)) {
            return InstructionType::BLX9;
        }
        if (masked_pattern_match(0xf801, 0xe801)) {
            return InstructionType::UD9;
        }
        if (masked_pattern_match(0xf000, 0xf000)) {
            return InstructionType::FMT19;
        }

        return InstructionType::UD;
    }
} // namespace disarmv4t::thumb
