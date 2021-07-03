#include "MMU/MMU.hpp"
#include "Headers/ARM7TDMI.hpp"
#include <fmt/format.h>

OpExecState ARM7TDMI::THUMB_ALU(THUMB::InstructionFormat4 instr) {
	/*
     *  Lookup table for THUMB format 4 instruction (ALU operations)
     */
	typedef OpExecState (::ARM7TDMI::* ThumbAluOperation)(::THUMB::InstructionFormat4);
	static const ThumbAluOperation s_thumb_alu_lookup[16] {
		&ARM7TDMI::THUMB_AND, &ARM7TDMI::THUMB_EOR, &ARM7TDMI::THUMB_LSL, &ARM7TDMI::THUMB_LSR,
		&ARM7TDMI::THUMB_ASR, &ARM7TDMI::THUMB_ADC, &ARM7TDMI::THUMB_SBC, &ARM7TDMI::THUMB_ROR,
		&ARM7TDMI::THUMB_TST, &ARM7TDMI::THUMB_NEG, &ARM7TDMI::THUMB_CMP, &ARM7TDMI::THUMB_CMN,
		&ARM7TDMI::THUMB_ORR, &ARM7TDMI::THUMB_MUL, &ARM7TDMI::THUMB_BIC, &ARM7TDMI::THUMB_MVN,
	};

	auto func = s_thumb_alu_lookup[instr.opcode()];
	return (*this.*func)(instr);
}

OpExecState ARM7TDMI::THUMB_FMT1(THUMB::InstructionFormat1 instr) {
	const auto& source = creg(instr.source_reg());
	auto& destination = reg(instr.destination_reg());
	const auto offset = instr.immediate();
	const auto opcode = instr.opcode();

	switch (opcode) {
		case 0: {
			destination = _alu_lsl(source, offset);
			break;
		}
		case 1: {
			destination = _alu_lsr(source, offset);
			break;
		}
		case 2: {
			destination = _alu_asr(source, offset);
			break;
		}
		default: {
			fmt::print("ARM7TDMI/ Invalid opcode {:04x} for THUMB mov shifted at pc {:08x}, instr {:04x}",
						opcode, pc() - 4, instr.raw());
			break;
		}
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT2(THUMB::InstructionFormat2 instr) {
	const auto& source = creg(instr.source_reg());
	auto& destination = reg(instr.destination_reg());

	uint32 operand2 = (instr.immediate_is_value()) ? (instr.immediate()) : (creg(instr.immediate()));

	uint32 result = (instr.subtract()) ? _alu_sub(source, operand2, true)
									   : _alu_add(source, operand2, true);
	destination = result;
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT3(THUMB::InstructionFormat3 instr) {
	switch (instr.opcode()) {
		case 0: {   //  MOV
			auto& target_reg = reg(instr.target_reg());
			target_reg = instr.immediate();
			_alu_set_flags_logical_op(instr.immediate());
			break;
		}
		case 1: {   //  CMP
			const auto& target_reg = creg(instr.target_reg());
			(void)_alu_sub(target_reg, instr.immediate(), true);
			break;
		}
		case 2: {   //  ADD
			auto& target_reg = reg(instr.target_reg());
			target_reg = _alu_add(target_reg, instr.immediate(), true);
			break;
		}
		case 3: {   //  SUB
			auto& target_reg = reg(instr.target_reg());
			target_reg = _alu_sub(target_reg, instr.immediate(), true);
			break;
		}
		default: ASSERT_NOT_REACHED();
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT5(THUMB::InstructionFormat5 instr) {
	const auto& source = creg(instr.source_reg());

	switch (instr.opcode()) {
		case 0: {
			auto& destination = reg(instr.destination_reg());
			destination = source + destination;
			break;
		}
		case 1: {
			const auto& destination = creg(instr.destination_reg());
			(void)_alu_sub(destination, source, true);
			break;
		}
		case 2: {
			auto& destination = reg(instr.destination_reg());
			destination = source;
			break;
		}
		case 3: {
			if(!instr.MSBd()) {
				cspr().set_state((source & 1) ? INSTR_MODE::THUMB : INSTR_MODE::ARM);
				if(!(source & 1))
					pc() = source & ~2u;
				else
					pc() = source & ~1u;
			}
			break;
		}
		default: ASSERT_NOT_REACHED();
	}
	return OpExecState::Finished;
}




OpExecState ARM7TDMI::THUMB_FMT6(THUMB::InstructionFormat6 instr) {
	auto& destination = reg(instr.destination_reg());
	const auto immediate_shifted = static_cast<uint16>(instr.immediate()) << 2u;
	const auto aligned_pc = const_pc() & ~0x3;
	destination = static_cast<uint32>(m_mmu.read32(aligned_pc + immediate_shifted));
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT7(THUMB::InstructionFormat7 instr) {
	const auto& base = creg(instr.base_reg());
	const auto& offset = creg(instr.offset_reg());

	auto address = base + offset;
	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		if(instr.quantity_in_bytes())
			target = m_mmu.read8(address);
		else {
			auto word = m_mmu.read32(address & ~3u);  //  Force align
			//  Rotate
			if(address & 3u) {
				word = rotr32(word, (address&3u)*8);
			}

			target = word;
		}
	}
	else {
		const auto target = creg(instr.target_reg());

		if(instr.quantity_in_bytes())
			m_mmu.write8(address, target & 0xFFu);
		else
			m_mmu.write32(address, target);
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT8(THUMB::InstructionFormat8 instr) {
	const auto& base = creg(instr.base_reg());
	const auto& offset = creg(instr.offset_reg());
	const auto address = base + offset;

	switch (instr.opcode()) {
		case 0: {
			const auto destination = creg(instr.destination_reg());
			m_mmu.write16(address & ~1u, destination & 0xffff);
			break;
		}
		case 1: {
			auto& destination = reg(instr.destination_reg());

			uint8 val = m_mmu.read8(address);
			destination = sign_extend<8>(val);
			break;
		}
		case 2: {
			auto& destination = reg(instr.destination_reg());

			uint32 word;
			word = static_cast<uint32>(m_mmu.read16(address & ~1u));
			if(address&1u) {
				log("Undefined behaviour: LDRH with unaligned address!");
				word = rotr32(word, 8);
			}

			destination = word;
			break;
		}
		case 3: {
			auto& destination = reg(instr.destination_reg());

			uint32 word;
			if(address&1u) {
				log("Undefined behaviour: LDRSH with unaligned address!");
				auto byte = m_mmu.read8(address);
				word = sign_extend<8>(byte);
			} else {
				auto hword = m_mmu.read16(address);
				word = sign_extend<16>(hword);
			}

			destination = word;
			break;
		}

		default: ASSERT_NOT_REACHED();
	}
	return OpExecState::Finished;
}


OpExecState ARM7TDMI::THUMB_FMT9(THUMB::InstructionFormat9 instr) {
	const auto& base = creg(instr.base_reg());

	auto offset = static_cast<uint16>(instr.offset());
	if(!instr.quantity_in_bytes())
		offset <<= 2u;
	uint32 address = base + offset;

	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		if(instr.quantity_in_bytes())
			target = m_mmu.read8(address);
		else {
			auto word = m_mmu.read32(address & ~3u);  //  Force align
//			log("LDR {:08x} from {:08x}", word, address);
			//  Rotate
			if(address & 3u) {
				word = rotr32(word, (address&3u)*8);
			}

			target = word;
		}
	}
	else {
		const auto target = creg(instr.target_reg());

		if(instr.quantity_in_bytes())
			m_mmu.write8(address, target & 0xFFu);
		else
			m_mmu.write32(address, target);
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT10(THUMB::InstructionFormat10 instr) {
	const auto& base = creg(instr.base_reg());
	const auto offset = instr.offset() << 1u;
	uint32 address = base + offset;

	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		uint32 word;
		word = static_cast<uint32>(m_mmu.read16(address & ~1u));
		if(address&1u) {
			log("Undefined behaviour: LDRH with unaligned address!");
			word = rotr32(word, 8);
		}

		target = word;
	} else {
		const auto target = creg(instr.target_reg());

		m_mmu.write16(address & ~1u, target & 0xFFFF);
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT11(THUMB::InstructionFormat11 instr) {
	const auto address = creg(13) + (static_cast<uint16>(instr.immediate()) << 2u);

	if(instr.load_from_memory()) {
		auto& destination = reg(instr.destination_reg());

		auto word = m_mmu.read32(address & ~3u);  //  Force align

		//  Rotate
		if(address & 3u) {
			word = rotr32(word, (address&3u)*8);
		}

		destination = word;
	} else {
		const auto destination = creg(instr.destination_reg());

		m_mmu.write32(address & ~3u, destination);
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT12(THUMB::InstructionFormat12 instr) {
	auto& destination = reg(instr.destination_reg());

	const auto offset = static_cast<uint16>(instr.immediate()) << 2u;
	auto address = (instr.source_is_sp()) ? creg(13) + offset
										  : (const_pc()&~0b10u) + offset;

	destination = address;
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT13(THUMB::InstructionFormat13 instr) {
	const auto offset = static_cast<uint16>(instr.offset()) << 2u;

	sp() = (instr.offset_is_negative()) ? sp() - offset
										: sp() + offset;
	return OpExecState::Finished;
}


OpExecState ARM7TDMI::THUMB_FMT14(THUMB::InstructionFormat14 instr) {
	//  POP {Rlist}
	if(instr.load_from_memory()) {
		for(int8 i = 0; i < 8; ++i) {
			if(instr.is_register_in_list(i)) {
				reg(i) = stack_pop32();
//				std::cout << "Restore reg" <<  std::to_string(i) << " " << std::hex << reg_by_number(i) << std::dec << "\n";
			}
		}
		if(instr.store_lr_load_pc()) {
//			if(!m_lr_saves.empty()) {
//				auto save = m_lr_saves.back();
//				if(save.m_address != sp()) {
//					log("THUMB loading lr from invalid stack addr, old={:08x} new={:08x}", save.m_address, cr13());
//					assert(false);
//				}
//			}

			pc() = stack_pop32() & ~0x1;

//			if(!m_lr_saves.empty()) {
//				auto save = m_lr_saves.back();
//				if(save.m_data != pc()) {
//					log("THUMB corrupted lr old={:08x} new={:08x}", save.m_data, pc());
//					assert(false);
//				}
//				m_lr_saves.pop_back();
//			}

//			std::cout << "Restore pc " << std::hex << pc() << std::dec << "\n";
		}
	}
	//  PUSH {Rlist}
	else {
		if(instr.store_lr_load_pc()) {
//			std::cout << "save lr " << std::hex << lr() << std::dec << "\n";
//			log("THUMB save sp addr={:08x}", sp() - 4);
//			m_lr_saves.push_back({.m_address = sp() - 4, .m_data = lr()});
			stack_push32(lr());
		}
		for(int8 i = 8; i >= 0; --i) {
			if(instr.is_register_in_list(i)) {
				stack_push32(creg(i));
//				std::cout << "Save reg" <<  std::to_string(i) << " " << std::hex << reg_by_number(i) << std::dec << "\n";
			}
		}
	}
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT15(THUMB::InstructionFormat15 instr) {
	if(instr.is_rlist_empty()) {
		auto& base = reg(instr.base_reg());

		if(instr.load_from_memory()) {
			pc() = m_mmu.read32(base & ~3u);
		} else {
//			log("RList empty store to {:08x}, raw {:08x}, pc={:08x}", base&~3u, base, pc());
			m_mmu.write32(base & ~3u, const_pc() + 2);
		}

		base += 0x40;
		return OpExecState::Finished;
	}

	const auto& base = creg(instr.base_reg());
	auto ptr = base;

	if(instr.load_from_memory()) {
		for(uint8 i = 0; i < 8; ++i) {
			if(instr.is_register_in_list(i)) {
				reg(i) = m_mmu.read32(ptr & ~3u);
				ptr += 4;
			}
		}
	} else {
		for(uint8 i = 0; i < 8; ++i) {
			if(instr.is_register_in_list(i)) {
				m_mmu.write32(ptr & ~3u, creg(i));
				ptr += 4;
			}
		}
	}

	if(instr.load_from_memory() && instr.is_register_in_list(instr.base_reg()))
		ptr = base;
	if(!instr.load_from_memory() && instr.is_register_first_in_rlist(instr.base_reg()))
		ptr = base;

	reg(instr.base_reg()) = ptr;    //  FIXME:  Potential fuckery with constness and R15

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT16(THUMB::InstructionFormat16 instr) {
	if (!cspr().evaluate_condition(instr.condition())) return OpExecState::Finished;

	const auto offset = sign_extend<9>(static_cast<uint16>(instr.offset()) << 1u);
	const auto new_pc = pc() + offset;

	pc() = new_pc;

	return OpExecState::Finished;
}

#define ON_CYCLE(a) if(current_instruction().m_cycles == (a))

OpExecState ARM7TDMI::THUMB_FMT17(THUMB::InstructionFormat17 instr) {

	ON_CYCLE(1) {
		log("Exception SWI THUMB, return={:08x}, cpsr={:08x}", const_pc()-2, cspr().raw());

		m_excepts.push_back({.return_pc = const_pc()-2, .entry_mode = cspr().mode(), .saved_psr = cspr()});
		m_registers.m_gSVC[1] = const_pc();
		m_saved_status.m_SVC = cspr();
		cspr().set_mode(PRIV_MODE::SVC);
		cspr().set(CSPR_REGISTERS::IRQn, true);

		return OpExecState::Stall;
	}

	ON_CYCLE(2) {
		cspr().set_state(INSTR_MODE::ARM);
		pc() = 0x08;
		lr() -= 2;
//		log("sp is {:08x}", sp());

		return OpExecState::Finished;
	}

	ASSERT_NOT_REACHED();
}

OpExecState ARM7TDMI::THUMB_FMT18(THUMB::InstructionFormat18 instr) {
	auto new_pc = pc() + sign_extend<12>(instr.offset() << 1u);
//	std::cout << "ARM7TDMI/ Branch to " << std::hex << new_pc << std::dec << ", offset " << std::to_string(instr.offset()) << "\n";

	pc() = new_pc;
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_FMT19(THUMB::InstructionFormat19 instr) {
//	log("pc: {:08x} lr: {:08x}", pc(), lr());
	if(!instr.low()) {
		auto offset = sign_extend<23>(static_cast<uint32>(instr.offset()) << 12u);
		lr() = (const_pc() + offset);
	} else {
		const auto next_pc = pc() - 2;
		pc() = lr() + ((instr.offset() << 1u));
		lr() = next_pc | 0x1;
//		std::cout << "ARM7TDMI/ Branch long to " << std::hex << pc() << std::dec << "\n";
	}
	return OpExecState::Finished;
}





OpExecState ARM7TDMI::THUMB_AND(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_and(target, source, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_EOR(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_eor(target, source, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_TST(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_and(target, source, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_ORR(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_or(target, source, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_BIC(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_and(target, _alu_not(source, true), true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_MVN(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_not(source, true);
	return OpExecState::Finished;
}




OpExecState ARM7TDMI::THUMB_LSL(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

//	target = _shift_lsl(target, shift_count, shift_count != 0);
	if(shift_count != 0)
		target = _shift_lsl(target, shift_count);
	_alu_set_flags_logical_op(target);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_LSR(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

//	target = (shift_count != 0) ? _shift_lsr(target, shift_count, true)
//								: _shift_lsl(target, shift_count, false);
	if(shift_count != 0)
		target = _shift_lsr(target, shift_count);
	_alu_set_flags_logical_op(target);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_ASR(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

//	target = _shift_asr(target, shift_count, shift_count != 0);
	if(shift_count != 0)
		target = _shift_asr(target, shift_count);
	_alu_set_flags_logical_op(target);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_ROR(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

	if(shift_count != 0)
		target = _shift_ror(target, shift_count);
	_alu_set_flags_logical_op(target);
	return OpExecState::Finished;
}



OpExecState ARM7TDMI::THUMB_ADC(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_adc(target, source, true);

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_SBC(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_sbc(target, source, true);

	return OpExecState::Finished;
}



OpExecState ARM7TDMI::THUMB_NEG(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_sub(0, source, true);

	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_CMP(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_sub(target, source, true);
	return OpExecState::Finished;
}

OpExecState ARM7TDMI::THUMB_CMN(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_add(target, source, true);
	return OpExecState::Finished;
}


OpExecState ARM7TDMI::THUMB_MUL(THUMB::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	uint32 result = target * source;
	_alu_set_flags_logical_op(result);  //  FIXME

	target = result;
	return OpExecState::Finished;
}
