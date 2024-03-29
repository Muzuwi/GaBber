#pragma once
#include <disarmv4t/arm.hpp>
#include <disarmv4t/thumb.hpp>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include "Bus/IO/Timer.hpp"
#include "CPU/GPR.hpp"
#include "CPU/PSR.hpp"
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"

enum class ExceptionVector {
	Reset = 0,
	DataAbort = 1,
	FIQ = 2,
	IRQ = 3,
	PrefetchAbort = 4,
	SWI = 5,
	UndefinedInstr = 6,
	Reserved = 7,
};

enum class AccessType;

/*
 *  Implementation of the ARM7TDMI processor
 */
class ARM7TDMI : Module {
protected:
	friend class GPRs;
	friend class IORegisters;
	friend class TestHarness;
	friend class Stacktrace;

	CSPR m_status;
	SPSR m_saved_status;
	GPR m_registers;

	CSPR& cspr() { return m_status; }
	const CSPR& cspr() const { return m_status; }

	std::optional<std::reference_wrapper<CSPR>> spsr() {
		switch(cspr().mode()) {
			case PRIV_MODE::FIQ: return m_saved_status.m_FIQ;
			case PRIV_MODE::SVC: return m_saved_status.m_SVC;
			case PRIV_MODE::ABT: return m_saved_status.m_ABT;
			case PRIV_MODE::IRQ: return m_saved_status.m_IRQ;
			case PRIV_MODE::UND: return m_saved_status.m_UND;
			default: return {};
		}
	}

	uint32 const& cr13() const {
		switch(cspr().mode()) {
			case PRIV_MODE::SYS:
			case PRIV_MODE::USR: return m_registers.m_base[13];
			case PRIV_MODE::FIQ: return m_registers.m_gFIQ[5];
			case PRIV_MODE::SVC: return m_registers.m_gSVC[0];
			case PRIV_MODE::ABT: return m_registers.m_gABT[0];
			case PRIV_MODE::IRQ: return m_registers.m_gIRQ[0];
			case PRIV_MODE::UND: return m_registers.m_gUND[0];
			default: ASSERT_NOT_REACHED();
		}
	}
	uint32& r13() {
		//  FIXME: This is an abomination
		return const_cast<uint32&>(static_cast<const ARM7TDMI*>(this)->cr13());
	}

	uint32 const& r14() const {
		switch(cspr().mode()) {
			case PRIV_MODE::SYS:
			case PRIV_MODE::USR: return m_registers.m_base[14];
			case PRIV_MODE::FIQ: return m_registers.m_gFIQ[6];
			case PRIV_MODE::SVC: return m_registers.m_gSVC[1];
			case PRIV_MODE::ABT: return m_registers.m_gABT[1];
			case PRIV_MODE::IRQ: return m_registers.m_gIRQ[1];
			case PRIV_MODE::UND: return m_registers.m_gUND[1];
			default: ASSERT_NOT_REACHED();
		}
	}
	uint32& r14() {
		//  FIXME: This is an abomination
		return const_cast<uint32&>(static_cast<const ARM7TDMI*>(this)->r14());
	}

	uint32& pc() {
		m_pc_dirty = true;
		return m_registers.m_base[15];
	}
	uint32 const& const_pc() const { return m_registers.m_base[15]; }
	uint32& sp() { return r13(); }
	uint32& lr() { return r14(); }

	uint32& reg(uint8 num) {
		assert(num < 16);
		if(num <= 7)
			return m_registers.m_base[num];
		else if(num >= 8 && num <= 12)
			return (cspr().mode() == PRIV_MODE::FIQ ? m_registers.m_gFIQ[num - 8] : m_registers.m_base[num]);
		else if(num == 13)
			return r13();
		else if(num == 14)
			return r14();
		else if(num == 15)
			return pc();
		ASSERT_NOT_REACHED();
	}
	uint32 const& creg(uint8 num) const {
		assert(num < 16);
		if(num <= 7)
			return m_registers.m_base[num];
		else if(num >= 8 && num <= 12)
			return (cspr().mode() == PRIV_MODE::FIQ ? m_registers.m_gFIQ[num - 8] : m_registers.m_base[num]);
		else if(num == 13)
			return cr13();
		else if(num == 14)
			return r14();
		else if(num == 15)
			return const_pc();
		ASSERT_NOT_REACHED();
	}

	mutable unsigned m_wait_cycles { 0 };
	uint64 m_cycles { 0 };
	bool m_pc_dirty { false };
	void exec_opcode();
	void execute_ARM(uint32 opcode);
	void execute_THUMB(uint16 opcode);
	uint32 fetch_instruction();
	[[nodiscard]] inline size_t current_instr_len() const { return ((cspr().state() == INSTR_MODE::ARM) ? 4 : 2); }

	bool irqs_enabled_globally() const;
	void enter_irq();
	void enter_swi();
	bool handle_halt();
	void handle_interrupts();

	void _alu_set_flags_logical_op(uint32 result);
	uint32 _alu_adc(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_sbc(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_add(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_sub(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_and(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_or(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_eor(uint32 op1, uint32 op2, bool should_affect_flags);
	uint32 _alu_not(uint32 op, bool should_affect_flags);
	uint32 _shift_lsl(uint32 op1, uint32 op2, bool affect_carry = true);
	uint32 _shift_lsr(uint32 op1, uint32 op2, bool affect_carry = true);
	uint32 _shift_asr(uint32 op1, uint32 op2, bool affect_carry = true);
	uint32 _shift_ror(uint32 op1, uint32 op2, bool affect_carry = true);
	uint32 _alu_lsl(uint32 op1, uint32 op2);
	uint32 _alu_lsr(uint32 op1, uint32 op2);
	uint32 _alu_asr(uint32 op1, uint32 op2);
	uint32 _alu_ror(uint32 op1, uint32 op2);

	//  FIXME: Move this elsewhere
	constexpr unsigned mult_m_cycles(uint64 multiplier) {
		multiplier >>= 8;
		if(multiplier == 0 || multiplier == 0xFFFFFF)
			return 1;
		multiplier >>= 8;
		if(multiplier == 0 || multiplier == 0xFFFF)
			return 2;
		multiplier >>= 8;
		if(multiplier == 0 || multiplier == 0xFF)
			return 3;
		return 4;
	}

	//  FIXME: Move this elsewhere
	constexpr unsigned unsigned_mult_m_cycles(uint64 multiplier) {
		multiplier >>= 8;
		if(multiplier == 0)
			return 1;
		multiplier >>= 8;
		if(multiplier == 0)
			return 2;
		multiplier >>= 8;
		if(multiplier == 0)
			return 3;
		return 4;
	}

	uint32 evaluate_operand1(disarmv4t::arm::instr::DataProcessInstruction instr) const;
	uint32 evaluate_operand2(disarmv4t::arm::instr::DataProcessInstruction instr, bool affect_carry = false);
	void stack_push32(uint32 val);
	uint32 stack_pop32();

	/*
	 *  ARM Opcodes
	 */
	void BX(disarmv4t::arm::instr::BXInstruction);
	void B(disarmv4t::arm::instr::BInstruction);
	void SWP(disarmv4t::arm::instr::SWPInstruction);
	void DPI(disarmv4t::arm::instr::DataProcessInstruction);
	void AND(disarmv4t::arm::instr::DataProcessInstruction);
	void EOR(disarmv4t::arm::instr::DataProcessInstruction);
	void SUB(disarmv4t::arm::instr::DataProcessInstruction);
	void RSB(disarmv4t::arm::instr::DataProcessInstruction);
	void ADD(disarmv4t::arm::instr::DataProcessInstruction);
	void ADC(disarmv4t::arm::instr::DataProcessInstruction);
	void SBC(disarmv4t::arm::instr::DataProcessInstruction);
	void RSC(disarmv4t::arm::instr::DataProcessInstruction);
	void TST(disarmv4t::arm::instr::DataProcessInstruction);
	void TEQ(disarmv4t::arm::instr::DataProcessInstruction);
	void CMP(disarmv4t::arm::instr::DataProcessInstruction);
	void CMN(disarmv4t::arm::instr::DataProcessInstruction);
	void ORR(disarmv4t::arm::instr::DataProcessInstruction);
	void MOV(disarmv4t::arm::instr::DataProcessInstruction);
	void BIC(disarmv4t::arm::instr::DataProcessInstruction);
	void MVN(disarmv4t::arm::instr::DataProcessInstruction);
	void SDT(disarmv4t::arm::instr::SDTInstruction);
	void SWI(disarmv4t::arm::instr::SWIInstruction);
	void MLL(disarmv4t::arm::instr::MultLongInstruction);
	void MUL(disarmv4t::arm::instr::MultInstruction);
	void BDT(disarmv4t::arm::instr::BDTInstruction);
	void HDT(disarmv4t::arm::instr::HDTInstruction);

	/*
	 *  THUMB opcodes
	 */
	void THUMB_FMT1(disarmv4t::thumb::instr::InstructionFormat1);
	void THUMB_FMT2(disarmv4t::thumb::instr::InstructionFormat2);
	void THUMB_FMT3(disarmv4t::thumb::instr::InstructionFormat3);
	void THUMB_FMT5(disarmv4t::thumb::instr::InstructionFormat5);
	void THUMB_FMT6(disarmv4t::thumb::instr::InstructionFormat6);
	void THUMB_FMT7(disarmv4t::thumb::instr::InstructionFormat7);
	void THUMB_FMT8(disarmv4t::thumb::instr::InstructionFormat8);
	void THUMB_FMT9(disarmv4t::thumb::instr::InstructionFormat9);
	void THUMB_FMT10(disarmv4t::thumb::instr::InstructionFormat10);
	void THUMB_FMT11(disarmv4t::thumb::instr::InstructionFormat11);
	void THUMB_FMT12(disarmv4t::thumb::instr::InstructionFormat12);
	void THUMB_FMT13(disarmv4t::thumb::instr::InstructionFormat13);
	void THUMB_FMT14(disarmv4t::thumb::instr::InstructionFormat14);
	void THUMB_FMT15(disarmv4t::thumb::instr::InstructionFormat15);
	void THUMB_FMT16(disarmv4t::thumb::instr::InstructionFormat16);
	void THUMB_FMT17(disarmv4t::thumb::instr::InstructionFormat17);
	void THUMB_FMT18(disarmv4t::thumb::instr::InstructionFormat18);
	void THUMB_FMT19(disarmv4t::thumb::instr::InstructionFormat19);
	void THUMB_ALU(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_AND(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_EOR(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_LSL(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_LSR(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_ASR(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_ADC(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_SBC(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_ROR(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_TST(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_NEG(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_CMP(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_CMN(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_ORR(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_MUL(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_BIC(disarmv4t::thumb::instr::InstructionFormat4);
	void THUMB_MVN(disarmv4t::thumb::instr::InstructionFormat4);

	template<typename... Args>
	void log(const char* format, const Args&... args) const {
		fmt::print("\u001b[32mARM7TDMI{{{}, mode={}, lr={:08x} sp={:08x}, pc={:08x}}}/ ", m_cycles, cspr().mode_str(),
		           r14(), cr13(), const_pc());
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}
	void dump_memory_around_pc() const;

	//  0xa50918a4
	uint8 mem_read8(uint32 address) const;
	uint16 mem_read16(uint32 address) const;
	uint32 mem_read32(uint32 address) const;
	void mem_write8(uint32 address, uint8 val);
	void mem_write16(uint32 address, uint16 val);
	void mem_write32(uint32 address, uint32 val);
	unsigned mem_waits_access32(uint32 address, AccessType type);
	unsigned mem_waits_access16(uint32 address, AccessType type);
	unsigned mem_waits_access8(uint32 address, AccessType type);
	uint32 mem_read_arm_opcode(uint32 address) const;
	uint16 mem_read_thumb_opcode(uint32 address) const;

	/*  ==============================================
	 *                      DMA
	 *  ==============================================
	 */
	template<unsigned x>
	void dma_resume();
	template<unsigned x>
	void dma_run();
	template<unsigned x>
	bool dma_is_running();
	void dma_run_all();

	/*  ==============================================
	 *                      Timers
	 *  ==============================================
	 */
	template<unsigned timer_num>
	void timers_cycle_n(Timer<timer_num>& timer, size_t n);
	template<unsigned timer_num>
	void timers_increment(Timer<timer_num>& timer);
	void timers_cycle_all(size_t n);

	unsigned run_to_next_state();
public:
	ARM7TDMI(GaBber& emu);

	void reset();
	unsigned run_next_instruction();

	void raise_irq(IRQType);
	void dma_start_vblank();
	void dma_start_hblank();
	void dma_request_fifoA();
	void dma_request_fifoB();
	template<unsigned x>
	void dma_on_enable();
};