#pragma once
#include "Headers/StdTypes.hpp"
#include "Tests/Test.hpp"

class GaBber;

class TestHarness {
	GaBber& m_gba;
	unsigned m_tests_run;
	unsigned m_tests_failed;

	std::string expectation_format_fail(Expectation const& expectation);

	bool test_validate_mem(MemState const&);
	bool test_validate_reg(RegState const&);
	bool test_validate_flag(FlagState const&);
	bool test_validate(Expectation const& expect) {
		switch (expect.m_type) {
			case ExpectType::Memory: return test_validate_mem(expect.m_data.m_mem_expect);
			case ExpectType::Register: return test_validate_reg(expect.m_data.m_reg_expect);
			case ExpectType::Flag: return test_validate_flag(expect.m_data.m_flag_expect);
			default: return true;
		}
	}

	void test_set_mem(MemState const&);
	void test_set_reg(RegState const&);
	void test_set_flag(FlagState const&);
	inline void test_set(InitialState const& expect) {
		switch (expect.m_type) {
			case ExpectType::Memory: {
				test_set_mem(expect.m_data.m_mem_expect);
				break;
			}
			case ExpectType::Register: {
				test_set_reg(expect.m_data.m_reg_expect);
				break;
			}
			case ExpectType::Flag: {
				test_set_flag(expect.m_data.m_flag_expect);
				break;
			}
			default: break;
		}
	}
	void test_run(Test&);
public:
	TestHarness(GaBber& v)
	: m_gba(v) {}

	void run_emulator_tests();
};