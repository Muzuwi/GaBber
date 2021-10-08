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

	static constexpr char const* labels[14] = {
		"VBlank", "HBlank", "VCounter", "Timer 0",
		"Timer 1", "Timer 2", "Timer 3", "Serial",
		"DMA0", "DMA1", "DMA2", "DMA3",
		"Keypad", "Game Pak"
	};

	bool if_values[14] = {};
	bool ie_values[14] = {};
	for(unsigned i = 0; i < 14; ++i) {
		if_values[i] = (*cpu.io.if_) & (1u << i);
		ie_values[i] = (*cpu.io.ie) & (1u << i);
	}

	ImGui::NewLine();
	ImGui::SameLine(106.0f); ImGui::Text("IE");
	ImGui::SameLine(156.0f); ImGui::Text("IF");
	for(unsigned i = 0; i < 14; ++i) {
		ImGui::Text("%s", labels[i]);
		ImGui::SameLine(100.0f);

		auto str_ie = "##ie" + std::to_string(i);
		auto str_if = "##if" + std::to_string(i);

		ImGui::Checkbox(str_ie.c_str(), &ie_values[i]);
		ImGui::SameLine(150.0f);
		ImGui::Checkbox(str_if.c_str(), &if_values[i]);
	}

	bool ime = *cpu.io.ime & 1u;
	ImGui::Text("IME");
	ImGui::SameLine(100.0f);
	ImGui::Checkbox("##ime", &ime);
	ImGui::Text("Wait cycles: %d\n", cpu.m_wait_cycles);

	ImGui::PlotLines("Cycle count per state update",
	                 [](void* data, int index) -> float {
		                 return static_cast<float>(reinterpret_cast<unsigned*>(data)[index]);
	                 },
	                 (void*)&GaBber::instance().cycle_samples()[0],
	                 GaBber::instance().cycle_samples().size(),
					 0,
					 nullptr,
					 0,
					 100,
					 ImVec2(800.0, 200.0));
}

void IORegisters::draw_ppu() {
	//  Because layout in ImGui requires a lot of boilerplate, these
	//  are at the top to make it easier to change them
	auto draw_dispcnt = [this] {
		auto& ctl = m_emu.mem().io.dispcnt;

		ImGui::Text("BG Mode: %d\n", ctl->video_mode);
		ImGui::Text("Frame Select: %d\n", ctl->frame_select);
		ImGui::Text("OAM Access: %d\n", ctl->oam_in_HBlank);
		ImGui::Text("OBJ Mapping: %s", ctl->obj_one_dim ? "One-dimensional" : "Two-dimensional");
		ImGui::Text("Forced blank: %d", ctl->forced_blank);
		bool values[5] = {
				ctl->BG0,
				ctl->BG1,
				ctl->BG2,
				ctl->BG3,
				ctl->OBJ
		};
		ImGui::Checkbox("BG0", &values[0]); ImGui::SameLine();
		ImGui::Checkbox("BG1", &values[1]); ImGui::SameLine();
		ImGui::Checkbox("BG2", &values[2]); ImGui::SameLine();
		ImGui::Checkbox("BG3", &values[3]); ImGui::SameLine();
		ImGui::Checkbox("OBJ", &values[4]);

		bool windowValues[3] = {
				ctl->window0,
				ctl->window1,
				ctl->objWindow
		};
		ImGui::Checkbox("Window 0", &windowValues[0]); ImGui::SameLine();
		ImGui::Checkbox("Window 1", &windowValues[1]); ImGui::SameLine();
		ImGui::Checkbox("OBJ Window", &windowValues[2]);
	};
	auto draw_dispstat = [this] {
		auto& stat = m_emu.mem().io.dispstat;
		bool values[] = {
				stat->VBlank,
				stat->HBlank,
				stat->VCounter,
				stat->VBlank_IRQ,
				stat->HBlank_IRQ,
				stat->VCounter_IRQ
		};

		ImGui::Checkbox("VBlank", &values[0]); ImGui::SameLine();
		ImGui::Checkbox("IRQ", &values[3]);
		ImGui::Checkbox("HBlank", &values[1]); ImGui::SameLine();
		ImGui::Checkbox("IRQ", &values[4]);
		ImGui::Checkbox("VCounter", &values[2]); ImGui::SameLine();
		ImGui::Checkbox("IRQ", &values[5]);
		ImGui::Text("LYC: %d", stat->VCounter);
		ImGui::Text("LY: %d", *m_emu.mem().io.vcount);
	};
	auto draw_bg0 = [this] {
		auto& bg = m_emu.mem().io.bg0;
		ImGui::Text("Priority: %d", bg.m_control->priority);
		ImGui::Text("Tile base: %08x", 0x06000000 + bg.m_control->base_tile_block * 2 * kB);
		ImGui::Text("Screen base: %08x", 0x06000000 + bg.m_control->base_screen_block * 16 * kB);
		ImGui::Text("Screen size: %dx%d",
		            bg.m_control->screen_size & 1u ? 512u : 256u,
		            bg.m_control->screen_size & 2u ? 512u : 256u);
		bool color = bg.m_control->palette_flag;
		bool mosaic = bg.m_control->palette_flag;
		ImGui::Checkbox("Depth flag", &color);
		ImGui::SameLine();
		ImGui::Checkbox("Mosaic", &color);
	};
	auto draw_bg1 = [this] {
		auto& bg = m_emu.mem().io.bg1;
		ImGui::Text("Priority: %d", bg.m_control->priority);
		ImGui::Text("Tile base: %08x", 0x06000000 + bg.m_control->base_tile_block * 2 * kB);
		ImGui::Text("Screen base: %08x", 0x06000000 + bg.m_control->base_screen_block * 16 * kB);
		ImGui::Text("Screen size: %dx%d",
		            bg.m_control->screen_size & 1u ? 512u : 256u,
		            bg.m_control->screen_size & 2u ? 512u : 256u);
		bool color = bg.m_control->palette_flag;
		bool mosaic = bg.m_control->palette_flag;
		ImGui::Checkbox("Depth flag", &color);
		ImGui::SameLine();
		ImGui::Checkbox("Mosaic", &color);
	};
	auto draw_bg2 = [this] {
		auto& bg = m_emu.mem().io.bg2;
		ImGui::Text("Priority: %d", bg.m_control->priority);
		ImGui::Text("Tile base: %08x", 0x06000000 + bg.m_control->base_tile_block * 2 * kB);
		ImGui::Text("Screen base: %08x", 0x06000000 + bg.m_control->base_screen_block * 16 * kB);
		ImGui::Text("Screen size: %dx%d",
		            bg.m_control->screen_size & 1u ? 512u : 256u,
		            bg.m_control->screen_size & 2u ? 512u : 256u);
		bool color = bg.m_control->palette_flag;
		bool mosaic = bg.m_control->palette_flag;
		ImGui::Checkbox("Depth flag", &color);
		ImGui::SameLine();
		ImGui::Checkbox("Mosaic", &color);
	};
	auto draw_bg3 = [this] {
		auto& bg = m_emu.mem().io.bg3;
		ImGui::Text("Priority: %d", bg.m_control->priority);
		ImGui::Text("Tile base: %08x", 0x06000000 + bg.m_control->base_tile_block * 2 * kB);
		ImGui::Text("Screen base: %08x", 0x06000000 + bg.m_control->base_screen_block * 16 * kB);
		ImGui::Text("Screen size: %dx%d",
		            bg.m_control->screen_size & 1u ? 512u : 256u,
		            bg.m_control->screen_size & 2u ? 512u : 256u);
		bool color = bg.m_control->palette_flag;
		bool mosaic = bg.m_control->palette_flag;
		ImGui::Checkbox("Depth flag", &color);
		ImGui::SameLine();
		ImGui::Checkbox("Mosaic", &color);
	};

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0, 5.0));
	if(ImGui::BeginTable("dispcntstat", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("DISPCNT");
		ImGui::TableNextColumn(); ImGui::TableHeader("DISPSTAT");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			draw_dispcnt();
		}
		ImGui::TableNextColumn();
		{
			draw_dispstat();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleVar(1);

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0, 5.0));
	if(ImGui::BeginTable("pputab", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("BG0");
		ImGui::TableNextColumn(); ImGui::TableHeader("BG1");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			draw_bg0();
		}
		ImGui::TableNextColumn();
		{
			draw_bg1();
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::TableHeader("BG2");
		ImGui::TableNextColumn(); ImGui::TableHeader("BG3");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			draw_bg2();
		}
		ImGui::TableNextColumn();
		{
			draw_bg3();
		}

		ImGui::EndTable();
	}
	ImGui::PopStyleVar(1);

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



	auto draw_dma_stats = []<const unsigned n>(DMAx<n> const& dma) {
		bool active = dma.m_is_running;
		ImGui::Checkbox("Active", &active);
		bool enabled = dma.m_ctrl->enable;
		ImGui::SameLine();
		ImGui::Checkbox("Enabled", &enabled);
		bool repeat = dma.m_ctrl->repeat;
		ImGui::Checkbox("Repeat", &repeat);
		ImGui::SameLine();
		bool irq_on_end = dma.m_ctrl->irq_on_finish;
		ImGui::Checkbox("IRQ on finish", &irq_on_end);
		ImGui::Text("Transfer type: %s", dma.m_ctrl->transfer_size ? "32bit" : "16bit");

		auto timing = dma.m_ctrl->start_timing;
		auto timing_str = [](DMAStartTiming timing) -> const char* {
			switch (timing) {
				case DMAStartTiming::HBlank:
					return "HBlank";
				case DMAStartTiming::VBlank:
					return "VBlank";
				case DMAStartTiming::Immediate:
					return "Immediate";
				case DMAStartTiming::Special:
				default:
					return "Special";
			}
		};

		ImGui::Text("Start timing: %s", timing_str(timing));

		ImGui::Text("Source: %08x", *dma.m_source);
		ImGui::Text("Destination: %08x", *dma.m_destination);
		ImGui::Text("Word count: %d", dma.m_ctrl->word_count);

		ImGui::Text("Internal Source: %08x", dma.m_source_ptr);
		ImGui::Text("Internal Destination: %08x", dma.m_destination_ptr);

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
