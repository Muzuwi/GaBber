#include "CPU/ARM7TDMI.hpp"
#include "Headers/Bits.hpp"

uint32 ARM7TDMI::evaluate_operand1(ARM::DataProcessInstruction instr) const {
	unsigned pc_offset = 0;
	if(!instr.immediate_is_value() && instr.is_shift_reg() && instr.operand1_reg() == 15)
		pc_offset = 4;
	return creg(instr.operand1_reg()) + pc_offset;
}

uint32 ARM7TDMI::evaluate_operand2(ARM::DataProcessInstruction instr, bool affect_carry) {
	affect_carry = instr.should_set_condition() && affect_carry;

	if(instr.immediate_is_value()) {
		const uint32 operand2 = instr.immediate();
		const uint8 shift_count = instr.rotate() * 2;

		if(shift_count == 0)
			return operand2;
		else
			return _shift_ror(operand2, shift_count, affect_carry);
	} else {
		const uint32 operand2 = creg(instr.operand2_reg()) + (instr.is_shift_reg() && instr.operand2_reg() == 15 ? current_instr_len() : 0);

		uint8 shift_count;
		if(instr.is_shift_reg()) {
			const uint8 reg_specified_shift_amount = creg(instr.shift_amount_or_reg()) & 0xFF;
			if(reg_specified_shift_amount == 0) {
				return operand2;
			}
			shift_count = reg_specified_shift_amount;
		} else {
			shift_count = instr.shift_amount_or_reg();
		}

		switch(instr.shift_type()) {
			case ARM::ShiftType::LogicalLeft: return _shift_lsl(operand2, shift_count, affect_carry);
			case ARM::ShiftType::LogicalRight: return _shift_lsr(operand2, shift_count, affect_carry);
			case ARM::ShiftType::ArithmeticRight: return _shift_asr(operand2, shift_count, affect_carry);
			case ARM::ShiftType::RotateRight: return _shift_ror(operand2, shift_count, affect_carry);
			default: ASSERT_NOT_REACHED();
		}
	}
}

void ARM7TDMI::_alu_set_flags_logical_op(uint32 result) {
	//  V not affected
	//  C should be set at this point by the shift operation
	cspr().set(CSPR_REGISTERS::Zero, result == 0);
	cspr().set(CSPR_REGISTERS::Negative, result & (1u << 31u));
}

uint32 ARM7TDMI::_alu_add(uint32 op1, uint32 op2, bool should_affect_flags) {
	if(should_affect_flags)
		cspr().set(CSPR_REGISTERS::Carry, (static_cast<uint64>(op1) + static_cast<uint64>(op2)) > 0xfffffffful);

	const uint32 v = op1 + op2;

	if(should_affect_flags) {
		const bool overflow =
		        (Bits::bit<31>(op1) == Bits::bit<31>(op2)) &&
		        (Bits::bit<31>(v) != Bits::bit<31>(op1));
		cspr().set(CSPR_REGISTERS::Overflow, overflow);
		_alu_set_flags_logical_op(v);
	}

	return v;
}

uint32 ARM7TDMI::_alu_sub(uint32 op1, uint32 op2, bool should_affect_flags) {
	if(should_affect_flags)
		cspr().set(CSPR_REGISTERS::Carry, op1 >= op2);

	const uint32 v = op1 - op2;

	if(should_affect_flags) {
		const bool overflow =
		        (Bits::bit<31>(op1) != Bits::bit<31>(op2)) &&
		        (Bits::bit<31>(v) == Bits::bit<31>(op2));
		cspr().set(CSPR_REGISTERS::Overflow, overflow);
		_alu_set_flags_logical_op(v);
	}

	return v;
}

uint32 ARM7TDMI::_alu_adc(uint32 op1, uint32 op2, bool should_affect_flags) {
	const uint32 C = cspr().is_set(CSPR_REGISTERS::Carry) ? 1 : 0;

	uint32 result = op1 + op2 + C;

	if(should_affect_flags) {
		const bool new_C = ((uint64)op1 + (uint64)op2 + (uint64)C) > 0xfffffffful;
		const bool new_V =
		        (Bits::bit<31>(op1) == Bits::bit<31>(op2)) &&
		        (Bits::bit<31>(result) != Bits::bit<31>(op1));
		cspr().set(CSPR_REGISTERS::Carry, new_C);
		cspr().set(CSPR_REGISTERS::Overflow, new_V);
		_alu_set_flags_logical_op(result);
	}

	return result;
}

uint32 ARM7TDMI::_alu_sbc(uint32 op1, uint32 op2, bool should_affect_flags) {
	const uint32 C = cspr().is_set(CSPR_REGISTERS::Carry) ? 0 : 1;

	uint32 result = op1 - op2 - C;

	if(should_affect_flags) {
		const bool new_C = ((uint64)op1 >= (uint64)op2 + (uint64)C);
		const bool new_V =
		        (Bits::bit<31>(op1) != Bits::bit<31>(op2)) &&
		        (Bits::bit<31>(result) == Bits::bit<31>(op2));
		cspr().set(CSPR_REGISTERS::Carry, new_C);
		cspr().set(CSPR_REGISTERS::Overflow, new_V);
		_alu_set_flags_logical_op(result);
	}

	return result;
}

uint32 ARM7TDMI::_alu_and(uint32 op1, uint32 op2, bool should_affect_flags) {
	const uint32 v = op1 & op2;
	if(should_affect_flags)
		_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_or(uint32 op1, uint32 op2, bool should_affect_flags) {
	const uint32 v = op1 | op2;
	if(should_affect_flags)
		_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_eor(uint32 op1, uint32 op2, bool should_affect_flags) {
	const uint32 v = op1 ^ op2;
	if(should_affect_flags)
		_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_not(uint32 op, bool should_affect_flags) {
	const uint32 v = ~op;
	if(should_affect_flags)
		_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_lsl(uint32 op1, uint32 op2) {
	auto v = _shift_lsl(op1, op2, true);
	_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_lsr(uint32 op1, uint32 op2) {
	auto v = _shift_lsr(op1, op2, true);
	_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_asr(uint32 op1, uint32 op2) {
	auto v = _shift_asr(op1, op2, true);
	_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_alu_ror(uint32 op1, uint32 op2) {
	auto v = _shift_ror(op1, op2, true);
	_alu_set_flags_logical_op(v);
	return v;
}

uint32 ARM7TDMI::_shift_lsl(uint32 op1, uint32 op2, bool affect_carry) {
	if(op2 == 0)
		return op1;

	if(affect_carry)
		cspr().set(CSPR_REGISTERS::Carry, (op2 <= 32) && (op1 & (1u << (32u - op2))));

	const uint32 v = (op2 < 32) ? static_cast<uint64>((uint64)op1 << op2)
	                            : 0;

	return v;
}

uint32 ARM7TDMI::_shift_lsr(uint32 op1, uint32 op2, bool affect_carry) {
	if(op2 == 0)
		op2 = 32;
	if(affect_carry) {
		if(op2 <= 32)
			cspr().set(CSPR_REGISTERS::Carry, (op1 & (1u << (op2 - 1))));
		else
			cspr().set(CSPR_REGISTERS::Carry, false);
	}

	const uint32 v = (op2 < 32) ? op1 >> op2 : 0;

	return v;
}

uint32 ARM7TDMI::_shift_asr(uint32 op1, uint32 op2, bool affect_carry) {
	if(op2 == 0)
		op2 = 32;

	if(affect_carry) {
		if(op2 < 32)
			cspr().set(CSPR_REGISTERS::Carry, (op1 & (1u << (op2 - 1))));
		else
			cspr().set(CSPR_REGISTERS::Carry, op1 & 0x80000000);
	}

	uint32 v;
	if(op2 >= 32) {
		v = (op1 & 0x80000000u) ? 0xffffffffu : 0x0u;
	} else {
		v = (static_cast<int32>(op1) >> op2);
	}

	return v;
}

uint32 ARM7TDMI::_shift_ror(uint32 op1, uint32 op2, bool affect_carry) {
	if(op2 > 32) op2 %= 32;

	uint32 v;
	if(op2 == 0) {
		v = (op1 >> 1u) | ((cspr().is_set(CSPR_REGISTERS::Carry) ? 1u : 0u) << 31u);
		if(affect_carry)
			cspr().set(CSPR_REGISTERS::Carry, op1 & 1u);
	} else if(op2 == 32) {
		if(affect_carry)
			cspr().set(CSPR_REGISTERS::Carry, op1 & 0x80000000);
		v = op1;
	} else {
		if(affect_carry)
			cspr().set(CSPR_REGISTERS::Carry, (op1 & (1u << (op2 - 1))));
		const unsigned int mask = (32 - 1);
		op2 &= mask;
		v = (op1 >> op2) | (op1 << ((-op2) & mask));
	}

	return v;
}
