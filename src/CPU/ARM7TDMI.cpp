#include <fstream>
#include "Headers/ARM7TDMI.hpp"
#include "Headers/ARM_Instruction.hpp"
#include "MMU/MMU.hpp"

void ARM7TDMI::reset() {
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::SVC);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	cspr().set(CSPR_REGISTERS::FIQn, true);
	cspr().set(CSPR_REGISTERS::State, false);

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

	pc() = 0x0 + 8;
	m_pc_dirty = false;
}

void ARM7TDMI::cycle() {
	m_cycles++;
	m_timers.cycle();

	if (dma_cycle_all())
		return;
	if (handle_halt())
		return;
	handle_interrupts();

	exec_opcode();
}


uint32 ARM7TDMI::fetch_instruction() {
	const auto op = (cspr().state() == INSTR_MODE::ARM) ? mem_read32(const_pc() - 2*current_instr_len())
	                                                    : mem_read16(const_pc() - 2*current_instr_len());
	return op;
}


void ARM7TDMI::exec_opcode() {
	const auto opcode = fetch_instruction();

	if (cspr().state() == INSTR_MODE::ARM)
		execute_ARM(opcode);
	else
		execute_THUMB(opcode);

	//  Always make sure the PC is 2 instructions ahead
	if(m_pc_dirty) {
		pc() += 2 * current_instr_len();
		pc() &= (cspr().state() == INSTR_MODE::ARM)  //  Force alignment for ALU opcodes modifying pc
				? ~3u
				: ~1u;
		m_pc_dirty = false;
	} else {
		pc_increment();
	}

	logDebug();
}


void ARM7TDMI::execute_ARM(uint32 opcode) {
	if (!cspr().evaluate_condition(ARM::Instruction(opcode).condition())) {
		return;
	}

	auto op = ARM::opcode_decode(opcode);
	switch (op) {
		case ARM::InstructionType::BBL: this->B(ARM::BInstruction(opcode)); return;
		case ARM::InstructionType::BX:  this->BX(ARM::BXInstruction(opcode)); return;
		case ARM::InstructionType::ALU: this->DPI(ARM::DataProcessInstruction(opcode)); return;
		case ARM::InstructionType::MUL: this->MUL(ARM::MultInstruction(opcode)); return;
		case ARM::InstructionType::MLL: this->MLL(ARM::MultLongInstruction(opcode)); return;
		case ARM::InstructionType::SDT: this->SDT(ARM::SDTInstruction(opcode)); return;
		case ARM::InstructionType::HDT: this->HDT(ARM::HDTInstruction(opcode)); return;
		case ARM::InstructionType::BDT: this->BDT(ARM::BDTInstruction(opcode)); return;
		case ARM::InstructionType::SWP: this->SWP(ARM::SWPInstruction(opcode)); return;
		case ARM::InstructionType::SWI: this->SWI(ARM::SWIInstruction(opcode)); return;

		case ARM::InstructionType::UD:
		default: {
			log("Invalid ARM opcode={:08x}", opcode);
			dump_memory_around_pc();
			ASSERT_NOT_REACHED();
		}
	}
}


void ARM7TDMI::execute_THUMB(uint16 opcode) {
	auto op = THUMB::opcode_decode(opcode);
	switch(op) {
		case THUMB::InstructionType::FMT1:  THUMB_FMT1(THUMB::InstructionFormat1(opcode)); return;
		case THUMB::InstructionType::FMT2:  THUMB_FMT2(THUMB::InstructionFormat2(opcode)); return;
		case THUMB::InstructionType::FMT3:  THUMB_FMT3(THUMB::InstructionFormat3(opcode)); return;
		case THUMB::InstructionType::FMT4:  THUMB_ALU(THUMB::InstructionFormat4(opcode)); return;
		case THUMB::InstructionType::FMT5:  THUMB_FMT5(THUMB::InstructionFormat5(opcode)); return;
		case THUMB::InstructionType::FMT6:  THUMB_FMT6(THUMB::InstructionFormat6(opcode)); return;
		case THUMB::InstructionType::FMT7:  THUMB_FMT7(THUMB::InstructionFormat7(opcode)); return;
		case THUMB::InstructionType::FMT8:  THUMB_FMT8(THUMB::InstructionFormat8(opcode)); return;
		case THUMB::InstructionType::FMT9:  THUMB_FMT9(THUMB::InstructionFormat9(opcode)); return;
		case THUMB::InstructionType::FMT10: THUMB_FMT10(THUMB::InstructionFormat10(opcode)); return;
		case THUMB::InstructionType::FMT11: THUMB_FMT11(THUMB::InstructionFormat11(opcode)); return;
		case THUMB::InstructionType::FMT12: THUMB_FMT12(THUMB::InstructionFormat12(opcode)); return;
		case THUMB::InstructionType::FMT13: THUMB_FMT13(THUMB::InstructionFormat13(opcode)); return;
		case THUMB::InstructionType::FMT14: THUMB_FMT14(THUMB::InstructionFormat14(opcode)); return;
		case THUMB::InstructionType::FMT15: THUMB_FMT15(THUMB::InstructionFormat15(opcode)); return;
		case THUMB::InstructionType::FMT16: THUMB_FMT16(THUMB::InstructionFormat16(opcode)); return;
		case THUMB::InstructionType::FMT17: THUMB_FMT17(THUMB::InstructionFormat17(opcode)); return;
		case THUMB::InstructionType::FMT18: THUMB_FMT18(THUMB::InstructionFormat18(opcode)); return;
		case THUMB::InstructionType::FMT19: THUMB_FMT19(THUMB::InstructionFormat19(opcode)); return;

		case THUMB::InstructionType::UD:
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
	const uint32 pc = const_pc() - 2*current_instr_len();
	const uint32 prev = (pc - 32) & ~0xf;
	const uint32 next = (pc + 32) & ~0xf;
	const unsigned size = cspr().state() == INSTR_MODE::ARM ? 4 : 2;

	for(uint32 addr = prev; addr < next; addr++) {
		if(((addr%16) == 0)) {
			fmt::print("${:08x}: ", addr);
		}

		if(addr == pc)
			fmt::print("[");
		else
			fmt::print(" ");
		fmt::print("{:02x}", m_mmu.read8(addr));
		if(addr == (pc+size-1))
			fmt::print("]");
		else
			fmt::print(" ");

		if((addr%16) == 15)
			fmt::print("\n");
	}

	if(m_last_written.contains(pc)) {
		const WriteContext& c = m_last_written.at(pc);

		fmt::print("Address {:08x} was last written by pc={:08x} on cycle={}\n", pc, c.pc, c.cycle);
		fmt::print("Instruction: {:08x}, return_address={}\n", c.instruction, c.return_address);
	} else {
		fmt::print("Address {:08x} was not modified recently\n", pc);
	}

	fmt::print("Last mode change at cycle {}\n", m_last_mode_change.cycle);
	fmt::print("Caused by: {}, pc={:08x}\n", m_last_mode_change.reason, m_last_mode_change.pc);
	fmt::print("Changed from mode {} -> {}\n", m_last_mode_change.prev, m_last_mode_change.neu);

	m_mmu.debug();
}

void ARM7TDMI::logDebug() {
	if constexpr(kill_debug())
		return;

	if(!log_file) {
		log_file = new std::ofstream{"debug_gabber.log"};
	}
	if(!log_file->good()) return;

	if(!seen_rom) {
		if(const_pc() == 0x08000000+8)
			seen_rom = true;
		else
			return;
	}

	for(unsigned i = 0; i < 16; ++i) {
		const uint32 v = creg(i) - (i == 15 ? current_instr_len() : 0);
		*log_file << fmt::format("{:08x},", v);
	}
	*log_file << fmt::format("{:08x}\n", cspr().raw());
}
