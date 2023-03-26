#pragma once
#include "internal/common.hpp"

namespace disarmv4t {
    enum class ShiftType : uint8 {
        LogicalLeft = 0b00,
        LogicalRight = 0b01,
        ArithmeticRight = 0b10,
        RotateRight = 0b11
    };
} // namespace disarmv4t
