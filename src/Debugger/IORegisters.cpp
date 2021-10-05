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
	ImGui::Text("Wait cycles: %d\n", cpu.m_wait_cycles);

}

void IORegisters::draw_ppu() {
	ImGui::Text("PPU");
}

void IORegisters::draw_sound() {
	auto& io = m_emu.mem().io;

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0, 5.0));
	if(ImGui::BeginTable("soundtab", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("Channel 1");
		ImGui::TableNextColumn(); ImGui::TableHeader("Channel 2");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			auto freq_hz = 131072u / (2048u - io.ch1ctlX->frequency);
			//  Draw ch0
			ImGui::Text("Frequency: %04x [%d Hz]", io.ch1ctlX->frequency, freq_hz);
		}
		ImGui::TableNextColumn();
		{
			auto freq_hz = 131072u / (2048u - io.ch2ctlH->frequency);
			//  Draw ch0
			ImGui::Text("Frequency: %04x [%d Hz]", io.ch2ctlH->frequency, freq_hz);
			//  Draw ch1
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("Channel 3");
		ImGui::TableNextColumn(); ImGui::TableHeader("Channel 4");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			//  Draw ch2
		}
		ImGui::TableNextColumn();
		{
			const unsigned sample_rate = 2097152 / (2048 - io.ch3ctlX->rate);
			ImGui::Text("Sample rate: %d Hz", sample_rate);
			//  Draw ch3
		}

		ImGui::EndTable();
	}
	ImGui::PopStyleVar(1);

	const char* strings[] = {
			"1", "2", "3", "4"
	};
	ImGui::Text("Channels [L]: "); ImGui::SameLine();
	for(unsigned i = 0; i < 4; ++i) {
		bool v = io.soundcntL->channel_enable_l & (1 << i);
		ImGui::Checkbox(strings[i], &v);
		if(i != 3) ImGui::SameLine();
	}
	ImGui::Text("Channels [R]: "); ImGui::SameLine();
	for(unsigned i = 0; i < 4; ++i) {
		bool v = io.soundcntL->channel_enable_r & (1 << i);
		ImGui::Checkbox(strings[i], &v);
		if(i != 3) ImGui::SameLine();
	}

	int vol_l = io.soundcntL->volume_l;
	int vol_r = io.soundcntL->volume_r;
	ImGui::Text("Volume [L]"); ImGui::SameLine();
	ImGui::PushID("volL");
	ImGui::SliderInt("", &vol_l, 0, 7);
	ImGui::PopID();

	ImGui::Text("Volume [R]"); ImGui::SameLine();
	ImGui::PushID("volR");
	ImGui::SliderInt("", &vol_r, 0, 7);
	ImGui::PopID();

	ImGui::PlotLines("Internal samples @ 262.144kHz",
					 &GaBber::instance().sound().internal_samples()[0],
					 GaBber::instance().sound().internal_samples().size(),
					 0,
					 nullptr,
					 -1.0,
					 1.0, ImVec2(800.0, 100.0));

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
