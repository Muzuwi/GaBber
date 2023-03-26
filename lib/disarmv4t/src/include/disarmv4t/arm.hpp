#pragma once
#include "condition.hpp"
#include "instructions/arm.hpp"
#include "internal/common.hpp"

namespace disarmv4t::arm {
    enum class InstructionType {
        BX = 0,
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
        CODT,
        CO9,
        CODO,
        CORT,
        MLH,
        QALU,
        CLZ,
        BKPT,
        _end
    };

    constexpr InstructionType decode(uint32 arm_opcode)
    {
        auto masked_pattern_match = [arm_opcode](uint32 mask, uint32 pattern) {
            return (arm_opcode & mask) == pattern;
        };

        if (masked_pattern_match(0x0f000000, 0x0f000000)) {
            return InstructionType::SWI;
        }
        if (masked_pattern_match(0x0f000010, 0x0e000010)) {
            return InstructionType::CORT;
        }
        if (masked_pattern_match(0x0f000010, 0x0e000000)) {
            return InstructionType::CODO;
        }
        if (masked_pattern_match(0x0fe00000, 0x0c400000)) {
            return InstructionType::CO9;
        }
        if (masked_pattern_match(0x0e000000, 0x0c000000)) {
            return InstructionType::CODT;
        }
        if (masked_pattern_match(0x0e000000, 0x0a000000)) {
            return InstructionType::BBL;
        }
        if (masked_pattern_match(0x0e000000, 0x08000000)) {
            return InstructionType::BDT;
        }
        if (masked_pattern_match(0x0c000000, 0x04000000)) {
            return InstructionType::SDT;
        }
        if (masked_pattern_match(0x0ffffff0, 0x12fff10)) {
            return InstructionType::BX;
        }
        if (masked_pattern_match(0xfff000f0, 0xe1200070)) {
            return InstructionType::BKPT;
        }
        if (masked_pattern_match(0x0fff0ff0, 0x016f0f10)) {
            return InstructionType::CLZ;
        }
        if (masked_pattern_match(0x0f900ff0, 0x01000050)) {
            return InstructionType::QALU;
        }
        if (masked_pattern_match(0x0fb00ff0, 0x01000090)) {
            return InstructionType::SWP;
        }
        if (masked_pattern_match(0x0f900090, 0x01000080)) {
            return InstructionType::MLH;
        }
        if (masked_pattern_match(0x0fc000f0, 0x90)) {
            return InstructionType::MUL;
        }
        if (masked_pattern_match(0x0f8000f0, 0x00800090)) {
            return InstructionType::MLL;
        }
        if (masked_pattern_match(0x0e400f90, 0x90)) {
            return InstructionType::HDT;
        }
        if (masked_pattern_match(0x0e400090, 0x400090)) {
            return InstructionType::HDT;
        }
        if (masked_pattern_match(0x0c000000, 0)) {
            return InstructionType::ALU;
        }

        return InstructionType::UD;
    }
} // namespace disarmv4t::arm
