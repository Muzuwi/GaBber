#include "Headers/ARM7TDMI.hpp"
#include "Headers/ARM_Instruction.hpp"
#include "MMU/MMU.hpp"

void ARM7TDMI::reset() {
	fmt::print("ARM7TDMI/ Reset");
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::SVC);
	cspr().set_register(CSPR_REGISTERS::IRQn, true);
	cspr().set_register(CSPR_REGISTERS::FIQn, true);
	cspr().set_register(CSPR_REGISTERS::State, false);

	m_soundbias.raw() = 0x0200;
	m_waitcnt.raw() = 0;
	m_POSTFLG.raw() = 0;
	m_IE.raw() = 0;
	m_IF.raw() = 0;

	for (unsigned i = 0; i < 16; ++i) {
		m_registers.m_base[i] = 0;
		if (i < 7) m_registers.m_gFIQ[i] = 0;
		if (i < 2) {
			m_registers.m_gSVC[i] = 0;
			m_registers.m_gABT[i] = 0;
			m_registers.m_gIRQ[i] = 0;
			m_registers.m_gUND[i] = 0;
		}
	}

//	m_registers.m_gSVC[0] = 0x03007fe0; //  SP_svc
//	m_registers.m_gIRQ[0] = 0x03007fa0; //  SP_irq
//	m_registers.m_base[13] = 0x03007f00; //  SP_usr

//	cspr().set_mode(PRIV_MODE::SYS);
//	cspr().set_state(INSTR_MODE::ARM);
//	m_registers.m_base[15] = 0x0;
//	lr() = 0x08000000;
}


bool ARM7TDMI::handle_dma_transfers() {
	m_dma0.cycle();
	if(m_dma0.running())
		return true;

	m_dma1.cycle();
	if(m_dma1.running())
		return true;

	m_dma2.cycle();
	if(m_dma2.running())
		return true;

	m_dma3.cycle();
	if(m_dma3.running())
		return true;

	return false;
}


bool ARM7TDMI::handle_halt() {
	if (!m_HALTCNT.m_halt && !m_HALTCNT.m_stop) return false;

	if (m_HALTCNT.m_halt) {
		if ((m_IE.raw() & m_IF.raw()) != 0) {
//			log("Exit HALT, IE={:04x}, IF={:04x}, pc_raw={:08x}", m_IE.raw(), m_IF.raw(), const_pc());
			m_HALTCNT.m_halt = false;
			return false;
		} else {
			return true;
		}
	}

	ASSERT_NOT_REACHED();
}


void ARM7TDMI::cycle() {
	m_cycles++;
	m_timers.cycle();

	if (handle_halt())
		return;
	if (handle_exceptions())
		return;
	if (handle_dma_transfers())
		return;

	if(!is_pipeline_stalled())
		_pipeline_fetch();

	if(m_prefetched.size() != 3) {
		_pc_increment();
		return;
	}

	auto old_sp = cr13();

	if(!is_pipeline_stalled() && current_instruction().m_pc != const_pc() - 2*current_instr_len()) {
		log("Pipeline misaligned! op_pc={:08x}, pc={:08x}", current_instruction().m_pc, const_pc() - 2*current_instr_len());
		assert(false);
	}

	exec_opcode();

//	if(m_sp_dirty) {
//		if(old_sp ^ cr13()) {
//			static int stack_depth = 0;
//			auto diff = (int)cr13() - (int)old_sp;
//			stack_depth += diff;
//			log("Modified sp: {:08x} -> {:08x} ({:+}, depth={:+})", old_sp, cr13(), diff, stack_depth);
//		}
//		m_sp_dirty = false;
//	}

	if (m_pc_dirty && !is_pipeline_stalled()) {
//		log("Processing flush after {} instruction cycles", m_cycles_since_pc_change);

		if (const_pc() & 1u) {
			m_registers.m_base[15] &= ~1u;
		}

		_pipeline_flush_and_fetch();

		m_pc_dirty = false;
		m_cycles_since_pc_change = 0;
	}

	if(!is_pipeline_stalled())
		_pc_increment();
}


void ARM7TDMI::exec_opcode() {
	auto& prefetch = current_instruction();

	OpExecState state;
	if (cspr().state() == INSTR_MODE::ARM)
		state = execute_ARM(prefetch.m_opcode);
	else
		state = execute_THUMB(prefetch.m_opcode);

	if (state == OpExecState::Finished) {
		m_prefetched.pop_front();
		m_is_pipeline_stalled = false;
	} else {
		if(m_pc_dirty)
			m_cycles_since_pc_change++;
		prefetch.m_cycles++;
		m_is_pipeline_stalled = true;
	}
}


void ARM7TDMI::_pipeline_fetch() {
	const auto op = (cspr().state() == INSTR_MODE::ARM) ? m_mmu.read32(const_pc())
	                                                    : m_mmu.read16(const_pc());
	m_prefetched.push_back({.m_opcode=op, .m_cycles=1, .m_pc=const_pc()});
}


void ARM7TDMI::_pipeline_flush_and_fetch() {
	assert(!m_is_pipeline_stalled);
	m_prefetched.clear();
	_pipeline_fetch();
}


OpExecState ARM7TDMI::execute_ARM(uint32 opcode) {
	if (!cspr().evaluate_condition(ARM::Instruction(opcode).condition())) {
		return OpExecState::Finished;
	}

	auto op = ARM::opcode_decode(opcode);
	switch (op) {
		case ARM::InstructionType::BBL: return this->B(ARM::BInstruction(opcode));
		case ARM::InstructionType::BX:  return this->BX(ARM::BXInstruction(opcode));
		case ARM::InstructionType::ALU: return this->DPI(ARM::DataProcessInstruction(opcode));
		case ARM::InstructionType::MUL: return this->MUL(ARM::MultInstruction(opcode));
		case ARM::InstructionType::MLL: return this->MLL(ARM::MultLongInstruction(opcode));
		case ARM::InstructionType::SDT: return this->SDT(ARM::SDTInstruction(opcode));
		case ARM::InstructionType::HDT: return this->HDT(ARM::HDTInstruction(opcode));
		case ARM::InstructionType::BDT: return this->BDT(ARM::BDTInstruction(opcode));
		case ARM::InstructionType::SWP: return this->SWP(ARM::SWPInstruction(opcode));
		case ARM::InstructionType::SWI: return this->SWI(ARM::SWIInstruction(opcode));

		case ARM::InstructionType::UD:
		default: {
			log("Invalid ARM opcode={:08x}", opcode);
			ASSERT_NOT_REACHED();

			return OpExecState::Finished;
		}
	}
}


OpExecState ARM7TDMI::execute_THUMB(uint16 opcode) {
	auto op = THUMB::opcode_decode(opcode);
	switch(op) {
		case THUMB::InstructionType::FMT1:  return THUMB_FMT1(THUMB::InstructionFormat1(opcode));
		case THUMB::InstructionType::FMT2:  return THUMB_FMT2(THUMB::InstructionFormat2(opcode));
		case THUMB::InstructionType::FMT3:  return THUMB_FMT3(THUMB::InstructionFormat3(opcode));
		case THUMB::InstructionType::FMT4:  return THUMB_ALU(THUMB::InstructionFormat4(opcode));
		case THUMB::InstructionType::FMT5:  return THUMB_FMT5(THUMB::InstructionFormat5(opcode));
		case THUMB::InstructionType::FMT6:  return THUMB_FMT6(THUMB::InstructionFormat6(opcode));
		case THUMB::InstructionType::FMT7:  return THUMB_FMT7(THUMB::InstructionFormat7(opcode));
		case THUMB::InstructionType::FMT8:  return THUMB_FMT8(THUMB::InstructionFormat8(opcode));
		case THUMB::InstructionType::FMT9:  return THUMB_FMT9(THUMB::InstructionFormat9(opcode));
		case THUMB::InstructionType::FMT10: return THUMB_FMT10(THUMB::InstructionFormat10(opcode));
		case THUMB::InstructionType::FMT11: return THUMB_FMT11(THUMB::InstructionFormat11(opcode));
		case THUMB::InstructionType::FMT12: return THUMB_FMT12(THUMB::InstructionFormat12(opcode));
		case THUMB::InstructionType::FMT13: return THUMB_FMT13(THUMB::InstructionFormat13(opcode));
		case THUMB::InstructionType::FMT14: return THUMB_FMT14(THUMB::InstructionFormat14(opcode));
		case THUMB::InstructionType::FMT15: return THUMB_FMT15(THUMB::InstructionFormat15(opcode));
		case THUMB::InstructionType::FMT16: return THUMB_FMT16(THUMB::InstructionFormat16(opcode));
		case THUMB::InstructionType::FMT17: return THUMB_FMT17(THUMB::InstructionFormat17(opcode));
		case THUMB::InstructionType::FMT18: return THUMB_FMT18(THUMB::InstructionFormat18(opcode));
		case THUMB::InstructionType::FMT19: return THUMB_FMT19(THUMB::InstructionFormat19(opcode));

		case THUMB::InstructionType::UD:
		default: {
			log("Invalid THUMB opcode={:04x}", opcode);
			ASSERT_NOT_REACHED();

			return OpExecState::Finished;
		}
	}
}


void ARM7TDMI::stack_push32(uint32 val) {
	sp() -= 4;
	m_mmu.write32(sp() & ~3u, val);
}


uint32 ARM7TDMI::stack_pop32() {
	auto val = m_mmu.read32(sp() & ~3u);
	sp() += 4;
	return val;
}


void ARM7TDMI::raise_irq(IRQType type) {
	auto irq_num = static_cast<unsigned>(type);
	if (irq_num > 13) {
		log("Invalid IRQ number {}", irq_num);
		return;
	}

	m_IF.raw() |= (1u << irq_num);
}


void ARM7TDMI::handle_interrupts() {
	if (!irqs_enabled_globally())
		return;

	//  Find first (?) available IRQ
	for (unsigned i = 0; i < 14; ++i) {
		const auto type = static_cast<IRQType>(i);
		if (is_irq_enabled(type) && is_irq_requested(type)) {
			// log("IRQ {} is raised\n", static_cast<IRQType>(i));
			raise_exception(ExceptionVector::IRQ);
		}
	}
}


bool ARM7TDMI::handle_exceptions() {
	handle_interrupts();

	if (m_exception_lines == 0 || is_pipeline_stalled()) return false;

	for (unsigned i = 0; i < 8; ++i) {
		if (m_exception_lines & (1u << i)) {
			const auto vector = static_cast<ExceptionVector>(i);
			switch (vector) {
				case ExceptionVector::Reset: {
					cspr().set_mode(PRIV_MODE::SVC);
					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::DataAbort: {
					cspr().set_mode(PRIV_MODE::ABT);

					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::FIQ: {
					cspr().set_mode(PRIV_MODE::FIQ);

					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::IRQ: {
					const auto return_pc = const_pc();
//					log("Exception enter IRQ, return_pc={:08x}, cspr={:08x}", return_pc-current_instr_len(), cspr().raw());
//					if(current_mode_has_spsr())
//						log("current spsr={:08x}", spsr().raw());
					m_excepts.push_back({.return_pc = static_cast<uint32>(return_pc - current_instr_len()), .entry_mode = cspr().mode(), .saved_psr = cspr()});

					m_registers.m_gIRQ[1] = return_pc - current_instr_len();
					m_saved_status.m_IRQ = cspr();
					cspr().set_mode(PRIV_MODE::IRQ);
					cspr().set_state(INSTR_MODE::ARM);
					cspr().set(CSPR_REGISTERS::IRQn, true);
					pc() = 0x18;
//					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::PrefetchAbort: {
					cspr().set_mode(PRIV_MODE::ABT);

					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::UndefinedInstr: {
					cspr().set_mode(PRIV_MODE::UND);

					ASSERT_NOT_REACHED();
					break;
				}
				case ExceptionVector::SWI: {
					m_excepts.push_back({.return_pc = static_cast<uint32>(const_pc() - current_instr_len()), .entry_mode = cspr().mode(), .saved_psr = cspr()});

					m_registers.m_gSVC[1] = const_pc() - current_instr_len();
					m_saved_status.m_SVC = cspr();
					cspr().set_state(INSTR_MODE::ARM);
					cspr().set_mode(PRIV_MODE::SVC);
					cspr().set(CSPR_REGISTERS::IRQn, true);
					pc() = 0x08;

					break;
				}
				case ExceptionVector::Reserved:
				default:
					ASSERT_NOT_REACHED();
			}

			_pipeline_flush_and_fetch();
			_pc_increment();
			m_exception_lines &= ~(1u << i);
			return false;
		}
	}

	return false;
}


void ARM7TDMI::raise_exception(ExceptionVector exception) {
	m_exception_lines |= (1u << static_cast<unsigned>(exception));
}
