#include "Headers/ARM7TDMI.hpp"
#include "MMU/MMU.hpp"

#define ON_CYCLE(a) if(current_instruction().m_cycles == a)

OpExecState ARM7TDMI::B(ARM::BInstruction instr) {
	static uint32 branch_destination {0};
//	static uint32 destination_fetch {0};

	ON_CYCLE(1) {
		branch_destination = const_pc() + instr.offset();
		return OpExecState::Stall;
	}

	ON_CYCLE(2) {
//		destination_fetch = mmu.read32(branch_destination);
		if(instr.is_link())
			r14() = const_pc();

		pc() = branch_destination;
		return OpExecState::Stall;
	}

	ON_CYCLE(3) {
		if(instr.is_link())
			r14() -= 4;
//		if(instr.is_link()) {
//			log("Function call", sp());
//		}

		return OpExecState::Finished;
	}

//		1
//	_reg_fetch(pc)
//	_alu_add(#immediate_offset)
//	_reg_put(pc)
//		2
//	@if is_link()
//		_reg_fetch(pc)
//		_reg_put(r14)
//		3
//	@if is_link()
//		_reg_fetch(r14)
//		_alu_sub(4)
//		_reg_put(r14)

	ASSERT_NOT_REACHED();
}

OpExecState ARM7TDMI::BX(ARM::BXInstruction instr) {
	const auto& reg = creg(instr.reg());
	const bool is_thumb = reg & 1;

	if(is_thumb)
		pc() = reg & ~1u;
	else
		pc() = reg;

	cspr().set_state(!is_thumb ? INSTR_MODE::ARM : INSTR_MODE::THUMB);

//	//  1
//	_reg_fetch(#instr.reg())
//	@if is_thumb()
//		_alu_and(~1u)
//	@else
//		_alu_and(~3u)
//	_reg_put(pc)
//	//  "virtual" 2 and 3

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::DPI(ARM::DataProcessInstruction instr) {
	/*
     *  Lookup table for data processing instructions
     */
	typedef OpExecState (::ARM7TDMI::* DataProcessFunction)(::ARM::DataProcessInstruction);
	static const DataProcessFunction s_dp_instruction_lookup[16] {
			&ARM7TDMI::AND, &ARM7TDMI::EOR, &ARM7TDMI::SUB, &ARM7TDMI::RSB,
			&ARM7TDMI::ADD, &ARM7TDMI::ADC, &ARM7TDMI::SBC, &ARM7TDMI::RSC,
			&ARM7TDMI::TST, &ARM7TDMI::TEQ, &ARM7TDMI::CMP, &ARM7TDMI::CMN,
			&ARM7TDMI::ORR, &ARM7TDMI::MOV, &ARM7TDMI::BIC, &ARM7TDMI::MVN,
	};

	auto func = s_dp_instruction_lookup[instr.opcode()];
	auto ret  = (*this.*func)(instr);

	if (instr.destination_reg() == 15 && instr.should_set_condition()) {
		if(m_excepts.empty()) {
			log("Leaving exception while no exception was captured");
		} else {
			auto v = spsr();
			auto r = m_excepts.back();

			if(const_pc() != r.return_pc) {
				log("  - Return PC [{:08x}] != saved PC [{:08x}]", const_pc(), r.return_pc);
			}

			if(!v.has_value()) {
				log("  - Current mode [{}] does not have an SPSR - cannot return", cspr().mode_str());
				ASSERT_NOT_REACHED();
			}

			if(v->get().mode() != r.entry_mode) {
				log("  - Return mode [{}] != saved return mode [{}]", v->get().mode(), r.entry_mode);
			}

			if(v->get().raw() != r.saved_psr.raw()) {
				log("  - Return PSR [{:08x}] != saved return PSR [{:08x}]", v->get().raw(), r.saved_psr.raw());
			}

			m_excepts.pop_back();
		}

//			log("Leaving exception, current pc={:08x}, spsr={:08x}", const_pc(), spsr().raw());
		auto v = spsr();
		if(!v.has_value()) {
			log("Leaving exception while current mode has no SPSR!");
			ASSERT_NOT_REACHED();
		} else {
			cspr() = *spsr();
		}
	}

	return ret;
}

/*
 *  Arithmetic operations
 */

OpExecState ARM7TDMI::SUB(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, false);
	const bool S = instr.should_set_condition();

	uint32 result = _alu_sub(operand1, operand2, S);
	destination = result;

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::RSB(ARM::DataProcessInstruction instr) {
    auto& destination = reg(instr.destination_reg());
    const auto operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, false);
	const bool S = instr.should_set_condition();

    uint32 result = _alu_sub(operand2, operand1, S);
    destination = result;
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::ADD(ARM::DataProcessInstruction instr) {
    auto& destination = reg(instr.destination_reg());
    const auto operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, false);
	const bool S = instr.should_set_condition();

    uint32 result = _alu_add(operand1, operand2, S);
	destination = result;

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::ADC(ARM::DataProcessInstruction instr) {
    auto& destination = reg(instr.destination_reg());
    const auto operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, false);

	destination = _alu_adc(operand1, operand2, instr.should_set_condition());

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::SBC(ARM::DataProcessInstruction instr) {
    auto& destination = reg(instr.destination_reg());
    const auto operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, false);

    destination = _alu_sbc(operand1, operand2, instr.should_set_condition());

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::RSC(ARM::DataProcessInstruction instr) {
    auto& destination = reg(instr.destination_reg());
    const auto operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, false);

    destination = _alu_sbc(operand2, operand1, instr.should_set_condition());

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::CMP(ARM::DataProcessInstruction instr) {
	//  MRS - transfer PSR contents to reg
	//  Source PSR = SPSR_current
	if(!instr.should_set_condition()) {
		auto& destination = reg(instr.destination_reg());
		auto v = spsr();
		if(!v.has_value())
			log("Undefined behavior: SPSR access in mode with no visible SPSR!");
		else
			destination = v->get().raw();

		return OpExecState::Finished;
	}

//	log("cmp r{}, r{}", instr.operand1_reg(), instr.operand2_reg());

	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, false);

	(void)_alu_sub(operand1, operand2, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::CMN(ARM::DataProcessInstruction instr) {
	//  MSR - transfer register contents to PSR
	//  Destination PSR = SPSR_curr
	if(!instr.should_set_condition()) {
		if(!instr.immediate_is_value()) {
			const auto& source = creg(instr.operand2_reg());
			auto v = spsr();

			if(!v.has_value())
				log("Undefined behavior: SPSR access in mode with no visible SPSR!");
			else
				v->get().set_raw(source);
		} else {
			const uint32 imm = evaluate_shift_operand(instr, false);
			auto v = spsr();

			if(!v.has_value())
				log("Undefined behavior: SPSR access in mode with no visible SPSR!");
			else
				v->get().set_raw(imm);
		}
		return OpExecState::Finished;
	}

	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, false);

	(void)_alu_add(operand1, operand2, true);
	return OpExecState::Finished;
}

/*
 *  Logical operations
 */

OpExecState ARM7TDMI::AND(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_and(operand1, operand2, instr.destination_reg() != 15 && S);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::EOR(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_eor(operand1, operand2, S);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::TST(ARM::DataProcessInstruction instr) {
	//  MRS - transfer PSR contents to reg
	//  Source PSR = CPSR
	if(!instr.should_set_condition()) {
    	auto& destination = reg(instr.destination_reg());
    	destination = cspr().raw();
		return OpExecState::Finished;
    }

    const auto& operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, true);

	(void)_alu_and(operand1, operand2, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::TEQ(ARM::DataProcessInstruction instr) {
	//  MSR - transfer register contents to PSR
	//  Destination PSR = CPSR
	if(!instr.should_set_condition()) {
		const uint32 value = !instr.immediate_is_value() ? creg(instr.operand2_reg())
														 : evaluate_shift_operand(instr, false);
		auto mask = instr.operand1_reg();
		bool f = (mask & 0b1000),
			 s = (mask & 0b0100),
			 x = (mask & 0b0010),
			 c = (mask & 0b0001);
		cspr().set_flags(value, f, s, x, c);

		return OpExecState::Finished;
    }

    const auto& operand1 = creg(instr.operand1_reg());
    uint32 operand2 = evaluate_shift_operand(instr, true);

	(void)_alu_eor(operand1, operand2, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::ORR(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_or(operand1, operand2, S);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::MOV(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);

	destination = operand2;
//	if(instr.destination_reg() == 15 && instr.operand2_reg() == 14) {
//		log("Function return");
//	}

	if(instr.should_set_condition())
		_alu_set_flags_logical_op(operand2);

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::BIC(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto& operand1 = creg(instr.operand1_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_and(operand1, _alu_not(operand2, S), S);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::MVN(ARM::DataProcessInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	uint32 operand2 = evaluate_shift_operand(instr, true);
	const bool S = instr.should_set_condition();

	destination = _alu_not(operand2, S);
	return OpExecState::Finished;
}


/*
 *  Data transfer operations
 */

OpExecState ARM7TDMI::HDT(ARM::HDTInstruction instr) {
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

	switch (instr.opcode()) {
		case 0: ASSERT_NOT_REACHED();
		case 1: {//  Unsigned hword
			if(instr.load_from_memory()) {
				auto word = static_cast<uint32>(m_mmu.read16(address & ~1u));

				if(address&1)   //  Misaligned LDRH
					word = rotr32(word, 8);

				word_for_load = word;
			} else
				m_mmu.write16(address & ~1u, target & 0xFFFF);

			break;
		}
		case 2: {//  Signed byte
			if(instr.load_from_memory())
				word_for_load = sign_extend<8>(m_mmu.read8(address));
			else
				m_mmu.write8(address, target);

			break;
		}
		case 3: {//  Signed hword
			if(instr.load_from_memory()) {
				uint32 word;

				if(address & 1u)
					word = sign_extend<8>(m_mmu.read8(address));
				else
					word = sign_extend<16>(m_mmu.read16(address));

				word_for_load = word;
			} else
				m_mmu.write16(address & ~1u, target & 0xFFFF);

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

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::SDT(ARM::SDTInstruction instr) {
    uint32 offset;
    if(instr.immediate_is_offset())
        offset = instr.offset();
    else {
		const auto shift_instr = ARM::DataProcessInstruction(instr.offset());
	    offset = evaluate_shift_operand(shift_instr); //  FIXME: Hacky hack z
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
			word = static_cast<uint32>(m_mmu.read8(address));;
        } else {
        	word = m_mmu.read32(address & ~3u);  //  Force align
        	//  Rotate
        	if(address & 3u)
				word = rotr32(word, (address&3u)*8);
        }

	    if(!instr.preindex())
		    address = get_target_address();
	    if((instr.writeback() || !instr.preindex()) && instr.base_reg() != 15)
		    reg(instr.base_reg()) = address;

	    target = word;
    } else {
	    const auto& target = creg(instr.target_reg());

		if(instr.quantity_in_bytes())
	    	m_mmu.write8(address, target + (instr.target_reg() == 15 ? 4 : 0));
		else
        	m_mmu.write32(address & ~3u, target + (instr.target_reg() == 15 ? 4 : 0));

        if(!instr.preindex())
		    address = get_target_address();
	    if((instr.writeback() || !instr.preindex()) && instr.base_reg() != 15)
		    reg(instr.base_reg()) = address;
    }

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::SWP(ARM::SWPInstruction instr) {
//	log("SWP op={:08x}", instr.m_data);

	const uint32 swap_address = creg(instr.base_reg());
	const uint32 source = creg(instr.source_reg());
	auto& dest = reg(instr.destination_reg());

	if(instr.swap_byte()) {
		const auto prev_contents = m_mmu.read8(swap_address);
		m_mmu.write8(swap_address, source & 0xFFu);
		dest = static_cast<uint32>(prev_contents);
	} else {
		uint32 prev_contents = m_mmu.read32(swap_address & ~3u);
		if(swap_address & 3u)
			prev_contents = _shift_ror(prev_contents, (swap_address&3u)*8);
		m_mmu.write32(swap_address & ~3u, source);
		dest = prev_contents;
	}
	return OpExecState::Finished;
}


OpExecState ARM7TDMI::SWI(ARM::SWIInstruction instr) {
	ON_CYCLE(1) {
		log("Exception SWI, return={:08x}, cpsr={:08x}", const_pc()-4, cspr().raw());
		m_excepts.push_back({.return_pc = const_pc()-4, .entry_mode = cspr().mode(), .saved_psr = cspr()});

		m_registers.m_gSVC[1] = const_pc();
		m_saved_status.m_SVC = cspr();
		cspr().set_mode(PRIV_MODE::SVC);
		cspr().set(CSPR_REGISTERS::IRQn, true);

		return OpExecState::Stall;
	}

	ON_CYCLE(2) {
		cspr().set_state(INSTR_MODE::ARM);
		pc() = 0x08;
		lr() -= 4;

		return OpExecState::Finished;
	}

	ASSERT_NOT_REACHED();

	raise_exception(ExceptionVector::SWI);
//	dump_regs();
//	ASSERT_NOT_REACHED();
	return OpExecState::Finished;
}



OpExecState ARM7TDMI::MLL(ARM::MultLongInstruction instr) {
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
		cspr().set(CSPR_REGISTERS::Carry, true);   //  FIXME: ???
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::MUL(ARM::MultInstruction instr) {
	auto& destination = reg(instr.destination_reg());
	const auto& m = creg(instr.multiplicand_reg());
	const auto& s = creg(instr.source_reg());
	const auto& n = creg(instr.accumulate_reg());

	uint32 result = m*s + ((instr.should_accumulate()) ? n : 0);
	destination = result;

	if(instr.should_set_condition()) {
		cspr().set(CSPR_REGISTERS::Zero, result == 0);
		cspr().set(CSPR_REGISTERS::Negative, result & (1u << 31u));
		cspr().set(CSPR_REGISTERS::Carry, false); //  "is set to a meaningless value"
	}

	return OpExecState::Finished;
}


OpExecState ARM7TDMI::BDT(ARM::BDTInstruction instr) {
	const auto& base = creg(instr.base_reg());
	auto address = base;

	if(instr.is_rlist_empty()) {
		if(instr.load_from_memory())
			pc() = m_mmu.read32(base & ~3u);
		else
			m_mmu.write32(base & ~3u, pc() + 4);

		reg(instr.base_reg()) = (instr.add_offset_to_base()) ? base + 0x40
															 : base - 0x40;

		return OpExecState::Finished;
	}

	auto advance_address = [&instr, &address] {
		if(instr.add_offset_to_base())
			address += 4;
		else
			address -= 4;
	};

	if(instr.load_from_memory()) {
		int8 i = (instr.add_offset_to_base()) ? 0 : 15;
		int8 end_at = (instr.add_offset_to_base()) ? 16 : -1;

		while(i != end_at) {
			if(instr.is_register_in_list(i)) {
				if(instr.preindex()) advance_address();
				uint32 saved = m_mmu.read32(address & ~3u);

				if(instr.PSR()) {
					m_registers.m_base[i] = saved;
					if(i == 15) {
						log("LDM - Mode change");
						auto v = spsr();
						if(v.has_value())
							cspr() = *spsr();
						else
							log("Undefined behavior: SPSR access in mode with no visible SPSR!");
					}
				} else {
//					if(i == 14) {
//						log("Loading r{} <- [{:08x}] from address {:08x}", i, saved, address);
//
////						if(!m_lr_saves.empty()) {
////							auto save = m_lr_saves.back();
////
////							if(save.m_address != address) {
////								log("Loading r14 from mismatched save! old={:08x} new={:08x}", save.m_address, address);
////								assert(false);
////							}
////							if(save.m_data != saved) {
////								log("Loading r14 from modified stack! old={:08x} new={:08x}", save.m_data, saved);
////								assert(false);
////							}
////
////							m_lr_saves.pop_back();
////						}
//					}


					reg(i) = saved;
				}

				if(!instr.preindex()) advance_address();
			}

			if(instr.add_offset_to_base())
				i++;
			else
				i--;
		}
	} else {
		int8 i = (instr.add_offset_to_base()) ? 0 : 15;
		int8 end_at = (instr.add_offset_to_base()) ? 16 : -1;

		while(i != end_at) {
			if(instr.is_register_in_list(i)) {
				if(instr.preindex()) advance_address();
				const auto offset_for_r15 = ((i == 15) ? 4 : 0);
				if(instr.PSR()) {
					log("STM - Save reg");
					m_mmu.write32(address & ~3u, m_registers.m_base[i] + offset_for_r15); //  Save regs from user bank, not current mode
				} else {
//					log("Saving r{} to address {:08x}", i, address);
//					if(i == 14) {
//						m_lr_saves.push_back({.m_address = address, .m_data = creg(i)});
//					}

					m_mmu.write32(address & ~3u, creg(i) + offset_for_r15);
				}
				if(!instr.preindex()) advance_address();
			}

			if(instr.add_offset_to_base())
				i++;
			else
				i--;
		}
	}

	//  Quirk: Writeback with Rb in Rlist
	//  FIXME: This isn't actually correct
	//	if(instr.is_register_in_list(instr.base_reg()) && instr.is_register_first_in_rlist(instr.base_reg())) {
	//		address = base;
	//	}

	if(instr.writeback() && !(instr.load_from_memory() && instr.is_register_in_list(instr.base_reg())))
		reg(instr.base_reg()) = address;

	return OpExecState::Finished;
}

