#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"

void Breakpoints::draw_window() {
	auto& flag = m_emu.debugger().m_break_on_undefined;
	ImGui::Checkbox("Break on undefined access", &flag);

	ImGui::InputScalar("Address",
					   ImGuiDataType_U32, &m_break_address, nullptr, nullptr, "%08x", ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::SameLine();
	if(ImGui::Button("Add")) {
		m_emu.debugger().m_execution_breakpoints.push_back(m_break_address);
	}
}