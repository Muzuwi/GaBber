#include <algorithm>
#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "Debugger/Debugger.hpp"
#include "Headers/GaBber.hpp"

static List<uint32> breakpoints {};

void Debugger::draw_debugger_contents() {
	make_window("Flow Control", ImGuiWindowFlags_AlwaysAutoResize, [this]{
		if(ImGui::ArrowButton("Step", ImGuiDir_Right)) {
			m_step = true;
			m_continue = false;
		}

		if(ImGui::ArrowButton("Continue", ImGuiDir_Down)) {
			m_continue = true;
		}

		if(emu.m_cpu.m_HALTCNT.m_halt) {
			ImGui::Text("HALT");
		}

		for(size_t i = 0; i < 16; ++i) {
			if(((i % 4) != 0)) ImGui::SameLine();

			auto reg = emu.m_cpu.creg(i);
			if(i == 15)
				reg -= emu.m_cpu.current_instr_len()*2;
			ImGui::Text("r%02zu: %08x", i, reg);
		}

		ImGui::Separator();
//		auto i = 0;
//		for(auto& v : emu.m_cpu.m_prefetched) {
//			ImGui::Text("Instr. %d: %08x, cycles: %d, pc: %08x", i++, v.m_opcode, v.m_cycles, v.m_pc);
//		}
		ImGui::Separator();
		ImGui::Text("CSPR: %08x", emu.m_cpu.cspr().raw());
		ImGui::Text("CSPR Interrupts: %s", !emu.m_cpu.cspr().is_set(CSPR_REGISTERS::IRQn) ? "enabled" : "disabled");
		ImGui::Text("IME: %08x", emu.m_cpu.m_IME.raw());
		ImGui::SameLine();
		if(ImGui::Button("Set")) {
			emu.m_cpu.m_IME.raw() |= 1u;
		}

		ImGui::Separator();
		ImGui::Text("%s%s%s%s",
			        emu.m_cpu.cspr().is_set(CSPR_REGISTERS::Carry) ? "C" : ".",
	                emu.m_cpu.cspr().is_set(CSPR_REGISTERS::Zero) ? "Z" : ".",
	                emu.m_cpu.cspr().is_set(CSPR_REGISTERS::Negative) ? "N" : ".",
	                emu.m_cpu.cspr().is_set(CSPR_REGISTERS::Overflow) ? "V" : "." );
	});

	make_window("MMU", 0, [this]{
		static MemoryEditor mem_edit;
		static MMU* mmu = &emu.m_mmu;

//		static unsigned fetched_at_cycle = 0;
		static uint32 address = 0;
		static uint32 prev_address = 1;
//		static Array<uint8, 16 * 32> mem_bytes;
//		//  Refresh memory bytes, they could have potentially changed
//		if(emu.m_cpu.m_cycles != fetched_at_cycle || (address != prev_address)) {
//			for(size_t i = 0; i < 16 * 32; ++i) {
//				mem_bytes[i] = emu.m_mmu.read8(address + i);
//			}
//
//			prev_address = address;
//			fetched_at_cycle = emu.m_cpu.m_cycles;
//		}
//
		static const BusDevice* selected_device {nullptr};
		if(ImGui::BeginCombo("MMU Devices", "Select device")) {
			for(const auto& dev : emu.m_mmu.s_devices) {
				const bool is_selected = static_cast<const BusDevice*>(dev) == selected_device;

				auto identity_string = fmt::format("{} [{:08x} - {:08x}]", dev->identify(), dev->start(), dev->end());

				if(ImGui::Selectable(identity_string.c_str(), is_selected)) {
					selected_device = static_cast<const BusDevice*>(dev);
					address = dev->start();
				}

				if(is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

//		ImGui::InputScalar("Address", ImGuiDataType_U32, &address, nullptr, nullptr, "%08x", ImGuiInputTextFlags_CharsHexadecimal);
//		for(size_t i = 0; i < 32; ++i) {
//			ImGui::Text("$%08lx: ", address + i*16);
//			for(size_t j = 0; j < 16; ++j) {
//				ImGui::SameLine();
//				ImGui::Text("%02x", mem_bytes[i*16 + j]);
//			}
//		}

		if(address != prev_address) {
			mem_edit.GotoAddrAndHighlight(address, address);
			prev_address = address;
		}

//		mem_edit.ReadOnly = true;
		mem_edit.ReadFn = [](const ImU8* addr, size_t off) -> ImU8 {
			return mmu->peek((uint64)addr + off);
		};
		mem_edit.WriteFn = [](ImU8* addr, size_t off, ImU8 val) {
			mmu->poke((uint64)addr + off, val);
		};
		mem_edit.DrawContents(nullptr, 0xfffffff);

	});

	make_window("Breakpoints", ImGuiWindowFlags_AlwaysAutoResize, [this]{
		static uint32 addr_field {0};

		ImGui::InputScalar("Address", ImGuiDataType_U32, &addr_field, nullptr, nullptr, "%08x", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::SameLine();
		if(ImGui::Button("Add")) {
			breakpoints.push_back(addr_field);
		}

		if(std::find(breakpoints.begin(), breakpoints.end(), emu.m_cpu.const_pc()) != breakpoints.end()) {
			fmt::print("Breakpoint reached at {:08x}", emu.m_cpu.const_pc());
			m_step = true;
			m_continue = false;
		}
	});

	make_window("Screen", ImGuiWindowFlags_AlwaysAutoResize, [this] {
		//  FIXME: Hacky cast to larger integer to avoid generating a warning
		ImGui::Image((ImTextureID)(GLuint64)emu.m_gba_texture, ImVec2(240 * emu.m_window_scale, 160 * emu.m_window_scale));
	});
}

void Debugger::make_window(const std::string& title, ImGuiWindowFlags flags, std::function<void()> handler) {
	if(!handler) return;
	ImGui::Begin(title.c_str(), nullptr, flags);
	handler();
	ImGui::End();
}

bool Debugger::handle_break() {
	const auto pc = emu.m_cpu.const_pc();
	return std::find(breakpoints.begin(), breakpoints.end(), pc-8) != breakpoints.end();
}
