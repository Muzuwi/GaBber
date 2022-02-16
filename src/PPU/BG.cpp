#include "PPU/BG.hpp"
#include <optional>
#include "PPU/PPU.hpp"

template<unsigned n>
constexpr BG<n>& Backgrounds::current_bg() {
	static_assert(n < 4, "Invalid BG number");
	if constexpr(n == 0)
		return m_ppu.mem.io.bg0;
	else if constexpr(n == 1)
		return m_ppu.mem.io.bg1;
	else if constexpr(n == 2)
		return m_ppu.mem.io.bg2;
	else
		return m_ppu.mem.io.bg3;
}

template<unsigned int n>
void Backgrounds::draw_textmode() {
	BG<n> const& bg = current_bg<n>();
	if(!bg_enabled<n>())
		return;

	//	assert(bg.m_control->screen_size == 0);
	//	assert(bg.m_control->mosaic == 0);

	const uint32 screen_base = bg.m_control->base_screen_block * 2 * kB;
	const uint32 tile_base = bg.m_control->base_tile_block * 16 * kB;
	const uint8 priority = bg.m_control->priority;
	const bool depth = bg.m_control->palette_flag;
	const unsigned vscreen_width = (bg.m_control->screen_size & 1u) ? 512u : 256u;
	const unsigned vscreen_height = (bg.m_control->screen_size & 2u) ? 512u : 256u;
	const unsigned vscreen_screensx = vscreen_width / 256u;
	const unsigned vscreen_screensy = vscreen_height / 256u;

	const auto scx = *bg.m_xoffset;
	const auto scy = *bg.m_yoffset;
	const auto ly = (scy + m_ppu.vcount()) % vscreen_height;
	const unsigned vscreen_y = (ly / 256u) % vscreen_screensy;

	for(unsigned i = scx; i < scx + 240u; ++i) {
		const unsigned x = i % vscreen_width;
		const unsigned vscreen_x = (x / 256u) % vscreen_screensx;

		const unsigned which_vscreen = vscreen_screensx * vscreen_y + vscreen_x;
		const uint32 vscreen_base = screen_base + which_vscreen * 0x800;

		const std::optional<TextScreenData> v = m_ppu.get_bg_text_data(vscreen_base, ly % 256u, x % 256u);
		assert(v.has_value());

		const TextScreenData text_data = *v;
		const bool xflip = text_data.m_struct.horizontal_flip;
		const bool yflip = text_data.m_struct.vertical_flip;
		const uint16 tile = text_data.m_struct.tile_number;
		const uint16 palette = text_data.m_struct.palette_number;

		const uint8 tile_dot = xflip ? (7 - (x % 8))
		                             : (x % 8);
		const uint8 tile_line = yflip ? (7 - (ly % 8))
		                              : (ly % 8);
		const uint8 dot_color = m_ppu.get_bg_tile_dot(tile_base, tile, tile_line, tile_dot, depth);

		const std::optional<Color> color = m_ppu.get_palette_color(depth ? 0 : palette, dot_color);
		assert(color.has_value());

		m_ppu.colorbuffer_write_bg(i - scx, dot_color, priority, *color);
	}
}

void Backgrounds::draw_mode0() {
	draw_textmode<0>();
	draw_textmode<1>();
	draw_textmode<2>();
	draw_textmode<3>();
}

void Backgrounds::draw_mode1() {
	//	ASSERT_NOT_REACHED();
}

void Backgrounds::draw_mode2() {
	//	ASSERT_NOT_REACHED();
}

void Backgrounds::draw_mode3() {
	const auto& ctl = m_ppu.mem.io.dispcnt;

	if(ctl->BG2) {
		const auto ly = *m_ppu.mem.io.vcount;
		const auto line_offset = ly * 480;//  480 bytes per line

		for(unsigned x = 0; x < 240; ++x) {
			const auto& color = (Color)m_ppu.mem.vram.read16(line_offset + x * 2);
			m_ppu.colorbuffer_write_bg(x, 1, 0, color);
		}
	}
}

void Backgrounds::draw_mode4() {
	const auto& ctl = m_ppu.mem.io.dispcnt;

	if(ctl->BG2) {
		const auto ly = *m_ppu.mem.io.vcount;
		const auto frame_offset = (ctl->frame_select ? 0xA000 : 0);

		for(unsigned i = 0; i < 240; ++i) {
			const auto line_offset = ly * 240;
			const auto pixel = m_ppu.mem.vram.read8(frame_offset + line_offset + i);
			const std::optional<Color> color = m_ppu.get_palette_color(0, pixel);
			assert(color.has_value());

			m_ppu.colorbuffer_write_bg(i, 1, 0, *color);
		}
	}
}

void Backgrounds::draw_mode5() {
	const auto& ctl = m_ppu.mem.io.dispcnt;

	if(ctl->BG2) {
		const auto ly = *m_ppu.mem.io.vcount;
		const auto frame_offset = (ctl->frame_select ? 0xA000 : 0);

		for(unsigned i = 0; i < 240; ++i) {
			const auto line_offset = ly * 240;
			const auto pixel = m_ppu.mem.vram.read8(frame_offset + line_offset + i);
			const std::optional<Color> color = m_ppu.get_palette_color(0, pixel);

			assert(color.has_value());
			m_ppu.colorbuffer_write_bg(i, 1, 0, *color);
		}
	}
}

void Backgrounds::draw_scanline() {
	const auto& ctl = m_ppu.mem.io.dispcnt;

	switch(ctl->video_mode) {
		case 0: draw_mode0(); break;
		case 1: draw_mode1(); break;
		case 2: draw_mode2(); break;
		case 3: draw_mode3(); break;
		case 4: draw_mode4(); break;
		case 5: draw_mode5(); break;
		default: {
			m_ppu.log("Invalid mode={}");
			break;
		}
	}
}

template<unsigned int n>
constexpr bool Backgrounds::bg_enabled() {
	if constexpr(n == 0)
		return m_ppu.mem.io.dispcnt->BG0;
	else if constexpr(n == 1)
		return m_ppu.mem.io.dispcnt->BG1;
	else if constexpr(n == 2)
		return m_ppu.mem.io.dispcnt->BG2;
	else
		return m_ppu.mem.io.dispcnt->BG3;
}
