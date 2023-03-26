#include "CPU/ARM7TDMI.hpp"
#include "Bus/Common/BusInterface.hpp"
#include "Debugger/Debugger.hpp"
#include "Emulator/GaBber.hpp"

ARM7TDMI::ARM7TDMI(GaBber& emu)
    : Module(emu) {}

void ARM7TDMI::reset() {
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::SVC);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	cspr().set(CSPR_REGISTERS::FIQn, true);
	cspr().set(CSPR_REGISTERS::State, false);

	m_saved_status.m_ABT.set_raw(0x10);
	m_saved_status.m_FIQ.set_raw(0x10);
	m_saved_status.m_IRQ.set_raw(0x10);
	m_saved_status.m_SVC.set_raw(0x10);
	m_saved_status.m_UND.set_raw(0x10);

	for(unsigned i = 0; i < 16; ++i) {
		m_registers.m_base[i] = 0;
		if(i < 7) {
			m_registers.m_gFIQ[i] = 0;
		}
		if(i < 2) {
			m_registers.m_gSVC[i] = 0;
			m_registers.m_gABT[i] = 0;
			m_registers.m_gIRQ[i] = 0;
			m_registers.m_gUND[i] = 0;
		}
	}

	pc() = 0x0 + 8;
	//	pc() = 0xFFFF0000 + 8;
	m_pc_dirty = false;
}

unsigned ARM7TDMI::run_next_instruction() {
	const unsigned n = run_to_next_state();
	timers_cycle_all(n);

	m_cycles += n;
	return n;
}

unsigned ARM7TDMI::run_to_next_state() {
	m_wait_cycles = 0;

	//  If in DMA, emulate the wait states used up by DMA
	if(dma_is_running<0>() || dma_is_running<1>() || dma_is_running<2>() || dma_is_running<3>()) {
		dma_run_all();
		return m_wait_cycles;
	}

	//  If in halt, emulate it cycle-by-cycle
	if(handle_halt()) {
		return 1;
	}

	handle_interrupts();
	exec_opcode();

	return m_wait_cycles;
}

uint32 ARM7TDMI::fetch_instruction() {
	const auto op = (cspr().state() == INSTR_MODE::ARM) ? mem_read_arm_opcode(const_pc() - 2 * current_instr_len())
	                                                    : mem_read_thumb_opcode(const_pc() - 2 * current_instr_len());
	return op;
}

void ARM7TDMI::exec_opcode() {
	const auto opcode = fetch_instruction();
	debugger().on_execute_opcode(const_pc() - 2 * current_instr_len());

	if(cspr().state() == INSTR_MODE::ARM)
		execute_ARM(opcode);
	else
		execute_THUMB(opcode);

	//  Always make sure the PC is 2 instructions ahead
	if(m_pc_dirty) {
		pc() += 2 * current_instr_len();
		pc() &= (cspr().state() == INSTR_MODE::ARM)//  Force alignment for ALU opcodes modifying pc
		                ? ~3u
		                : ~1u;
		m_pc_dirty = false;
	} else {
		pc() += current_instr_len();
		m_pc_dirty = false;
	}
}

void ARM7TDMI::execute_ARM(uint32 opcode) {
	if(!cspr().evaluate_condition(disarmv4t::arm::instr::Instruction(opcode).condition())) {
		//  Unevaluated instructions take one S-cycle
		m_wait_cycles += mem_waits_access32(const_pc(), AccessType::Seq);
		return;
	}

#define BADOP(op)                               \
	case op:                                    \
		log("Unimplemented opcode: " #op "\n"); \
		dump_memory_around_pc();                \
		m_wait_cycles += 1;                     \
		break

	auto op = disarmv4t::arm::decode(opcode);
	switch(op) {
		case disarmv4t::arm::InstructionType::BBL: {
			this->B(disarmv4t::arm::instr::BInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::BX: {
			this->BX(disarmv4t::arm::instr::BXInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::ALU: {
			this->DPI(disarmv4t::arm::instr::DataProcessInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::MUL: {
			this->MUL(disarmv4t::arm::instr::MultInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::MLL: {
			this->MLL(disarmv4t::arm::instr::MultLongInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::SDT: {
			this->SDT(disarmv4t::arm::instr::SDTInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::HDT: {
			this->HDT(disarmv4t::arm::instr::HDTInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::BDT: {
			this->BDT(disarmv4t::arm::instr::BDTInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::SWP: {
			this->SWP(disarmv4t::arm::instr::SWPInstruction(opcode));
			return;
		}
		case disarmv4t::arm::InstructionType::SWI: {
			this->SWI(disarmv4t::arm::instr::SWIInstruction(opcode));
			return;
		}// clang-format off
		BADOP(disarmv4t::arm::InstructionType::CODT);
		BADOP(disarmv4t::arm::InstructionType::CO9);
		BADOP(disarmv4t::arm::InstructionType::CODO);
		BADOP(disarmv4t::arm::InstructionType::CORT);
		BADOP(disarmv4t::arm::InstructionType::MLH);
		BADOP(disarmv4t::arm::InstructionType::QALU);
		BADOP(disarmv4t::arm::InstructionType::CLZ);
		BADOP(disarmv4t::arm::InstructionType::BKPT);
		// clang-format on
		case disarmv4t::arm::InstructionType::UD:
		default: {
			log("Invalid ARM opcode={:08x}", opcode);
			dump_memory_around_pc();
			ASSERT_NOT_REACHED();
		}
	}
}

void ARM7TDMI::execute_THUMB(uint16 opcode) {
	auto op = disarmv4t::thumb::decode(opcode);
	switch(op) {
		case disarmv4t::thumb::InstructionType::FMT1: {
			THUMB_FMT1(disarmv4t::thumb::instr::InstructionFormat1(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT2: {
			THUMB_FMT2(disarmv4t::thumb::instr::InstructionFormat2(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT3: {
			THUMB_FMT3(disarmv4t::thumb::instr::InstructionFormat3(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT4: {
			THUMB_ALU(disarmv4t::thumb::instr::InstructionFormat4(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT5: {
			THUMB_FMT5(disarmv4t::thumb::instr::InstructionFormat5(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT6: {
			THUMB_FMT6(disarmv4t::thumb::instr::InstructionFormat6(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT7: {
			THUMB_FMT7(disarmv4t::thumb::instr::InstructionFormat7(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT8: {
			THUMB_FMT8(disarmv4t::thumb::instr::InstructionFormat8(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT9: {
			THUMB_FMT9(disarmv4t::thumb::instr::InstructionFormat9(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT10: {
			THUMB_FMT10(disarmv4t::thumb::instr::InstructionFormat10(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT11: {
			THUMB_FMT11(disarmv4t::thumb::instr::InstructionFormat11(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT12: {
			THUMB_FMT12(disarmv4t::thumb::instr::InstructionFormat12(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT13: {
			THUMB_FMT13(disarmv4t::thumb::instr::InstructionFormat13(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT14: {
			THUMB_FMT14(disarmv4t::thumb::instr::InstructionFormat14(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT15: {
			THUMB_FMT15(disarmv4t::thumb::instr::InstructionFormat15(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT16: {
			THUMB_FMT16(disarmv4t::thumb::instr::InstructionFormat16(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT17: {
			THUMB_FMT17(disarmv4t::thumb::instr::InstructionFormat17(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT18: {
			THUMB_FMT18(disarmv4t::thumb::instr::InstructionFormat18(opcode));
			return;
		}
		case disarmv4t::thumb::InstructionType::FMT19: {
			THUMB_FMT19(disarmv4t::thumb::instr::InstructionFormat19(opcode));
			return;
		}// clang-format off
		BADOP(disarmv4t::thumb::InstructionType::UD9);
		BADOP(disarmv4t::thumb::InstructionType::BKPT);
		BADOP(disarmv4t::thumb::InstructionType::BLX9);
		// clang-format on
		case disarmv4t::thumb::InstructionType::UD:
		default: {
			log("Invalid THUMB opcode={:04x}", opcode);
			dump_memory_around_pc();
			ASSERT_NOT_REACHED();
		}
	}
}

void ARM7TDMI::stack_push32(uint32 val) {
	sp() -= 4;
	mem_write32(sp() & ~3u, val);
}

uint32 ARM7TDMI::stack_pop32() {
	auto val = mem_read32(sp() & ~3u);
	sp() += 4;
	return val;
}

void ARM7TDMI::dump_memory_around_pc() const {
	const uint32 pc = const_pc() - 2 * current_instr_len();
	const uint32 prev = (pc - 32) & ~0xf;
	const uint32 next = (pc + 32) & ~0xf;
	const unsigned size = cspr().state() == INSTR_MODE::ARM ? 4 : 2;

	for(uint32 addr = prev; addr < next; addr++) {
		if(((addr % 16) == 0)) {
			fmt::print("${:08x}: ", addr);
		}

		if(addr == pc)
			fmt::print("[");
		else
			fmt::print(" ");
		fmt::print("{:02x}", bus().read8(addr));
		if(addr == (pc + size - 1))
			fmt::print("]");
		else
			fmt::print(" ");

		if((addr % 16) == 15)
			fmt::print("\n");
	}

	bus().debug();
	m_emu.toggle_debug_mode();
}
