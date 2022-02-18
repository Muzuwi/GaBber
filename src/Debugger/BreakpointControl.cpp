#include <imgui_internal.h>
#include "Debugger/Debugger.hpp"
#include "Debugger/WindowDefinitions.hpp"
#include "Emulator/GaBber.hpp"

void BreakpointControl::draw_window() {
	auto& flag = m_emu.debugger().m_break_on_undefined;
	ImGui::Checkbox("Break on undefined access", &flag);
	ImGui::InputScalar("Address", ImGuiDataType_U32, &m_break.start, nullptr, nullptr, "%08x",
	                   ImGuiInputTextFlags_CharsHexadecimal);

	bool R = m_break.type & BreakRead;
	bool W = m_break.type & BreakWrite;
	bool X = m_break.type & BreakExec;
	if(ImGui::Checkbox("R", &R)) {
		m_break.type = static_cast<BreakpointType>((m_break.type & ~BreakRead) |
		                                           ((m_break.type & BreakRead) ? 0x0 : BreakRead));
	}
	ImGui::SameLine();
	if(ImGui::Checkbox("W", &W)) {
		m_break.type = static_cast<BreakpointType>((m_break.type & ~BreakWrite) |
		                                           ((m_break.type & BreakWrite) ? 0x0 : BreakWrite));
	}
	ImGui::SameLine();
	if(ImGui::Checkbox("X", &X)) {
		m_break.type = static_cast<BreakpointType>((m_break.type & ~BreakExec) |
		                                           ((m_break.type & BreakExec) ? 0x0 : BreakExec));
	}
	ImGui::SameLine();

	const bool valid = R || W || X;
	if(!valid) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	if(ImGui::Button("Add", ImVec2(-FLT_MIN, 0))) {
		//  FIXME:
		m_break.size = 1;
		m_emu.debugger().m_breakpoints.push_back(m_break);
	}
	if(!valid) {
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	//  Breakpoint list
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildR", ImVec2(0, 320), true);

		unsigned n = 0;
		for(auto& breakpoint : m_emu.debugger().m_breakpoints) {
			ImGui::Text("0x%08x - 0x%08x [%c%c%c]", breakpoint.start, breakpoint.start + breakpoint.size - 1,
			            (breakpoint.type & BreakRead) ? 'R' : '-', (breakpoint.type & BreakWrite) ? 'W' : '-',
			            (breakpoint.type & BreakExec) ? 'X' : '-');
			ImGui::SameLine();

			ImGui::PushID(n++);
			if(ImGui::Button("Remove", ImVec2(-FLT_MIN, 0.0f))) {
				m_emu.debugger().m_breakpoints.erase(m_emu.debugger().m_breakpoints.begin() + n);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}