#include "Headers/ARM7TDMI.hpp"

void ARM7TDMI::enter_irq() {
	m_last_mode_change.cycle = m_cycles;
	m_last_mode_change.pc = const_pc() - 2*current_instr_len();
	m_last_mode_change.prev = cspr().state();
	m_last_mode_change.reason = "Exception - IRQ";
	m_last_mode_change.neu = INSTR_MODE::ARM;

	m_registers.m_gIRQ[1] = const_pc() - current_instr_len();
	m_saved_status.m_IRQ = cspr();
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::IRQ);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	pc() = 0x18 + 8;
	m_pc_dirty = false;
}

void ARM7TDMI::enter_swi() {
	m_last_mode_change.cycle = m_cycles;
	m_last_mode_change.pc = const_pc() - 2*current_instr_len();
	m_last_mode_change.prev = cspr().state();
	m_last_mode_change.reason = "Exception - SWI";
	m_last_mode_change.neu = INSTR_MODE::ARM;

	m_registers.m_gSVC[1] = const_pc() - current_instr_len();
	m_saved_status.m_SVC = cspr();
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::SVC);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	pc() = 0x08 + 8;
	m_pc_dirty = false;
}

void ARM7TDMI::raise_irq(IRQType type) {
	auto irq_num = static_cast<unsigned>(type);
	m_IF.raw() |= (1u << irq_num);
}

bool ARM7TDMI::handle_halt() {
	//  FIXME: Implement stop? Does anything use this?
	if(m_HALTCNT.m_stop)
		return false;

	if(m_HALTCNT.m_halt) {
		if ((m_IE.raw() & m_IF.raw()) != 0) {
			m_HALTCNT.m_halt = false;
			return false;
		} else {
			return true;
		}
	}

	return false;
}

void ARM7TDMI::handle_interrupts() {
	if (!irqs_enabled_globally())
		return;

	const uint16 result = m_IF.raw() & m_IE.raw();
	if(result)
		enter_irq();
}
