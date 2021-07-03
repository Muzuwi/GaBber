#pragma once
#include <fmt/format.h>
#include "PPU/BG.hpp"
#include "Devices/VRAM.hpp"
#include "PPU/Unions.hpp"

class MMU;
class ARM7TDMI;

class PPU {
	friend class Backgrounds;

	ARM7TDMI& cpu;
	MMU& mmu;

	LCDCtl m_lcd_ctl;
	VRAM m_vram;
	PaletteRAM m_palette_ram;
	OAM m_oam;
	Backgrounds m_backgrounds;
	IOReg<0x04000130, _DummyReg<uint16>, IOAccess::R> m_keypad;
	IOReg<0x04000132, KEYCNTReg, IOAccess::RW> m_keypadcnt;

	uint32 m_framebuffer[240 * 160];
	bool m_frame_ready { false };

	void next_scanline();
	bool is_HBlank() const;
	bool is_VBlank() const;

	template<typename... Args>
	void log(const char* format, const Args& ...args) const {
		fmt::print("\u001b[35mPPU/");
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}

	static inline uint32 color_to_rgba32(Color const& color) {
		uint32 result = (color.red << 27u) | (color.green << 19u) | (color.blue << 11u) | 0xFF;
		return result;
	}

	inline Optional<OBJAttr> get_obj_attrs(uint8 obj) const {
		return m_oam.readT<OBJAttr>(obj*8);
	}

	inline Optional<Color> get_palette_color(uint8 palette_number, uint16 color) const {
		return m_palette_ram.readT<Color>(palette_number * 32 + color * 2);
	}

	inline Optional<Color> get_obj_palette_color(uint8 palette_number, uint16 color) const {
		return m_palette_ram.readT<Color>(0x200 + palette_number * 32 + color * 2);
	}

	inline Optional<TextScreenData> get_bg_text_data(uint32 screen_base, uint16 ly, uint16 dot) const {
		return m_vram.readT<TextScreenData>(screen_base + (ly/8)*0x40 + (dot/8)*2);
	}

	inline uint8 get_bg_tile_dot(uint32 base, uint16 tile, uint8 ly_in_tile, uint8 dot_in_tile, bool depth_flag) const {
		const unsigned d = depth_flag ? 1 : 2;
		const unsigned offset_to_tile = tile * (64/d);
		const unsigned offset_to_dot = ly_in_tile * (8/d) + (dot_in_tile/d);

		Optional<uint8> ret = m_vram.readT<uint8>(base + offset_to_tile + offset_to_dot);
		assert(ret.has_value());

		uint8 byte = (ret.has_value() ? *ret : 0);
		if(!depth_flag) {
			const bool is_right_pixel = (dot_in_tile % 2) != 0;
			if(is_right_pixel) byte >>= 4u;
			byte &= 0x0Fu;
		}

		return byte;
	}

	inline uint8 get_obj_tile_dot(uint16 tile, uint8 ly_in_tile, uint8 dot_in_tile, bool depth_flag) const {
		const uint32 base = (m_lcd_ctl.m_dispcnt.reg().video_mode <= 2) ? 0x00010000 : 0x00014000;
		return get_bg_tile_dot(base, tile, ly_in_tile, dot_in_tile, depth_flag);
	}

	struct Dot {
		bool dirty {};
		uint8 tile_number {};
		Color color {};
		uint8 priority {};
	};
	Dot m_colorbuffer[240] {};

	inline void colorbuffer_write(uint8 x, uint8 tile_number, uint8 priority, Color color) {
		if(x >= 240) return;
		if(tile_number == 0) return;

		//  Allow writing dots of higher priority than the currently present one, and in free spots
		if(!m_colorbuffer[x].dirty || (priority <= m_colorbuffer[x].priority)) {
			m_colorbuffer[x] = {.dirty = true, .tile_number = tile_number, .color = color, .priority = priority};
			return;
		}
	}

	uint16& vcount() { return m_lcd_ctl.m_vcount.raw(); }
	uint16 const& vcount() const { return m_lcd_ctl.m_vcount.raw(); }

	void colorbuffer_blit();
	void objects_draw_line(uint16 ly);
	void objects_draw_obj(uint16 ly, OBJAttr obj);

public:
	PPU(ARM7TDMI&, MMU&);
	void cycle();
	bool frame_ready() const { return m_frame_ready; }
	void clear_frame_ready() { m_frame_ready = false; }

	void handle_key_irq();
	void handle_key_down(KeypadKey key);
	void handle_key_up(KeypadKey key);

	LCDCtl& lcd() { return m_lcd_ctl; }
	const uint32* framebuffer() const { return m_framebuffer; }

	void draw_scanline();
};
