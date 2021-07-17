#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"
#include "Headers/ARM7TDMI.hpp"

void IORegisters::draw_window() {
	if(ImGui::BeginTabBar("ioreg_tabs")) {
		if(ImGui::BeginTabItem("Interrupts")) {
			m_which_tab = WindowTab::Interrupts;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("DMA")) {
			m_which_tab = WindowTab::DMA;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("PPU")) {
			m_which_tab = WindowTab::PPU;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Sound")) {
			m_which_tab = WindowTab::Sound;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	switch (m_which_tab) {
		case WindowTab::Interrupts: this->draw_interrupts(); break;
		case WindowTab::PPU: this->draw_ppu(); break;
		case WindowTab::Sound: this->draw_sound(); break;
		case WindowTab::DMA: this->draw_dma(); break;
	}
}

void IORegisters::draw_interrupts() {
	auto& cpu = m_emu.cpu();
	ImGui::Text("IF: %04x", *cpu.io.if_);
	ImGui::Text("IE: %04x", *cpu.io.ie);
	ImGui::Text("IME: %08x", *cpu.io.ime);

}

void IORegisters::draw_ppu() {
	ImGui::Text("PPU");
}

void IORegisters::draw_sound() {
	ImGui::Text("Sound");
}

void IORegisters::draw_dma() {
	auto& cpu = m_emu.cpu();
	DMAx<0> const& dma0 = cpu.io.dma0;
	DMAx<1> const& dma1 = cpu.io.dma1;
	DMAx<2> const& dma2 = cpu.io.dma2;
	DMAx<3> const& dma3 = cpu.io.dma3;


	auto draw_dma_stats = [](auto& dma) {
		ImGui::Text("Source: %08x", *dma.m_source);
		ImGui::Text("Dest: %08x", *dma.m_destination);
		ImGui::Text("Words: %d", dma.m_ctrl->word_count);
	};

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0, 5.0));
	if(ImGui::BeginTable("dmatab", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("DMA0");
		ImGui::TableNextColumn(); ImGui::TableHeader("DMA1");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		draw_dma_stats(dma0);
		ImGui::TableNextColumn();
		draw_dma_stats(dma1);

		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("DMA2");
		ImGui::TableNextColumn(); ImGui::TableHeader("DMA3");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		draw_dma_stats(dma2);
		ImGui::TableNextColumn();
		draw_dma_stats(dma3);

		ImGui::EndTable();
	}
	ImGui::PopStyleVar(1);

}
