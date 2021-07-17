#include "Headers/ARM7TDMI.hpp"
#include "Debugger/Debugger.hpp"

void ARM7TDMI::enter_irq() {
	log("Entering IRQ from {}", cspr().state() == INSTR_MODE::ARM ? "ARM" : "THUMB");

	const uint32 offset = cspr().state() == INSTR_MODE::ARM ? 4 : 0;
	m_registers.m_gIRQ[1] = const_pc() - offset;
	m_saved_status.m_IRQ = cspr();
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::IRQ);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	pc() = 0x18 + 8;
	m_pc_dirty = false;
}

void ARM7TDMI::enter_swi() {
	log("Entering SWI");

	m_registers.m_gSVC[1] = const_pc() - current_instr_len();
	m_saved_status.m_SVC = cspr();
	cspr().set_state(INSTR_MODE::ARM);
	cspr().set_mode(PRIV_MODE::SVC);
	cspr().set(CSPR_REGISTERS::IRQn, true);
	pc() = 0x08;
}

void ARM7TDMI::raise_irq(IRQType type) {
	auto irq_num = static_cast<unsigned>(type);
	*io.if_ |= (1u << irq_num);
}

bool ARM7TDMI::handle_halt() {
	//  FIXME: Implement stop? Does anything use this?
	if(io.haltcnt.m_stop)
		return false;

	if(io.haltcnt.m_halt) {
		if ((*io.ie & *io.if_) != 0) {
			io.haltcnt.m_halt = false;
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

	const uint16 result = *io.if_ & *io.ie;
	if(result)
		enter_irq();
}
