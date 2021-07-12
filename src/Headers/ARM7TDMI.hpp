#pragma once
#include <fmt/format.h>
#include "Headers/ARM_Instruction.hpp"
#include "Headers/StdTypes.hpp"
#include "Headers/THUMB_Instruction.hpp"
#include "CPU/GPR.hpp"
#include "CPU/PSR.hpp"
#include "CPU/Interrupt.hpp"
#include "CPU/Unions.hpp"
#include "MMU/IOReg.hpp"
#include "CPU/Timer.hpp"
#include "CPU/DMA.hpp"

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


class MMU;
class Debugger;

/*
 *  Implementation of the ARM7TDMI processor
 */
class ARM7TDMI {
protected:
	friend class GPRs;
	friend class IORegisters;
	friend class TestHarness;

	MMU& m_mmu;
	Debugger& m_debugger;

	CSPR m_status;
	SPSR m_saved_status;
	GPR m_registers;

	IE m_IE;
	IF m_IF;
	IME m_IME;
	HALTCNT m_HALTCNT;
	POSTFLG m_POSTFLG;


	class : public IOReg<0x04000088, _DummyReg<uint32>, IOAccess::RW> {
		void reload() override {
			raw() = 0x0200;
		}
	} m_soundbias;

	Timers m_timers;

	CSPR& cspr() {
		return m_status;
	}

	const CSPR& cspr() const {
		return m_status;
	}

	Optional<Ref<CSPR>> spsr() {
		switch (cspr().mode()) {
			case PRIV_MODE::FIQ:
				return m_saved_status.m_FIQ;
			case PRIV_MODE::SVC:
				return m_saved_status.m_SVC;
			case PRIV_MODE::ABT:
				return m_saved_status.m_ABT;
			case PRIV_MODE::IRQ:
				return m_saved_status.m_IRQ;
			case PRIV_MODE::UND:
				return m_saved_status.m_UND;
			default:
				return {};
		}
	}

	uint32 const& cr13() const {
		switch (cspr().mode()) {
			case PRIV_MODE::SYS:
			case PRIV_MODE::USR:
				return m_registers.m_base[13];
			case PRIV_MODE::FIQ:
				return m_registers.m_gFIQ[5];
			case PRIV_MODE::SVC:
				return m_registers.m_gSVC[0];
			case PRIV_MODE::ABT:
				return m_registers.m_gABT[0];
			case PRIV_MODE::IRQ:
				return m_registers.m_gIRQ[0];
			case PRIV_MODE::UND:
				return m_registers.m_gUND[0];
			default:
				ASSERT_NOT_REACHED();
		}
    }
    uint32& r13() {
		//  FIXME: This is an abomination
		return const_cast<uint32&>(static_cast<const ARM7TDMI*>(this)->cr13());
	}

	uint32 const& r14() const {
		switch (cspr().mode()) {
			case PRIV_MODE::SYS:
			case PRIV_MODE::USR:
				return m_registers.m_base[14];
			case PRIV_MODE::FIQ:
				return m_registers.m_gFIQ[6];
			case PRIV_MODE::SVC:
				return m_registers.m_gSVC[1];
			case PRIV_MODE::ABT:
				return m_registers.m_gABT[1];
			case PRIV_MODE::IRQ:
				return m_registers.m_gIRQ[1];
			case PRIV_MODE::UND:
				return m_registers.m_gUND[1];
			default:
				ASSERT_NOT_REACHED();
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
    uint32 const& const_pc() const {
		return m_registers.m_base[15];
	}

    uint32& sp() {
    	return r13();
    }
	uint32& lr() {
		return r14();
	}

	uint32& reg(uint8 num) {
		assert(num < 16);
		if(num <= 7)
			return m_registers.m_base[num];
		else if(num >= 8 && num <= 12)
			return (cspr().mode() == PRIV_MODE::FIQ ? m_registers.m_gFIQ[num-8] : m_registers.m_base[num]);
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
			return (cspr().mode() == PRIV_MODE::FIQ ? m_registers.m_gFIQ[num-8] : m_registers.m_base[num]);
		else if(num == 13)
			return cr13();
		else if(num == 14)
			return r14();
		else if(num == 15)
			return const_pc();
		ASSERT_NOT_REACHED();
	}


    uint64 m_cycles {0};
    bool m_pc_dirty {false};
    void exec_opcode();
	void execute_ARM(uint32 opcode);
	void execute_THUMB(uint16 opcode);
	uint32 fetch_instruction();
	[[nodiscard]] inline size_t current_instr_len() const { return ((cspr().state() == INSTR_MODE::ARM) ? 4 : 2); }
	inline void pc_increment() { m_registers.m_base[15] += current_instr_len(); }


	[[nodiscard]] bool irqs_enabled_globally() const { return m_IME.enabled() && !cspr().is_set(CSPR_REGISTERS::IRQn); }
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
	inline uint32 evaluate_operand1(ARM::DataProcessInstruction instr) const {
		unsigned pc_offset = 0;
		if(!instr.immediate_is_value() && instr.is_shift_reg() && instr.operand1_reg() == 15)
			pc_offset = 4;
		return creg(instr.operand1_reg()) + pc_offset;
	}

	uint32 rotr32 (uint32_t n, unsigned int c);

	uint32 evaluate_operand2(ARM::DataProcessInstruction instr, bool affect_carry = false);

	template<size_t value_size>
	constexpr uint32 sign_extend(uint32 value) {
		static_assert(value_size < sizeof(uint32)*8);

		const auto mask = ((1 << value_size) - 1);
		const auto shift_count = 32 - value_size;

		auto u = (static_cast<uint32>(value) & mask);
		return ((signed int)(u << shift_count)) >> shift_count;
	}

	void stack_push32(uint32 val);
	uint32 stack_pop32();

    /*
     *  ARM Opcodes
     */
	void BX(ARM::BXInstruction);
	void B(ARM::BInstruction);
	void SWP(ARM::SWPInstruction);
	void DPI(ARM::DataProcessInstruction);
	void AND(ARM::DataProcessInstruction);
	void EOR(ARM::DataProcessInstruction);
	void SUB(ARM::DataProcessInstruction);
	void RSB(ARM::DataProcessInstruction);
	void ADD(ARM::DataProcessInstruction);
	void ADC(ARM::DataProcessInstruction);
	void SBC(ARM::DataProcessInstruction);
	void RSC(ARM::DataProcessInstruction);
	void TST(ARM::DataProcessInstruction);
	void TEQ(ARM::DataProcessInstruction);
	void CMP(ARM::DataProcessInstruction);
	void CMN(ARM::DataProcessInstruction);
	void ORR(ARM::DataProcessInstruction);
	void MOV(ARM::DataProcessInstruction);
	void BIC(ARM::DataProcessInstruction);
	void MVN(ARM::DataProcessInstruction);
	void SDT(ARM::SDTInstruction);
	void SWI(ARM::SWIInstruction);
	void MLL(ARM::MultLongInstruction);
	void MUL(ARM::MultInstruction);
	void BDT(ARM::BDTInstruction);
	void HDT(ARM::HDTInstruction);

	/*
	 *  THUMB opcodes
	 */
	void THUMB_FMT1(THUMB::InstructionFormat1);
	void THUMB_FMT2(THUMB::InstructionFormat2);
	void THUMB_FMT3(THUMB::InstructionFormat3);
	void THUMB_FMT5(THUMB::InstructionFormat5);
	void THUMB_FMT6(THUMB::InstructionFormat6);
	void THUMB_FMT7(THUMB::InstructionFormat7);
	void THUMB_FMT8(THUMB::InstructionFormat8);
	void THUMB_FMT9(THUMB::InstructionFormat9);
	void THUMB_FMT10(THUMB::InstructionFormat10);
	void THUMB_FMT11(THUMB::InstructionFormat11);
	void THUMB_FMT12(THUMB::InstructionFormat12);
	void THUMB_FMT13(THUMB::InstructionFormat13);
	void THUMB_FMT14(THUMB::InstructionFormat14);
	void THUMB_FMT15(THUMB::InstructionFormat15);
	void THUMB_FMT16(THUMB::InstructionFormat16);
	void THUMB_FMT17(THUMB::InstructionFormat17);
	void THUMB_FMT18(THUMB::InstructionFormat18);
	void THUMB_FMT19(THUMB::InstructionFormat19);
	void THUMB_ALU(THUMB::InstructionFormat4);
	void THUMB_AND(THUMB::InstructionFormat4);
	void THUMB_EOR(THUMB::InstructionFormat4);
	void THUMB_LSL(THUMB::InstructionFormat4);
	void THUMB_LSR(THUMB::InstructionFormat4);
	void THUMB_ASR(THUMB::InstructionFormat4);
	void THUMB_ADC(THUMB::InstructionFormat4);
	void THUMB_SBC(THUMB::InstructionFormat4);
	void THUMB_ROR(THUMB::InstructionFormat4);
	void THUMB_TST(THUMB::InstructionFormat4);
	void THUMB_NEG(THUMB::InstructionFormat4);
	void THUMB_CMP(THUMB::InstructionFormat4);
	void THUMB_CMN(THUMB::InstructionFormat4);
	void THUMB_ORR(THUMB::InstructionFormat4);
	void THUMB_MUL(THUMB::InstructionFormat4);
	void THUMB_BIC(THUMB::InstructionFormat4);
	void THUMB_MVN(THUMB::InstructionFormat4);

	template<typename... Args>
	void log(const char* format, const Args& ...args) const {
		fmt::print("\u001b[32mARM7TDMI{{{}, mode={}, lr={:08x} sp={:08x}, pc={:08x}}}/ ", m_cycles, cspr().mode_str(), r14(), cr13(), const_pc());
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}
	void dump_memory_around_pc() const;

	inline uint8 mem_read8(uint32 address) const {
		return m_mmu.read8(address);
	}
	inline uint16 mem_read16(uint32 address) const {
		return m_mmu.read16(address);
	}
	inline uint32 mem_read32(uint32 address) const {
		return m_mmu.read32(address);
	}

	struct WriteContext {
		uint32 pc;
		uint32 instruction;
		uint64 cycle;
		void* return_address;
	};
	struct {
		uint32 pc;
		uint64 cycle;
		std::string reason;
		INSTR_MODE prev;
		INSTR_MODE neu;
	} m_last_mode_change;

	std::unordered_map<uint32, WriteContext> m_last_written;
	void mem_write8(uint32 address, uint8 val) {
		m_mmu.write8(address, val);
		m_last_written[address] = WriteContext{.pc = (uint32)(const_pc() - 2*current_instr_len()), .instruction = mem_read32(const_pc() - 2*current_instr_len()), .cycle = m_cycles, .return_address = __builtin_return_address(0)};
	}
	void mem_write16(uint32 address, uint16 val) {
		m_mmu.write16(address, val);
		m_last_written[address] = WriteContext{.pc = (uint32)(const_pc() - 2*current_instr_len()), .instruction = mem_read32(const_pc() - 2*current_instr_len()), .cycle = m_cycles, .return_address = __builtin_return_address(0)};
	}
	void mem_write32(uint32 address, uint32 val) {
		m_mmu.write32(address, val);
		m_last_written[address] = WriteContext{.pc = (uint32)(const_pc() - 2*current_instr_len()), .instruction = mem_read32(const_pc() - 2*current_instr_len()), .cycle = m_cycles, .return_address = __builtin_return_address(0)};
	}



	/*  ==============================================
	 *                      DMA
	 *  ==============================================
	 */
	DMAData m_dma;
	template<unsigned x> void dma_start();
	template<unsigned x> void dma_cycle();
	template<unsigned x> inline bool dma_is_running() { return m_dma.get_data<x>().m_is_running; }
	template<unsigned x> inline bool dma_is_finished() { return m_dma.get_data<x>().m_finished; }
	bool dma_cycle_all();
public:
	explicit ARM7TDMI(MMU& v, Debugger& dbg)
	: m_mmu(v), m_debugger(dbg), m_registers(), m_timers(*this) {}
	MMU& mmu() { return m_mmu; }

	void cycle();
	void reset();

    void dump_regs();

	void raise_irq(IRQType);
	void dma_start_vblank();
	void dma_start_hblank();

	void logDebug();
};