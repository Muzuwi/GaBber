#include "Bus/Common/BusInterface.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Emulator/Bits.hpp"

namespace thumb = disarmv4t::thumb::instr;

void ARM7TDMI::THUMB_ALU(thumb::InstructionFormat4 instr) {
	/*
	 *  Lookup table for THUMB format 4 instruction (ALU operations)
	 */
	typedef void (::ARM7TDMI::*ThumbAluOperation)(::thumb::InstructionFormat4);
	static const ThumbAluOperation s_thumb_alu_lookup[16] {
		&ARM7TDMI::THUMB_AND, &ARM7TDMI::THUMB_EOR, &ARM7TDMI::THUMB_LSL, &ARM7TDMI::THUMB_LSR,
		&ARM7TDMI::THUMB_ASR, &ARM7TDMI::THUMB_ADC, &ARM7TDMI::THUMB_SBC, &ARM7TDMI::THUMB_ROR,
		&ARM7TDMI::THUMB_TST, &ARM7TDMI::THUMB_NEG, &ARM7TDMI::THUMB_CMP, &ARM7TDMI::THUMB_CMN,
		&ARM7TDMI::THUMB_ORR, &ARM7TDMI::THUMB_MUL, &ARM7TDMI::THUMB_BIC, &ARM7TDMI::THUMB_MVN,
	};

	m_wait_cycles += mem_waits_access16(const_pc() + 4, AccessType::Seq);
	auto func = s_thumb_alu_lookup[instr.opcode()];
	(*this.*func)(instr);
}

void ARM7TDMI::THUMB_FMT1(thumb::InstructionFormat1 instr) {
	const auto& source = creg(instr.source_reg());
	auto& destination = reg(instr.destination_reg());
	const auto offset = instr.immediate();
	const auto opcode = instr.opcode();

	switch(opcode) {
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
			log("ARM7TDMI/ Invalid opcode {:04x} for THUMB mov shifted at pc {:08x}, instr {:04x}", opcode, pc() - 4,
			    instr.raw());
			break;
		}
	}

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT2(thumb::InstructionFormat2 instr) {
	const auto& source = creg(instr.source_reg());
	auto& destination = reg(instr.destination_reg());

	uint32 operand2 = (instr.immediate_is_value()) ? (instr.immediate()) : (creg(instr.immediate()));

	const uint32 result = (instr.subtract()) ? _alu_sub(source, operand2, true) : _alu_add(source, operand2, true);

	destination = result;

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT3(thumb::InstructionFormat3 instr) {
	switch(instr.opcode()) {
		case 0: {//  MOV
			auto& target_reg = reg(instr.target_reg());
			target_reg = instr.immediate();
			_alu_set_flags_logical_op(instr.immediate());
			break;
		}
		case 1: {//  CMP
			const auto& target_reg = creg(instr.target_reg());
			(void)_alu_sub(target_reg, instr.immediate(), true);
			break;
		}
		case 2: {//  ADD
			auto& target_reg = reg(instr.target_reg());
			target_reg = _alu_add(target_reg, instr.immediate(), true);
			break;
		}
		case 3: {//  SUB
			auto& target_reg = reg(instr.target_reg());
			target_reg = _alu_sub(target_reg, instr.immediate(), true);
			break;
		}
		default: ASSERT_NOT_REACHED();
	}

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT5(thumb::InstructionFormat5 instr) {
	const auto& source = creg(instr.source_reg());

	switch(instr.opcode()) {
		case 0: {
			m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

			auto& destination = reg(instr.destination_reg());
			destination = source + destination;

			if(instr.destination_reg() == 15) {
				m_wait_cycles += mem_waits_access16(const_pc(), AccessType::NonSeq) +
				                 mem_waits_access16(const_pc(), AccessType::Seq);
			}
			break;
		}
		case 1: {
			const auto& destination = creg(instr.destination_reg());
			(void)_alu_sub(destination, source, true);

			m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
			break;
		}
		case 2: {
			m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

			auto& destination = reg(instr.destination_reg());
			destination = source;

			if(instr.destination_reg() == 15) {
				m_wait_cycles += mem_waits_access16(const_pc(), AccessType::NonSeq) +
				                 mem_waits_access16(const_pc(), AccessType::Seq);
			}
			break;
		}
		case 3: {
			if(!instr.MSBd()) {
				m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

				cspr().set_state((source & 1) ? INSTR_MODE::THUMB : INSTR_MODE::ARM);
				if(!(source & 1)) {
					pc() = source & ~2u;
				} else {
					pc() = source & ~1u;
				}

				if(cspr().state() == INSTR_MODE::ARM) {
					m_wait_cycles += mem_waits_access32(const_pc() + 0, AccessType::NonSeq) +
					                 mem_waits_access32(const_pc() + 4, AccessType::Seq);
				} else {
					m_wait_cycles += mem_waits_access16(const_pc() + 0, AccessType::NonSeq) +
					                 mem_waits_access16(const_pc() + 2, AccessType::Seq);
				}
			}
			break;
		}
		default: ASSERT_NOT_REACHED();
	}
}

void ARM7TDMI::THUMB_FMT6(thumb::InstructionFormat6 instr) {
	auto& destination = reg(instr.destination_reg());
	const auto immediate_shifted = static_cast<uint16>(instr.immediate()) << 2u;
	const auto aligned_pc = const_pc() & ~0x3;
	destination = static_cast<uint32>(mem_read32(aligned_pc + immediate_shifted));

	m_wait_cycles += 1 /*I*/ + mem_waits_access16(const_pc(), AccessType::Seq) +
	                 mem_waits_access32(aligned_pc + immediate_shifted, AccessType::NonSeq);
}

void ARM7TDMI::THUMB_FMT7(thumb::InstructionFormat7 instr) {
	const auto& base = creg(instr.base_reg());
	const auto& offset = creg(instr.offset_reg());

	auto address = base + offset;
	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		if(instr.quantity_in_bytes()) {
			target = mem_read8(address);
			m_wait_cycles += mem_waits_access8(address, AccessType::NonSeq);
		} else {
			auto word = mem_read32(address & ~3u);//  Force align
			//  Rotate
			if(address & 3u) {
				word = Bits::rotr32(word, (address & 3u) * 8);
			}
			target = word;

			m_wait_cycles += mem_waits_access32(address & ~3u, AccessType::NonSeq);
		}
		m_wait_cycles += 1 /*I*/ + mem_waits_access16(const_pc(), AccessType::Seq);
	} else {
		const auto target = creg(instr.target_reg());

		if(instr.quantity_in_bytes()) {
			mem_write8(address, target & 0xFFu);
			m_wait_cycles += mem_waits_access8(address, AccessType::NonSeq);
		} else {
			mem_write32(address, target);
			m_wait_cycles += mem_waits_access32(address, AccessType::NonSeq);
		}
		m_wait_cycles += mem_waits_access16(const_pc(), AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT8(thumb::InstructionFormat8 instr) {
	const auto& base = creg(instr.base_reg());
	const auto& offset = creg(instr.offset_reg());
	const auto address = base + offset;

	switch(instr.opcode()) {
		case 0: {
			const auto destination = creg(instr.destination_reg());
			mem_write16(address & ~1u, destination & 0xffff);

			m_wait_cycles += mem_waits_access16(address & ~1u, AccessType::NonSeq) +
			                 mem_waits_access16(const_pc(), AccessType::NonSeq);
			break;
		}
		case 1: {
			auto& destination = reg(instr.destination_reg());

			uint8 val = mem_read8(address);
			destination = Bits::sign_extend<8>(val);

			m_wait_cycles += 1 /*I*/ + mem_waits_access8(address, AccessType::NonSeq) +
			                 mem_waits_access16(const_pc(), AccessType::Seq);
			break;
		}
		case 2: {
			auto& destination = reg(instr.destination_reg());

			uint32 word;
			word = static_cast<uint32>(mem_read16(address & ~1u));
			if(address & 1u) {
				log("Undefined behaviour: LDRH with unaligned address!");
				word = Bits::rotr32(word, 8);
			}

			destination = word;

			m_wait_cycles += 1 /*I*/ + mem_waits_access16(address & ~1u, AccessType::NonSeq) +
			                 mem_waits_access16(const_pc(), AccessType::Seq);
			break;
		}
		case 3: {
			auto& destination = reg(instr.destination_reg());

			uint32 word;
			if(address & 1u) {
				log("Undefined behaviour: LDRSH with unaligned address!");
				auto byte = mem_read8(address);
				word = Bits::sign_extend<8>(byte);
			} else {
				auto hword = mem_read16(address);
				word = Bits::sign_extend<16>(hword);
			}

			destination = word;

			m_wait_cycles += 1 /*I*/ + mem_waits_access16(address, AccessType::NonSeq) +
			                 mem_waits_access16(const_pc(), AccessType::Seq);
			break;
		}

		default: ASSERT_NOT_REACHED();
	}
}

void ARM7TDMI::THUMB_FMT9(thumb::InstructionFormat9 instr) {
	const auto& base = creg(instr.base_reg());

	auto offset = static_cast<uint16>(instr.offset());
	if(!instr.quantity_in_bytes())
		offset <<= 2u;
	uint32 address = base + offset;

	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		if(instr.quantity_in_bytes()) {
			target = mem_read8(address);
			m_wait_cycles += mem_waits_access8(address, AccessType::NonSeq);
		} else {
			auto word = mem_read32(address & ~3u);//  Force align
			//  Rotate
			if(address & 3u) {
				word = Bits::rotr32(word, (address & 3u) * 8);
			}

			target = word;
			m_wait_cycles += mem_waits_access32(address & ~3u, AccessType::NonSeq);
		}

		m_wait_cycles += 1 /*I*/ + mem_waits_access16(const_pc(), AccessType::Seq);
	} else {
		const auto target = creg(instr.target_reg());

		if(instr.quantity_in_bytes()) {
			mem_write8(address, target & 0xFFu);
			m_wait_cycles += mem_waits_access8(address, AccessType::NonSeq);
		} else {
			mem_write32(address, target);
			m_wait_cycles += mem_waits_access32(address, AccessType::NonSeq);
		}
		m_wait_cycles += mem_waits_access16(const_pc(), AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT10(thumb::InstructionFormat10 instr) {
	const auto& base = creg(instr.base_reg());
	const auto offset = instr.offset() << 1u;
	uint32 address = base + offset;

	if(instr.load_from_memory()) {
		auto& target = reg(instr.target_reg());

		uint32 word;
		word = static_cast<uint32>(mem_read16(address & ~1u));
		if(address & 1u) {
			log("Undefined behaviour: LDRH with unaligned address!");
			word = Bits::rotr32(word, 8);
		}

		target = word;

		m_wait_cycles += 1 /*I*/ + mem_waits_access16(address & ~1u, AccessType::NonSeq) +
		                 mem_waits_access16(const_pc(), AccessType::Seq);
	} else {
		const auto target = creg(instr.target_reg());

		mem_write16(address & ~1u, target & 0xFFFF);

		m_wait_cycles += mem_waits_access16(address & ~1u, AccessType::NonSeq) +
		                 mem_waits_access16(const_pc(), AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT11(thumb::InstructionFormat11 instr) {
	const auto address = creg(13) + (static_cast<uint16>(instr.immediate()) << 2u);

	if(instr.load_from_memory()) {
		auto& destination = reg(instr.destination_reg());

		auto word = mem_read32(address & ~3u);//  Force align

		//  Rotate
		if(address & 3u) {
			word = Bits::rotr32(word, (address & 3u) * 8);
		}

		destination = word;

		m_wait_cycles += 1 /*I*/ + mem_waits_access32(address & ~3u, AccessType::NonSeq) +
		                 mem_waits_access16(const_pc(), AccessType::Seq);
	} else {
		const auto destination = creg(instr.destination_reg());

		mem_write32(address & ~3u, destination);

		m_wait_cycles += mem_waits_access32(address & ~3u, AccessType::NonSeq) +
		                 mem_waits_access16(const_pc(), AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT12(thumb::InstructionFormat12 instr) {
	auto& destination = reg(instr.destination_reg());

	const auto offset = static_cast<uint16>(instr.immediate()) << 2u;
	auto address = (instr.source_is_sp()) ? creg(13) + offset : (const_pc() & ~0b10u) + offset;

	destination = address;

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT13(thumb::InstructionFormat13 instr) {
	const auto offset = static_cast<uint16>(instr.offset()) << 2u;

	sp() = (instr.offset_is_negative()) ? sp() - offset : sp() + offset;

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT14(thumb::InstructionFormat14 instr) {
	unsigned n = 0;

	//  POP {Rlist}
	if(instr.load_from_memory()) {
		for(int8 i = 0; i < 8; ++i) {
			if(instr.is_register_in_list(i)) {
				reg(i) = stack_pop32();
				++n;
			}
		}
		if(instr.store_lr_load_pc()) {
			pc() = stack_pop32() & ~0x1;
		}

		//  FIXME: Weird timing edge cases
		m_wait_cycles += 1 /*I*/ + mem_waits_access32(sp() + 0, AccessType::NonSeq) +
		                 n * mem_waits_access32(sp() + 0, AccessType::Seq);
		if(instr.store_lr_load_pc()) {
			m_wait_cycles += mem_waits_access32(const_pc(), AccessType::NonSeq) +
			                 mem_waits_access32(const_pc() + 2, AccessType::Seq);
		}
	}
	//  PUSH {Rlist}
	else {
		if(instr.store_lr_load_pc()) {
			stack_push32(lr());
		}
		for(int8 i = 8; i >= 0; --i) {
			if(instr.is_register_in_list(i)) {
				stack_push32(creg(i));
				++n;
			}
		}

		//  FIXME: Weird timing edge cases
		const unsigned clamped_n = n > 0 ? n - 1 : 0;//  (n-1)
		m_wait_cycles += mem_waits_access32(sp(), AccessType::NonSeq) +
		                 clamped_n * mem_waits_access32(sp(), AccessType::Seq) +
		                 mem_waits_access32(const_pc() + 8, AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT15(thumb::InstructionFormat15 instr) {
	if(instr.is_rlist_empty()) {
		auto& base = reg(instr.base_reg());

		if(instr.load_from_memory()) {
			pc() = mem_read32(base & ~3u);
		} else {
			mem_write32(base & ~3u, const_pc() + 2);
		}

		base += 0x40;

		//  FIXME: Emulate cycles
		m_wait_cycles += 1;
		return;
	}

	const auto& base = creg(instr.base_reg());
	auto ptr = base;

	unsigned n = 0;
	for(uint8 reg = 0; reg < 8; ++reg) {
		if(!instr.is_register_in_list(reg))
			continue;
		++n;

		if(instr.load_from_memory()) {
			this->reg(reg) = mem_read32(ptr & ~3u);
		} else {
			uint32 word = creg(reg);
			//  Writeback quirk with base in rlist - final base value is written instead
			if(reg == instr.base_reg() && !instr.is_register_first_in_rlist(instr.base_reg())) {
				word = base + instr.total_offset();
			}
			mem_write32(ptr & ~3u, word);
		}

		ptr += 4;
	}

	//  LDM writeback is overwritten by the memory load
	if(instr.load_from_memory() && instr.is_register_in_list(instr.base_reg()))
		ptr = base;

	reg(instr.base_reg()) = ptr;//  FIXME:  Potential weirdness with constness and R15

	if(instr.load_from_memory()) {
		//  FIXME: Weird timing edge cases
		m_wait_cycles += 1 /*I*/ + mem_waits_access32(ptr + 0, AccessType::NonSeq) +
		                 n * mem_waits_access32(ptr + 0, AccessType::Seq);
	} else {
		//  FIXME: Weird timing edge cases
		m_wait_cycles += mem_waits_access32(ptr, AccessType::NonSeq) +
		                 (n - 1) * mem_waits_access32(ptr, AccessType::Seq) +
		                 mem_waits_access32(const_pc() + 8, AccessType::NonSeq);
	}
}

void ARM7TDMI::THUMB_FMT16(thumb::InstructionFormat16 instr) {
	if(!cspr().evaluate_condition(instr.condition())) {
		m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
		return;
	}

	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

	const auto offset = Bits::sign_extend<9>(static_cast<uint16>(instr.offset()) << 1u);
	const auto new_pc = pc() + offset;

	pc() = new_pc;

	m_wait_cycles += mem_waits_access16(const_pc() + 0, AccessType::NonSeq) +
	                 mem_waits_access16(const_pc() + 2, AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT17(thumb::InstructionFormat17) {
	enter_swi();
}

void ARM7TDMI::THUMB_FMT18(thumb::InstructionFormat18 instr) {
	m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

	auto new_pc = pc() + Bits::sign_extend<12>(instr.offset() << 1u);
	pc() = new_pc;

	m_wait_cycles += mem_waits_access16(const_pc() + 0, AccessType::NonSeq) +
	                 mem_waits_access16(const_pc() + 2, AccessType::Seq);
}

void ARM7TDMI::THUMB_FMT19(thumb::InstructionFormat19 instr) {
	if(!instr.low()) {
		auto offset = Bits::sign_extend<23>(static_cast<uint32>(instr.offset()) << 12u);
		lr() = (const_pc() + offset);

		m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);
	} else {
		m_wait_cycles += mem_waits_access16(const_pc(), AccessType::Seq);

		const auto next_pc = pc() - 2;
		pc() = lr() + ((instr.offset() << 1u));
		lr() = next_pc | 0x1;

		m_wait_cycles += mem_waits_access16(const_pc() + 0, AccessType::NonSeq) +
		                 mem_waits_access16(const_pc() + 2, AccessType::Seq);
	}
}

void ARM7TDMI::THUMB_AND(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_and(target, source, true);
}

void ARM7TDMI::THUMB_EOR(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_eor(target, source, true);
}

void ARM7TDMI::THUMB_TST(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_and(target, source, true);
}

void ARM7TDMI::THUMB_ORR(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_or(target, source, true);
}

void ARM7TDMI::THUMB_BIC(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_and(target, _alu_not(source, true), true);
}

void ARM7TDMI::THUMB_MVN(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_not(source, true);
}

void ARM7TDMI::THUMB_LSL(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

	if(shift_count != 0)
		target = _shift_lsl(target, shift_count);
	_alu_set_flags_logical_op(target);

	m_wait_cycles += 1 /*I*/;
}

void ARM7TDMI::THUMB_LSR(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

	if(shift_count != 0)
		target = _shift_lsr(target, shift_count);
	_alu_set_flags_logical_op(target);

	m_wait_cycles += 1 /*I*/;
}

void ARM7TDMI::THUMB_ASR(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

	if(shift_count != 0)
		target = _shift_asr(target, shift_count);
	_alu_set_flags_logical_op(target);

	m_wait_cycles += 1 /*I*/;
}

void ARM7TDMI::THUMB_ROR(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());
	const auto shift_count = source & 0x0ffu;

	if(shift_count != 0)
		target = _shift_ror(target, shift_count);
	_alu_set_flags_logical_op(target);

	m_wait_cycles += 1 /*I*/;
}

void ARM7TDMI::THUMB_ADC(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_adc(target, source, true);
}

void ARM7TDMI::THUMB_SBC(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_sbc(target, source, true);
}

void ARM7TDMI::THUMB_NEG(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	target = _alu_sub(0, source, true);
}

void ARM7TDMI::THUMB_CMP(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_sub(target, source, true);
}

void ARM7TDMI::THUMB_CMN(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	const auto& target = creg(instr.target_reg());

	(void)_alu_add(target, source, true);
}

void ARM7TDMI::THUMB_MUL(thumb::InstructionFormat4 instr) {
	const auto& source = creg(instr.source_reg());
	auto& target = reg(instr.target_reg());

	uint32 result = target * source;
	_alu_set_flags_logical_op(result);//  FIXME

	m_wait_cycles += mult_m_cycles(target) /*I*/;
	target = result;
}
