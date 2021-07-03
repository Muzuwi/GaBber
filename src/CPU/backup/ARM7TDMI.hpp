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
	UndefinedInstr = 5,
	SWI = 6,
	Reserved = 7,
};

enum class PipelineState {
	Fetch,
	Decode,
	Execute
};

//enum class OpExecState {
//	Finished,   //  Instruction has finished execution in this cycle
//	Stall       //  Instruction continues execution onto the next cycle
//};

struct PipelineInstruction {
	uint32 m_opcode;
	uint16 m_cycles;
	uint32 m_pc;
};


class MMU;

/*
 *  Implementation of the ARM7TDMI processor
 */
class ARM7TDMI {
protected:
	friend class Debugger;
	friend class GaBber;
	friend class TestHarness;

	MMU& m_mmu;

	CSPR m_status;
	SPSR m_saved_status;
	GPR m_registers;

	class : public IOReg<0x04000200, IEReg, IOAccess::RW> {
	public:
		virtual void on_write(uint16 val) override {
//			fmt::print("IE write {:04x}", val&~0xc000);
			m_register.m_raw = (val & ~0xc000);
		}
	} m_IE;

	class : public IOReg<0x04000202, IFReg, IOAccess::RW> {
		virtual void on_write(uint16 val) override {
			m_register.m_raw &= ~val;
		}
	} m_IF;

	class : public IOReg<0x04000208, IMEReg, IOAccess::RW> {
		virtual void on_write(uint32 val) override {
			m_register.m_raw = val & 1u;
//			if(val&1u)
//				fmt::print("IME enabled");
//			else
//				fmt::print("IME disabled");
		}
	} m_IME;
	class : public IOReg<0x04000300, _DummyReg<uint8>, IOAccess::RW> {} m_POSTFLG;

	class : public IOReg<0x04000301, _DummyReg<uint8>, IOAccess::RW> {
		virtual void on_write(uint8 val) override {
			//  Halt
			if(val == 0) {
				m_halt = true;
			}
			else if (val == 0x80)
				m_stop = true;
		}
	public:
		bool m_halt {false};
		bool m_stop {false};
	} m_HALTCNT;


	class : public IOReg<0x04000088, _DummyReg<uint32>, IOAccess::RW> {} m_soundbias;
	class : public IOReg<0x04000204, _DummyReg<uint16>, IOAccess::RW> {} m_waitcnt;

	Timers m_timers;
	DMA<0> m_dma0;
	DMA<1> m_dma1;
	DMA<2> m_dma2;
	DMA<3> m_dma3;

	CSPR& cspr() {
		return m_status;
	}

	const CSPR& cspr() const {
		return m_status;
	}

	Optional<RefWrapper<CSPR>> spsr() {
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

	List<PipelineInstruction> m_prefetched;
	bool m_is_pipeline_stalled {false};

	PipelineInstruction& current_instruction() {
		assert(!m_prefetched.empty());
		return m_prefetched.front();
	}

	PipelineInstruction const& current_instruction() const {
		assert(!m_prefetched.empty());
		return m_prefetched.front();
	}


    uint64 m_cycles {0};
    bool m_pc_dirty {false};
	uint32 m_cycles_since_pc_change {0};
	inline void pc_increment() {
		m_registers.m_base[15] += current_instr_len();
	}
    void _pipeline_fetch();
    void _pipeline_flush_and_fetch();
    [[nodiscard]] bool is_pipeline_stalled() const { return m_is_pipeline_stalled; }
	void exec_opcode();

	void execute_ARM(uint32 opcode);
	void execute_THUMB(uint16 opcode);
	[[nodiscard]] inline size_t current_instr_len() const { return ((cspr().state() == INSTR_MODE::ARM) ? 4 : 2); }

	unsigned _coverage_THUMB[20] {};
	unsigned _coverage_ARM[11] {};

	uint8 m_exception_lines {0};
	[[nodiscard]] bool is_ime_enabled() const { return m_IME.raw() & 1u; }
	[[nodiscard]] bool irqs_enabled_globally() const { return is_ime_enabled() && !cspr().is_set(CSPR_REGISTERS::IRQn); }
	[[nodiscard]] bool is_irq_enabled(IRQType type) { return m_IE.raw() & (1u << static_cast<unsigned>(type)); }
	[[nodiscard]] bool is_irq_requested(IRQType type) { return m_IF.raw() & (1u << static_cast<unsigned>(type)); }
	bool handle_exceptions();
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

	uint32 rotr32 (uint32_t n, unsigned int c);

	uint32 evaluate_shift_operand(ARM::DataProcessInstruction, bool affect_carry = false);

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
	OpExecState BX(ARM::BXInstruction);
	OpExecState B(ARM::BInstruction);
	OpExecState SWP(ARM::SWPInstruction);
	OpExecState DPI(ARM::DataProcessInstruction);
	OpExecState AND(ARM::DataProcessInstruction);
	OpExecState EOR(ARM::DataProcessInstruction);
	OpExecState SUB(ARM::DataProcessInstruction);
	OpExecState RSB(ARM::DataProcessInstruction);
	OpExecState ADD(ARM::DataProcessInstruction);
	OpExecState ADC(ARM::DataProcessInstruction);
	OpExecState SBC(ARM::DataProcessInstruction);
	OpExecState RSC(ARM::DataProcessInstruction);
	OpExecState TST(ARM::DataProcessInstruction);
	OpExecState TEQ(ARM::DataProcessInstruction);
	OpExecState CMP(ARM::DataProcessInstruction);
	OpExecState CMN(ARM::DataProcessInstruction);
	OpExecState ORR(ARM::DataProcessInstruction);
	OpExecState MOV(ARM::DataProcessInstruction);
	OpExecState BIC(ARM::DataProcessInstruction);
	OpExecState MVN(ARM::DataProcessInstruction);
	OpExecState SDT(ARM::SDTInstruction);
	OpExecState SWI(ARM::SWIInstruction);
	OpExecState MLL(ARM::MultLongInstruction);
	OpExecState MUL(ARM::MultInstruction);
	OpExecState BDT(ARM::BDTInstruction);
	OpExecState HDT(ARM::HDTInstruction);

	/*
	 *  THUMB opcodes
	 */
	OpExecState THUMB_FMT1(THUMB::InstructionFormat1);
	OpExecState THUMB_FMT2(THUMB::InstructionFormat2);
	OpExecState THUMB_FMT3(THUMB::InstructionFormat3);
	OpExecState THUMB_FMT5(THUMB::InstructionFormat5);
	OpExecState THUMB_FMT6(THUMB::InstructionFormat6);
	OpExecState THUMB_FMT7(THUMB::InstructionFormat7);
	OpExecState THUMB_FMT8(THUMB::InstructionFormat8);
	OpExecState THUMB_FMT9(THUMB::InstructionFormat9);
	OpExecState THUMB_FMT10(THUMB::InstructionFormat10);
	OpExecState THUMB_FMT11(THUMB::InstructionFormat11);
	OpExecState THUMB_FMT12(THUMB::InstructionFormat12);
	OpExecState THUMB_FMT13(THUMB::InstructionFormat13);
	OpExecState THUMB_FMT14(THUMB::InstructionFormat14);
	OpExecState THUMB_FMT15(THUMB::InstructionFormat15);
	OpExecState THUMB_FMT16(THUMB::InstructionFormat16);
	OpExecState THUMB_FMT17(THUMB::InstructionFormat17);
	OpExecState THUMB_FMT18(THUMB::InstructionFormat18);
	OpExecState THUMB_FMT19(THUMB::InstructionFormat19);
	OpExecState THUMB_ALU(THUMB::InstructionFormat4);
	OpExecState THUMB_AND(THUMB::InstructionFormat4);
	OpExecState THUMB_EOR(THUMB::InstructionFormat4);
	OpExecState THUMB_LSL(THUMB::InstructionFormat4);
	OpExecState THUMB_LSR(THUMB::InstructionFormat4);
	OpExecState THUMB_ASR(THUMB::InstructionFormat4);
	OpExecState THUMB_ADC(THUMB::InstructionFormat4);
	OpExecState THUMB_SBC(THUMB::InstructionFormat4);
	OpExecState THUMB_ROR(THUMB::InstructionFormat4);
	OpExecState THUMB_TST(THUMB::InstructionFormat4);
	OpExecState THUMB_NEG(THUMB::InstructionFormat4);
	OpExecState THUMB_CMP(THUMB::InstructionFormat4);
	OpExecState THUMB_CMN(THUMB::InstructionFormat4);
	OpExecState THUMB_ORR(THUMB::InstructionFormat4);
	OpExecState THUMB_MUL(THUMB::InstructionFormat4);
	OpExecState THUMB_BIC(THUMB::InstructionFormat4);
	OpExecState THUMB_MVN(THUMB::InstructionFormat4);

	template<typename... Args>
	void log(const char* format, const Args& ...args) const {
		fmt::print("\u001b[32mARM7TDMI{{{}, mode={}, lr={:08x} sp={:08x}, pc={:08x}}}/ ", m_cycles, cspr().mode_str(), r14(), cr13(), const_pc());
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}

	struct ExceptionDebug {
		uint32 return_pc;
		PRIV_MODE entry_mode;
		CSPR saved_psr;
	};
	List<ExceptionDebug> m_excepts;

	bool handle_dma_transfers();
public:
	explicit ARM7TDMI(MMU& v)
	: m_mmu(v), m_registers(), m_timers(*this), m_dma0(*this), m_dma1(*this), m_dma2(*this), m_dma3(*this) {}
	MMU& mmu() { return m_mmu; }

	void cycle();
	void reset();

    void dump_regs();

	void raise_irq(IRQType);
	void raise_exception(ExceptionVector);
};