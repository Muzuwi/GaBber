# disarmv4t

## About

This is a small library for decoding ARM/THUMB opcodes for the ARMv4T architecture.

## Usage

Call into the corresponding `disarmv4t::arm/thumb::decode(op)` functions to decode the instruction type of the given opcode.
All invalid opcodes are bunched into the corresponding `UD` instruction type.

To decode instruction fields, construct the matching `XXXInstruction` available in the `disarmv4t::arm/thumb::instr` namespace.
This might be improved in the future by using `std::variant` instead.
