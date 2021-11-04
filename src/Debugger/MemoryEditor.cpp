#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"
#include "MMU/BusInterface.hpp"

void MemEditor::draw_window() {
	static BusInterface& mmu = this->m_emu.mmu();
	static uint32 start = 0;
	static uint32 size  = 0x4000;

	ImGui::SetWindowSize(ImVec2(720.0f, 600.0f));

	if(ImGui::BeginTabBar("ioreg_tabs")) {
		if(ImGui::BeginTabItem("BIOS")) {
			start = 0;
			size = 0x4000;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("WRAM")) {
			start = 0x02000000;
			size = 0x40000;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("IWRAM")) {
			start = 0x03000000;
			size = 0x8000;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("I/O")) {
			start = 0x04000000;
			size = 0x400;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Palette")) {
			start = 0x05000000;
			size = 0x400;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("VRAM")) {
			start = 0x06000000;
			size = 0x18000;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("OAM")) {
			start = 0x07000000;
			size = 0x400;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("ROM")) {
			start = 0x08000000;
			size = 0x02000000;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("SRAM")) {
			start = 0x0e000000;
			size = 0x10000;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	m_editor.ReadFn = [](const ImU8* addr, size_t off) -> ImU8 {
		return mmu.peek((uint64)addr + off);
	};
	m_editor.WriteFn = [](ImU8* addr, size_t off, ImU8 val) {
		mmu.poke((uint64)addr + off, val);
	};
	m_editor.DrawContents((void*)start, size, start);
}

