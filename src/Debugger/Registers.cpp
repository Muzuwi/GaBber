#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"
#include "CPU/ARM7TDMI.hpp"

void GPRs::draw_window() {
	auto& cpu = m_emu.cpu();

	for(size_t i = 0; i < 16; ++i) {
		if(((i % 4) != 0)) ImGui::SameLine();

		uint32 reg = cpu.creg(i);
		if(i == 15)
			reg -= cpu.current_instr_len()*2;
		ImGui::Text("r%02zu: %08x", i, reg);
	}

	ImGui::Separator();
	bool Z = cpu.cspr().is_set(CSPR_REGISTERS::Zero),
		 N = cpu.cspr().is_set(CSPR_REGISTERS::Negative),
		 C = cpu.cspr().is_set(CSPR_REGISTERS::Carry),
		 V = cpu.cspr().is_set(CSPR_REGISTERS::Overflow);
	{
		ImGui::Text("Flags: "); ImGui::SameLine(); ImGui::Dummy(ImVec2(10.0, 0.0)); ImGui::SameLine();
		ImGui::Checkbox("Z", &Z); ImGui::SameLine();
		ImGui::Checkbox("N", &N); ImGui::SameLine();
		ImGui::Checkbox("C", &C); ImGui::SameLine();
		ImGui::Checkbox("V", &V);
	}

	bool I = cpu.cspr().is_set(CSPR_REGISTERS::IRQn),
		 T = cpu.cspr().is_set(CSPR_REGISTERS::State);
	{
		ImGui::Text("Control: "); ImGui::SameLine(); ImGui::Dummy(ImVec2(10.0, 0.0)); ImGui::SameLine();
		ImGui::Checkbox("I", &I); ImGui::SameLine();
		ImGui::Checkbox("T", &T);
	}
	ImGui::Text("Mode: %s", cpu.cspr().mode_str());
	ImGui::Text("CPSR: %08x", cpu.cspr().raw());
}