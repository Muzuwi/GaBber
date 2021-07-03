#pragma once
#include "Headers/StdTypes.hpp"
#include "Headers/ARM7TDMI.hpp"

struct MemState {
	enum class OpSize {
		b32,
		b16,
		b8
	};
	OpSize m_size;

	uint32 m_address;
	union {
		uint32 m_word;
		uint16 m_hword;
		uint8 m_byte;
	};

	MemState() = default;

	explicit MemState(uint32 address, uint32 val)
	: m_size(OpSize::b32), m_address(address), m_word(val) {}

	explicit MemState(uint32 address, uint16 val)
	: m_size(OpSize::b16), m_address(address), m_hword(val) {}

	explicit MemState(uint32 address, uint8 val)
	: m_size(OpSize::b8), m_address(address), m_byte(val) {}
};

struct RegState {
	uint8 m_reg;
	uint32 m_reg_value;

	RegState() = default;
	RegState(uint8 reg, uint32 value)
	: m_reg(reg & 0xfu), m_reg_value(value) {}
};

struct FlagState {
	CSPR m_flag_reg;

	FlagState() = default;
	FlagState(CSPR const& v)
	: m_flag_reg(v) {}
};



enum class ExpectType {
	Memory,
	Register,
	Flag
};

union ExpectationData {
	RegState m_reg_expect;
	MemState m_mem_expect;
	FlagState m_flag_expect;
};

struct Expectation {
	ExpectType m_type;
	ExpectationData m_data;

	Expectation(RegState reg_expect)
	: m_type(ExpectType::Register) {
		m_data.m_reg_expect = reg_expect;
	}

	Expectation(MemState mem_expect)
	: m_type(ExpectType::Memory) {
		m_data.m_mem_expect = mem_expect;
	}

	Expectation(FlagState flag_expect)
	: m_type(ExpectType::Flag) {
		m_data.m_flag_expect = flag_expect;
	}
};

struct InitialState {
	ExpectType m_type;
	ExpectationData m_data;

	InitialState(RegState reg)
	: m_type(ExpectType::Register) {
		m_data.m_reg_expect = reg;
	}

	InitialState(MemState mem)
	: m_type(ExpectType::Memory) {
		m_data.m_mem_expect = mem;
	}

	InitialState(FlagState state)
	: m_type(ExpectType::Flag) {
		m_data.m_flag_expect = state;
	}
};


enum class TestType {
	InstructionARM,
	InstructionTHUMB
};

class Test {
	TestType m_test_type;
	uint32 m_opcode;
	List<Expectation> m_expects;
	List<InitialState> m_state;
	std::string m_test_case;

	void expect_add(Expectation const& expectation) {
		switch (expectation.m_type) {
			case ExpectType::Register: {
				auto it = std::find_if(m_expects.begin(), m_expects.end(), [&expectation](Expectation const& v) -> bool {
                    return v.m_type == ExpectType::Register && v.m_data.m_reg_expect.m_reg == expectation.m_data.m_reg_expect.m_reg;
				});

				if(it != m_expects.end()) return;
				break;
			}
			default: break;
		}

		m_expects.push_back(expectation);
	}
public:

	Test(TestType type, uint32 opcode, std::string test_case)
	: m_test_type(type), m_opcode(opcode), m_test_case(test_case) {}

	Test& expect(Expectation const& exp) {
		expect_add(exp);
		return *this;
	}

	Test& prepare(InitialState const& state) {
		m_state.push_back(state);
		return *this;
	}

	uint32 opcode() const { return m_opcode; }
	TestType type() const { return m_test_type; }
	List<Expectation> const& expectations() const { return m_expects; }
	List<InitialState> const& initial_states() const { return m_state; }
	std::string test_case() const { return m_test_case; }
};
