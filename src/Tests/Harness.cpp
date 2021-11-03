#include "Tests/Harness.hpp"
#include "Headers/GaBber.hpp"
#include "CPU/Instructions/ARM.hpp"
#include "CPU/Instructions/THUMB.hpp"
#include "Test.hpp"

void TestHarness::run_emulator_tests() {
//	Test stuff {TestType::InstructionARM, 0xea000000, "Testing"};
//	stuff.prepare(RegState{0, 0})
//	     .prepare(RegState{1,0})
//	     .expect(RegState{15, 0})
//	     .expect(MemState{0x0, (uint16)0x0});
//	test_run(stuff);

	ARM::BInstruction::Reg instr {};
	instr.offset = 0x20;
	instr.link_flag = false;
	instr.condition = ARM::InstructionCondition::AL;
	instr._dummy = 0b101;

	Test branch {TestType::InstructionARM, instr._opcode, "Branch/No Link"};
	branch.prepare(RegState{15, 0x08000000})
	      .prepare(RegState{14, 0x11111111})
	      .expect(RegState{15, 0x08000000+0x20*4})
	      .expect(RegState{14, 0x11111111});
	test_run(branch);

	instr.link_flag = true;
	Test branchL {TestType::InstructionARM, instr._opcode, "Branch/Link"};
	branchL.prepare(RegState{15, 0x08000008})
	      .prepare(RegState{14, 0x11111111})
	      .expect(RegState{15, 0x08000008+0x20*4})
	      .expect(RegState{14, 0x08000004});
	test_run(branchL);

	Test branchXA {TestType::InstructionARM, 0xe12fff10 | 0b0001u, "Branch Exchange/Into ARM"};
	branchXA.prepare(RegState{15, 0x08000000})
			.prepare(RegState{1, 0x08000105})
			.expect(RegState{15, 0x08000104});
	test_run(branchXA);

	Test branchXT {TestType::InstructionARM, 0xe12fff10 | 0b0001u, "Branch Exchange/Into THUMB"};
	branchXT.prepare(RegState{15, 0x08000000})
	        .prepare(RegState{1, 0x08000103})
	        .expect(RegState{15, 0x08000102});
	test_run(branchXT);

	ARM::DataProcessInstruction::Reg r {};
	r.set_condition = false;
	r.opcode = 0b1101;
	r.condition = ARM::InstructionCondition::AL;
	r.operand2 = 0xff;
	r.destination_reg = 0b0010;
	r._bitseq1 = 0b00;
	r.immediate_operand = true;
    Test movT {TestType::InstructionARM, r._opcode, "MOV/Simple Immediate"};
    movT.prepare(RegState{2, 0xDEADBABE})
        .expect(RegState{2, 0xff});
	test_run(movT);

	r.immediate_operand = false;
	r.operand2 = 0b0011;
	Test movTR {TestType::InstructionARM, r._opcode, "MOV/Simple Reg"};
	movT.prepare(RegState{2, 0xDEADBABE})
		.prepare(RegState{3, 0xFEEDFACE})
	    .expect(RegState{2, 0xFEEDFACE});
	test_run(movTR);

	Test adcs {TestType::InstructionARM, 0xe0b00271, "ADCS/Shift Carry In"};
	CSPR t {};
	t.set(CSPR_REGISTERS::Carry, true);
	t.set_mode(PRIV_MODE::SYS);
	t.set_state(INSTR_MODE::ARM);
	adcs.prepare(FlagState{t})
		.prepare(RegState{0, 0})
		.prepare(RegState{1, 0x80})
		.prepare(RegState{2, 0x8})
		.expect(RegState{0, 0x80000001});
	test_run(adcs);

	Test adcs2 {TestType::InstructionARM, 0xe0b00271, "ADCS/Shift Carry In 2"};
	adcs2.prepare(FlagState{t})
	    .prepare(RegState{0, 0})
	    .prepare(RegState{1, 0x70})
	    .prepare(RegState{2, 0x8})
	    .expect(RegState{0, 0x70000001});
	test_run(adcs2);

	Test stmdbbase {TestType::InstructionARM, 0xe9200003, "BDT/STMDB base in rlist"};
	stmdbbase.prepare(RegState{0, 0x03ffffa0})
			 .prepare(RegState{1, 1})
			 .prepare(RegState{2, 2})
			 .prepare(MemState{0x03ffff9c, (uint32)0})
			 .prepare(MemState{0x03ffff9c-4, (uint32)0})
			 .expect(MemState{0x03ffff9c-4, (uint32)0x03ffffa0})
			 .expect(MemState{0x03ffff9c-0, (uint32)0x1})
			 .expect(RegState{0, 0x03ffff98});
	test_run(stmdbbase);

	Test stmdbbase2 {TestType::InstructionARM, 0xe922001f, "BDT/STMDB base in rlist/base not first"};
	stmdbbase2.prepare(RegState{0, 0})
	          .prepare(RegState{1, 1})
	          .prepare(RegState{2, 0x03ffffa0})
	          .prepare(RegState{3, 3})
	          .prepare(RegState{4, 4})
	          .prepare(MemState{0x03ffff9c, (uint32)0})
	          .prepare(MemState{0x03ffff9c-4, (uint32)0})
	          .prepare(MemState{0x03ffff9c-8, (uint32)0})
	          .prepare(MemState{0x03ffff9c-12, (uint32)0})
	          .prepare(MemState{0x03ffff9c-16, (uint32)0xffffffff})
	          .expect(MemState{0x03ffff9c, (uint32)4})
	          .expect(MemState{0x03ffff9c-4, (uint32)3})
	          .expect(MemState{0x03ffff9c-8, (uint32)0x03ffff8c})
	          .expect(MemState{0x03ffff9c-12, (uint32)1})
	          .expect(MemState{0x03ffff9c-16, (uint32)0})
	          .expect(RegState{2, 0x03ffff8c});
	test_run(stmdbbase2);

	Test stmdbbase3 {TestType::InstructionARM, 0xe92200fc, "BDT/STMDB base in rlist/base first"};
	stmdbbase3.prepare(RegState{2, 0x03ffffa0})
	          .prepare(RegState{3, 3})
	          .prepare(RegState{4, 4})
	          .prepare(RegState{5, 5})
	          .prepare(RegState{6, 6})
	          .prepare(RegState{7, 7})
	          .prepare(MemState{0x03ffff9c, (uint32)0})
	          .prepare(MemState{0x03ffff9c-4, (uint32)0})
	          .prepare(MemState{0x03ffff9c-8, (uint32)0})
	          .prepare(MemState{0x03ffff9c-12, (uint32)0})
	          .prepare(MemState{0x03ffff9c-16, (uint32)0})
	          .prepare(MemState{0x03ffff9c-20, (uint32)0})
	          .expect(MemState{0x03ffff9c, (uint32)7})
	          .expect(MemState{0x03ffff9c-4, (uint32)6})
	          .expect(MemState{0x03ffff9c-8, (uint32)5})
	          .expect(MemState{0x03ffff9c-12, (uint32)4})
	          .expect(MemState{0x03ffff9c-16, (uint32)3})
	          .expect(MemState{0x03ffff9c-20, (uint32)0x03ffffa0})
	          .expect(RegState{2, 0x03ffff88});
	test_run(stmdbbase3);

	Test stmibbase1 {TestType::InstructionARM, 0xe9a200ff, "BDT/STMIB base in rlist/base not first"};
	stmibbase1.prepare(RegState{0, 777})
	          .prepare(RegState{1, 1})
			  .prepare(RegState{2, 0x03ffffa0})
	          .prepare(RegState{3, 3})
	          .prepare(RegState{4, 4})
	          .prepare(RegState{5, 5})
	          .prepare(RegState{6, 6})
	          .prepare(RegState{7, 7})
	          .prepare(MemState{0x03ffffa4, (uint32)0})
	          .prepare(MemState{0x03ffffa4+4, (uint32)0})
	          .prepare(MemState{0x03ffffa4+8, (uint32)0})
	          .prepare(MemState{0x03ffffa4+12, (uint32)0})
	          .prepare(MemState{0x03ffffa4+16, (uint32)0})
	          .prepare(MemState{0x03ffffa4+20, (uint32)0})
	          .prepare(MemState{0x03ffffa4+24, (uint32)0})
	          .prepare(MemState{0x03ffffa4+28, (uint32)0})
	          .expect(MemState{0x03ffffa4, (uint32)777})
	          .expect(MemState{0x03ffffa4+4, (uint32)1})
	          .expect(MemState{0x03ffffa4+8, (uint32)0x03ffffc0})
	          .expect(MemState{0x03ffffa4+12, (uint32)3})
	          .expect(MemState{0x03ffffa4+16, (uint32)4})
	          .expect(MemState{0x03ffffa4+20, (uint32)5})
	          .expect(MemState{0x03ffffa4+24, (uint32)6})
	          .expect(MemState{0x03ffffa4+28, (uint32)7})
	          .expect(RegState{2, 0x03ffffc0});
	test_run(stmibbase1);

	Test stmdbemptyrlist {TestType::InstructionARM, 0xE8200000, "BDT/STMDB empty rlist"};
	stmdbemptyrlist.prepare(RegState{0,0x777})
				   .expect(RegState{0, 0x737});
	test_run(stmdbemptyrlist);

	Test abc {TestType::InstructionARM, 0xe2688000, "RSB shift count 0"};
	abc.prepare(RegState{8,1})
		.expect(RegState{8, 0xffffffff});
	test_run(abc);


//	r.operand2 = 0b1111;
//	Test movTRP {TestType::InstructionARM, r._opcode, "MOV/Simple Reg (PC)"};
//	movT.prepare(RegState{15, 0x08000008})
//		.prepare(RegState{2, 0xDDDDDDDD})
//	    .expect(RegState{2, 0x08000000});
//	test_run(movTR);


	fmt::print("Completed {} tests, \u001b[92m{}\u001b[0m tests passed, \u001b[31m{}\u001b[0m tests failed\n",
			m_tests_run, m_tests_run - m_tests_failed, m_tests_failed);
}

void TestHarness::test_run(Test& test) {
	bool has_failed {false};
	fmt::print("Running testcase: \u001b[96m{}\u001b[0m - ", test.test_case());

	auto const& states = test.initial_states();
	for(auto& state : states) {
		test_set(state);
	}

	if(test.type() == TestType::InstructionARM)
		m_gba.cpu().execute_ARM(test.opcode());
	else
		m_gba.cpu().execute_THUMB(test.opcode());

	auto const& expectations = test.expectations();
	for(auto& expect : expectations) {
		bool pass = test_validate(expect);
		if(!pass) {
			if(!has_failed) {
				fmt::print("\u001b[31mFAILED\u001b[0m\n");
				has_failed = true;
			}

			fmt::print("\t\u001b[31mExpectation failed:\u001b[0m {}\n", expectation_format_fail(expect));
		}
	}

	m_tests_run++;
	if(has_failed) m_tests_failed++;

	if(!has_failed) {
		fmt::print("\u001b[92mPASSED\u001b[0m\n");
	}
}

void TestHarness::test_set_mem(MemState const& state) {
	switch (state.m_size) {
		case MemState::OpSize::b32: {
			m_gba.mmu().write32(state.m_address, state.m_word);
			break;
		}
		case MemState::OpSize::b16: {
			m_gba.mmu().write16(state.m_address, state.m_hword);
			break;
		}
		case MemState::OpSize::b8: {
			m_gba.mmu().write8(state.m_address, state.m_byte);
			break;
		}
		default: break;
	}
}

void TestHarness::test_set_reg(RegState const& state) {
	m_gba.cpu().reg(state.m_reg) = state.m_reg_value;
	m_gba.cpu().m_pc_dirty = false;
}

bool TestHarness::test_validate_mem(MemState const& state) {
	switch (state.m_size) {
		case MemState::OpSize::b32: return m_gba.mmu().read32(state.m_address) == state.m_word;
		case MemState::OpSize::b16: return m_gba.mmu().read16(state.m_address) == state.m_hword;
		case MemState::OpSize::b8: return m_gba.mmu().read8(state.m_address) == state.m_byte;
		default: return false;
	}
}

bool TestHarness::test_validate_reg(RegState const& state) {
	uint32 current = m_gba.cpu().creg(state.m_reg);
	return current == state.m_reg_value;
}

std::string TestHarness::expectation_format_fail(Expectation const& expectation) {
	switch (expectation.m_type) {
		case ExpectType::Register: {
			return fmt::format("r{} = {:08x}, actual value = {:08x}",
					  expectation.m_data.m_reg_expect.m_reg, expectation.m_data.m_reg_expect.m_reg_value,
					  m_gba.cpu().creg(expectation.m_data.m_reg_expect.m_reg));
		}
		case ExpectType::Memory: {
			auto const& expect = expectation.m_data.m_mem_expect;
			switch(expect.m_size) {
				case MemState::OpSize::b32: return fmt::format("word[{:08x}] = {:08x}, actual value = {:08x}", expect.m_address, expect.m_word, m_gba.mmu().read32(expect.m_address));
				case MemState::OpSize::b16: return fmt::format("hword[{:08x}] = {:04x}, actual value = {:04x}", expect.m_address, expect.m_hword, m_gba.mmu().read16(expect.m_address));
				case MemState::OpSize::b8: return fmt::format("byte[{:08x}] = {:02x}, actual value = {:02x}", expect.m_address, expect.m_byte, m_gba.mmu().read8(expect.m_address));
				default: return "";
			}
		}
		default: return "<invalid ExpectType>";
	}
}

void TestHarness::test_set_flag(FlagState const& v) {
	m_gba.cpu().cspr() = v.m_flag_reg;
}

bool TestHarness::test_validate_flag(FlagState const& v) {
	return m_gba.cpu().cspr().raw() == v.m_flag_reg.raw();
}

