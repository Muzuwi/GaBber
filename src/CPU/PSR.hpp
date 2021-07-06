#pragma once
#include "Headers/StdTypes.hpp"

enum class INSTR_MODE{
	ARM,
	THUMB
};

enum class PRIV_MODE{
	USR = 0b10000,
	FIQ = 0b10001,
	IRQ = 0b10010,
	SVC = 0b10011,
	ABT = 0b10111,
	UND = 0b11011,
	SYS = 0b11111,
};

/*
 *  Masks for the CSPR
 */
enum class CSPR_REGISTERS : unsigned int {
	Negative = 1u << 31u,
	Zero = 1u << 30u,
	Carry = 1u << 29u,
	Overflow = 1u << 28u,
	//  ...  reserved ...
	IRQn = 1u << 7u,
	FIQn = 1u << 6u,
	State = 1u << 5u,
	M4 = 1u << 4u,
	M3 = 1u << 3u,
	M2 = 1u << 2u,
	M1 = 1u << 1u,
	M0 = 1u << 0u,
};

/*
 *  Current Program Status Register
 */
class CSPR {
	uint32 data;

	static inline bool verify_mode(uint32 data) {
		data &= 0b11111;
		switch(static_cast<PRIV_MODE>(data)) {
			case PRIV_MODE::USR:
			case PRIV_MODE::FIQ:
			case PRIV_MODE::SVC:
			case PRIV_MODE::ABT:
			case PRIV_MODE::IRQ:
			case PRIV_MODE::UND:
			case PRIV_MODE::SYS:
				return true;
			default:
				return false;
		}

	}

public:
	CSPR() = default;

	void set_raw(uint32 v) {
		if(mode() != PRIV_MODE::USR && !verify_mode(v)) {
			fmt::print("PSR/ Tried writing invalid mode bits! {:08x}\n", v);
			v = (v & ~0x1f) | (uint32)mode();
		}

		if(mode() == PRIV_MODE::USR) {
			data = (v & 0xF0000000) | (data & ~0xF0000000);
		} else {
			data = v;
		}
	}

	uint32 raw() const {
		return data;
	}

	inline bool is_set(CSPR_REGISTERS mask) const {
		return (data & (uint32)mask);
	}

	inline bool is_clear(CSPR_REGISTERS mask) const {
		return !(data & (uint32)mask);
	}

	void set(CSPR_REGISTERS reg, bool v) {
		data = (data & (~(uint32)reg)) | (v ? (uint32)reg : 0);
	}

	void set_flags(uint32 flags, bool F, bool S, bool X, bool C) {
		if(F) {
			data &= ~0xff000000;
			data |= flags & 0xff000000;
		}

		if(S) {
			data &= ~0x00ff0000;
			data |= flags & 0x00ff0000;
		}

		if(X) {
			data &= ~0x0000ff00;
			data |= flags & 0x000ff000;
		}

		if(C && mode() != PRIV_MODE::USR) {
			if(!verify_mode(flags)) {
				fmt::print("PSR/ Tried writing invalid mode bits! {:08x}\n", flags);
				flags = (flags & ~0x1f) | (uint32)mode();
			}

			data &= ~0x000000ff;
			data |= flags & 0x000000ff;
		}
	}

	INSTR_MODE state() const {
		return (data & ((uint32)CSPR_REGISTERS::State)) ? INSTR_MODE::THUMB
		                                                : INSTR_MODE::ARM;
	}

	void set_state(INSTR_MODE state) {
		data = (data & ~((uint32)CSPR_REGISTERS::State))
		       | ((state == INSTR_MODE::THUMB) ? (uint32)CSPR_REGISTERS::State : 0u);
	}

	PRIV_MODE mode() const {
		return static_cast<PRIV_MODE>(data & (0b11111));
	}

	const char* mode_str() const {
		switch (mode()) {
			case PRIV_MODE::USR: return "USR";
			case PRIV_MODE::FIQ: return "FIQ";
			case PRIV_MODE::SVC: return "SVC";
			case PRIV_MODE::ABT: return "ABT";
			case PRIV_MODE::IRQ: return "IRQ";
			case PRIV_MODE::UND: return "UND";
			case PRIV_MODE::SYS: return "SYS";
			default: return "INVALID";
		}
	}

	void set_mode(PRIV_MODE mode) {
		data = (data & ~0b11111u) | ((uint32)mode & 0b11111u);
	}

	bool evaluate_condition(ARM::InstructionCondition condition) const {
		switch (condition) {
			case ARM::InstructionCondition::EQ:
				return is_set(CSPR_REGISTERS::Zero);
			case ARM::InstructionCondition::NE:
				return is_clear(CSPR_REGISTERS::Zero);
			case ARM::InstructionCondition::CS:
				return is_set(CSPR_REGISTERS::Carry);
			case ARM::InstructionCondition::CC:
				return is_clear(CSPR_REGISTERS::Carry);
			case ARM::InstructionCondition::MI:
				return is_set(CSPR_REGISTERS::Negative);
			case ARM::InstructionCondition::PL:
				return is_clear(CSPR_REGISTERS::Negative);
			case ARM::InstructionCondition::VS:
				return is_set(CSPR_REGISTERS::Overflow);
			case ARM::InstructionCondition::VC:
				return is_clear(CSPR_REGISTERS::Overflow);
			case ARM::InstructionCondition::HI:
				return is_set(CSPR_REGISTERS::Carry) && is_clear(CSPR_REGISTERS::Zero);
			case ARM::InstructionCondition::LS:
				return is_clear(CSPR_REGISTERS::Carry) || is_set(CSPR_REGISTERS::Zero);
			case ARM::InstructionCondition::GE:
				return is_set(CSPR_REGISTERS::Negative) == is_set(CSPR_REGISTERS::Overflow);
			case ARM::InstructionCondition::LT:
				return is_set(CSPR_REGISTERS::Negative) ^ is_set(CSPR_REGISTERS::Overflow);
			case ARM::InstructionCondition::GT:
				return is_clear(CSPR_REGISTERS::Zero) && (is_set(CSPR_REGISTERS::Negative) == is_set(CSPR_REGISTERS::Overflow));
			case ARM::InstructionCondition::LE:
				return is_set(CSPR_REGISTERS::Zero) || (is_set(CSPR_REGISTERS::Negative) != is_set(CSPR_REGISTERS::Overflow));
			case ARM::InstructionCondition::AL:
			default:
				return true;
		}
	}
};

/*
 *  Saved Program Status Register
 */
struct SPSR {
	CSPR m_FIQ;
	CSPR m_SVC;
	CSPR m_ABT;
	CSPR m_IRQ;
	CSPR m_UND;
};