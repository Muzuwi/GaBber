#include "CPU/ARM7TDMI.hpp"
#include "Headers/Bits.hpp"
#include "MMU/BusInterface.hpp"

void ARM7TDMI::B(ARM::BInstruction instr) {
	const auto old_pc = const_pc();

	pc() = const_pc() + instr.offset();
	if(instr.is_link())
		r14() = old_pc - 4;

	m_wait_cycles += 2 /*S*/ + 1 /*N*/;
}

void ARM7TDMI::BX(ARM::BXInstruction instr) {
	const auto& reg = creg(instr.reg());
	const bool is_thumb = reg & 1;

	if(is_thumb)
		pc() = reg & ~1u;
	else
		pc() = reg;

	cspr().set_state(!is_thumb ? INSTR_MODE::ARM : INSTR_MODE::THUMB);

	m_wait_cycles += 2 /*S*/ + 1 /*N*/;
}

void ARM7TDMI::DPI(ARM::DataProcessInstruction instr) {
	/*
	 *  Lookup table for data processing instructions
	 */
	typedef void (::ARM7TDMI::*DataProcessFunction)(::ARM::DataProcessInstruction);
	static const DataProcessFunction s_dp_instruction_lookup[16] {
		&ARM7TDMI::AND,
		&ARM7TDMI::EOR,
		&ARM7TDMI::SUB,
		&ARM7TDMI::RSB,
		&ARM7TDMI::ADD,
		&ARM7TDMI::ADC,
		&ARM7TDMI::SBC,
		&ARM7TDMI::RSC,
		&ARM7TDMI::TST,
		&ARM7TDMI::TEQ,
		&ARM7TDMI::CMP,
		&ARM7TDMI::CMN,
		&ARM7TDMI::ORR,
		&ARM7TDMI::MOV,
		&ARM7TDMI::BIC,
		&ARM7TDMI::MVN,
	};

	auto func = s_dp_instruction_lookup[instr.opcode()];
	(*this.*func)(instr);

	if(instr.destination_reg() == 15 && instr.should_set_condition()) {
		auto v = spsr();
		if(v.has_value()) {
			cspr() = *spsr();
			//			log("Leaving exception, pc={:08x}, cspr={:08x}", const_pc(), cspr().raw());
		}
	}

	m_wait_cycles += 1 /*S*/;
	if(instr.destination_reg() == 15) {
		m_wait_cycles += 1 /*S*/ + 1 /*N*/;
	}
	if(instr.immediate_is_value() && instr.is_shift_reg()) {
		m_wait_cycles += 1 /*I*/;
	}
}

/*
 *  Arithmetic operations
 */

void ARM7TDMI::SUB(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);
	const bool S = instr.should_set_condition();

	uint32 result = _alu_sub(operand1, operand2, S);
	destination = result;
}

void ARM7TDMI::RSB(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);
	const bool S = instr.should_set_condition();

	uint32 result = _alu_sub(operand2, operand1, S);
	destination = result;
}

void ARM7TDMI::ADD(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);
	const bool S = instr.should_set_condition();

	uint32 result = _alu_add(operand1, operand2, S);
	destination = result;
}

void ARM7TDMI::ADC(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);

	destination = _alu_adc(operand1, operand2, instr.should_set_condition());
}

void ARM7TDMI::SBC(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);

	destination = _alu_sbc(operand1, operand2, instr.should_set_condition());
}

void ARM7TDMI::RSC(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);

	destination = _alu_sbc(operand2, operand1, instr.should_set_condition());
}

void ARM7TDMI::CMP(ARM::DataProcessInstruction instr) {
	//  MRS - transfer PSR contents to reg
	//  Source PSR = SPSR_current
	if(!instr.should_set_condition()) {
		auto& destination = reg(instr.destination_reg());
		auto v = spsr();
		if(!v.has_value()) {
			log("Undefined behavior: SPSR access in mode with no visible SPSR!");
			destination = cspr().raw();
		} else {
			destination = v->get().raw();
		}

		return;
	}

	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);

	(void)_alu_sub(operand1, operand2, true);
}

void ARM7TDMI::CMN(ARM::DataProcessInstruction instr) {
	//  MSR - transfer register contents to PSR
	//  Destination PSR = SPSR_curr
	if(!instr.should_set_condition()) {
		if(!instr.immediate_is_value()) {
			const auto& source = creg(instr.operand2_reg());
			auto v = spsr();

			if(!v.has_value()) {
				log("Undefined behavior: SPSR access in mode with no visible SPSR!");
				cspr().set_raw(source);
			} else {
				v->get().set_raw(source);
			}
		} else {
			const uint32 imm = evaluate_operand2(instr, false);
			auto v = spsr();

			if(!v.has_value()) {
				log("Undefined behavior: SPSR access in mode with no visible SPSR!");
				cspr().set_raw(imm);
			} else {
				v->get().set_raw(imm);
			}
		}
		return;
	}

	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, false);

	(void)_alu_add(operand1, operand2, true);
}

/*
 *  Logical operations
 */

void ARM7TDMI::AND(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_and(operand1, operand2, instr.destination_reg() != 15 && S);
}

void ARM7TDMI::EOR(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_eor(operand1, operand2, S);
}

void ARM7TDMI::TST(ARM::DataProcessInstruction instr) {
	//  MRS - transfer PSR contents to reg
	//  Source PSR = CPSR
	if(!instr.should_set_condition()) {
		auto& destination = reg(instr.destination_reg());
		destination = cspr().raw();
		return;
	}

	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);

	(void)_alu_and(operand1, operand2, true);
}

void ARM7TDMI::TEQ(ARM::DataProcessInstruction instr) {
	//  MSR - transfer register contents to PSR
	//  Destination PSR = CPSR
	if(!instr.should_set_condition()) {
		const uint32 value = !instr.immediate_is_value() ? creg(instr.operand2_reg())
		                                                 : evaluate_operand2(instr, false);
		auto mask = instr.operand1_reg();
		bool f = (mask & 0b1000),
		     s = (mask & 0b0100),
		     x = (mask & 0b0010),
		     c = (mask & 0b0001);
		cspr().set_flags(value, f, s, x, c);

		return;
	}

	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);

	(void)_alu_eor(operand1, operand2, true);
}

void ARM7TDMI::ORR(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_or(operand1, operand2, S);
}

void ARM7TDMI::MOV(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	uint32 operand2 = evaluate_operand2(instr, true);

	destination = operand2;

	if(instr.should_set_condition())
		_alu_set_flags_logical_op(operand2);
}

void ARM7TDMI::BIC(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 operand1 = evaluate_operand1(instr);
	uint32 operand2 = evaluate_operand2(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_and(operand1, _alu_not(operand2, S), S);
}

void ARM7TDMI::MVN(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	uint32 operand2 = evaluate_operand2(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_not(operand2, S);
}

/*
 *  Data transfer operations
 */

void ARM7TDMI::HDT(ARM::HDTInstruction instr) {
	const auto& base = creg(instr.base_reg());
	const auto& target = creg(instr.target_reg());
	auto offset = instr.is_offset_immediate() ? instr.immediate()
	                                          : creg(instr.offset_reg_or_immediate_low());

	auto address = base;
	if(instr.preindex())
		address = instr.add_offset_to_base() ? base + offset
		                                     : base - offset;

	//  For easier implementing of writeback with the same register
	uint32 word_for_load {};

	switch(instr.opcode()) {
		case 0: ASSERT_NOT_REACHED();
		case 1: {//  Unsigned hword
			if(instr.load_from_memory()) {
				auto word = static_cast<uint32>(mem_read16(address & ~1u));

				if(address & 1)//  Misaligned LDRH
					word = Bits::rotr32(word, 8);

				word_for_load = word;
			} else
				mem_write16(address & ~1u, target & 0xFFFF);

			break;
		}
		case 2: {//  Signed byte
			if(instr.load_from_memory())
				word_for_load = Bits::sign_extend<8>(mem_read8(address));
			else
				mem_write8(address, target);

			break;
		}
		case 3: {//  Signed hword
			if(instr.load_from_memory()) {
				uint32 word;

				if(address & 1u)
					word = Bits::sign_extend<8>(mem_read8(address));
				else
					word = Bits::sign_extend<16>(mem_read16(address));

				word_for_load = word;
			} else
				mem_write16(address & ~1u, target & 0xFFFF);

			break;
		}
		default: ASSERT_NOT_REACHED();
	}

	if(!instr.preindex())
		address = instr.add_offset_to_base() ? base + offset
		                                     : base - offset;
	if(instr.writeback() || !instr.preindex())
		reg(instr.base_reg()) = address;

	if(instr.load_from_memory())
		reg(instr.target_reg()) = word_for_load;

	if(instr.load_from_memory()) {
		m_wait_cycles += 1 /*S*/ + 1 /*N*/ + 1 /*I*/;
		if(instr.target_reg() == 15) {
			m_wait_cycles += 1 /*S*/ + 1 /*N*/;
		}
	} else {
		m_wait_cycles += 2 /*N*/;
	}
}

void ARM7TDMI::SDT(ARM::SDTInstruction instr) {
	uint32 offset;
	if(instr.immediate_is_offset())
		offset = instr.offset();
	else {
		const auto shift_instr = ARM::DataProcessInstruction(instr.offset());
		offset = evaluate_operand2(shift_instr);//  FIXME: Hacky hack z
	}

	const auto& base = creg(instr.base_reg());

	auto get_target_address = [&]() -> uint32 {
		if(instr.add_offset_to_base())
			return base + offset;
		else
			return base - offset;
	};

	uint32 address = base;
	if(instr.preindex())
		address = get_target_address();

	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		uint32 word;
		if(instr.quantity_in_bytes()) {
			word = static_cast<uint32>(mem_read8(address));
		} else {
			word = mem_read32(address & ~3u);//  Force align
			//  Rotate
			if(address & 3u)
				word = Bits::rotr32(word, (address & 3u) * 8);
		}

		if(!instr.preindex())
			address = get_target_address();
		if((instr.writeback() || !instr.preindex()) && instr.base_reg() != 15)
			reg(instr.base_reg()) = address;

		target = word;
		m_wait_cycles += 1 /*S*/ + 1 /*N*/ + 1 /*I*/;
		if(instr.target_reg() == 15) {
			m_wait_cycles += 1 /*S*/ + 1 /*N*/;
		}
	} else {
		const auto& target = creg(instr.target_reg());

		if(instr.quantity_in_bytes())
			mem_write8(address, target + (instr.target_reg() == 15 ? 4 : 0));
		else
			mem_write32(address & ~3u, target + (instr.target_reg() == 15 ? 4 : 0));

		if(!instr.preindex())
			address = get_target_address();
		if((instr.writeback() || !instr.preindex()) && instr.base_reg() != 15)
			reg(instr.base_reg()) = address;

		m_wait_cycles += 2 /*N*/;
	}
}

void ARM7TDMI::SWP(ARM::SWPInstruction instr) {
	const uint32 swap_address = creg(instr.base_reg());
	const uint32 source = creg(instr.source_reg());
	auto& dest = reg(instr.destination_reg());

	if(instr.swap_byte()) {
		const auto prev_contents = mem_read8(swap_address);
		mem_write8(swap_address, source & 0xFFu);
		dest = static_cast<uint32>(prev_contents);
	} else {
		uint32 prev_contents = mem_read32(swap_address & ~3u);
		if(swap_address & 3u)
			prev_contents = _shift_ror(prev_contents, (swap_address & 3u) * 8);
		mem_write32(swap_address & ~3u, source);
		dest = prev_contents;
	}
	m_wait_cycles += 1 /*S*/ + 2 /*N*/ + 1 /*I*/;
}

void ARM7TDMI::SWI(ARM::SWIInstruction) {
	enter_swi();
}

void ARM7TDMI::MLL(ARM::MultLongInstruction instr) {
	auto& lower = reg(instr.destLo_reg());
	auto& higher = reg(instr.destHi_reg());

	const uint32 m = creg(instr.operand1_reg());
	const uint32 s = creg(instr.operand2_reg());
	const uint64 acc = (static_cast<uint64>(higher) << 32u) | lower;
	const uint64 offset = (instr.should_accumulate() ? acc : 0);

	uint64 result;
	if(instr.is_signed()) {
		result = static_cast<int64>(static_cast<int32>(m)) * static_cast<int64>(static_cast<int32>(s)) + offset;
	} else {
		result = static_cast<uint64>(m) * static_cast<uint64>(s) + offset;
	}

	lower = static_cast<uint32>(result & 0xFFFFFFFFu);
	higher = static_cast<uint32>(result >> 32u);

	if(instr.should_set_condition()) {
		cspr().set(CSPR_REGISTERS::Zero, result == 0);
		cspr().set(CSPR_REGISTERS::Negative, result & (1ul << 63ul));
		cspr().set(CSPR_REGISTERS::Carry, true);//  FIXME: ???
	}

	m_wait_cycles += 1 /*S*/
	                 + (instr.is_signed() ? ARM::mult_m_cycles(m) : ARM::unsigned_mult_m_cycles(m)) + 1 + (instr.should_accumulate() ? 1 : 0);
}

void ARM7TDMI::MUL(ARM::MultInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const uint32 m = creg(instr.multiplicand_reg());
	const uint32 s = creg(instr.source_reg());
	const uint32 n = creg(instr.accumulate_reg());

	uint32 result = m * s + ((instr.should_accumulate()) ? n : 0);
	destination = result;

	if(instr.should_set_condition()) {
		cspr().set(CSPR_REGISTERS::Zero, result == 0);
		cspr().set(CSPR_REGISTERS::Negative, result & (1u << 31u));
		cspr().set(CSPR_REGISTERS::Carry, false);//  "is set to a meaningless value"
	}

	m_wait_cycles += 1 /*S*/ + ARM::mult_m_cycles(s) + (instr.should_accumulate() ? 1 : 0);
}

void ARM7TDMI::BDT(ARM::BDTInstruction instr) {
	const auto& base = creg(instr.base_reg());
	uint32 address = base;

	//  FIXME: All these offset variations
	if(instr.is_rlist_empty()) {
		if(instr.load_from_memory())
			pc() = mem_read32(base & ~3u);
		else
			mem_write32(base & ~3u, const_pc() + 4);

		reg(instr.base_reg()) = (instr.add_offset_to_base()) ? base + 0x40
		                                                     : base - 0x40;
		//  STMDA -60 from base, writeback at -4
		//  STMDB -64 from base, writeback at written
		//  STMIB  +4 after base, writeback +64
		//  STMIA  at base, writeback +64
		//  LDMDA ??
		//  LDMDB ??
		//  LDMIB ??
		//  LDMIA ??

		//  FIXME: Emulate cycles
		m_wait_cycles += 1;
		return;
	}

	unsigned n = 0;
	for(unsigned r = 0; r < 16; ++r) {
		const uint8 reg = instr.add_offset_to_base() ? r : (15 - r);
		if(!instr.is_register_in_list(reg)) continue;

		if(instr.preindex()) address += (instr.add_offset_to_base()) ? 4 : -4;

		++n;
		if(instr.load_from_memory()) {
			uint32 word = mem_read32(address & ~3u);

			uint32& gpr = (instr.PSR() ? m_registers.m_base[reg] : this->reg(reg));
			gpr = word;

			if(instr.PSR() && reg == 15) {
				auto v = spsr();
				if(v.has_value())
					cspr() = *spsr();
				else
					log("Undefined behavior: LDM - SPSR access in mode with no visible SPSR!");
			}
		} else {
			const auto offset_for_r15 = ((reg == 15) ? 4 : 0);

			uint32 word = (instr.PSR() ? m_registers.m_base[reg] : this->creg(reg));
			//  Quirk with writeback and base in Rlist
			if(instr.writeback() && reg == instr.base_reg() && !instr.is_register_first_in_rlist(instr.base_reg())) {
				word = base + instr.total_offset();
			}

			mem_write32(address & ~3u, word + offset_for_r15);
		}

		if(!instr.preindex()) address += (instr.add_offset_to_base()) ? 4 : -4;
	}

	if(instr.writeback() && !(instr.load_from_memory() && instr.is_register_in_list(instr.base_reg()))) {
		reg(instr.base_reg()) = address;
	}

	if(instr.load_from_memory()) {
		m_wait_cycles += n /*S*/ + 1 /*N*/ + 1 /*I*/;
		if(instr.is_register_in_list(15)) {
			m_wait_cycles += 1 /*S*/ + 1 /*N*/;
		}
	} else {
		m_wait_cycles += (n - 1) /*S*/ + 2 /*N*/;
	}
}
