#pragma once
#include <fmt/format.h>
#include "MMU/MemoryLayout.hpp"

class BusInterface;
class ARM7TDMI;

class PPU {
	friend class Backgrounds;

	struct Dot {
		bool dirty;
		uint8 color_number;
		Color color;
		uint8 priority;
	};
	ARM7TDMI& cpu;
	MemoryLayout& mem;
	Backgrounds m_backgrounds;

	uint32 m_framebuffer[240 * 160];
	bool m_frame_ready { false };
	Dot m_colorbuffer[8][240] {};

	void next_scanline();
	bool is_HBlank() const;
	bool is_VBlank() const;

	template<typename... Args>
	void log(const char* format, const Args&... args) const {
		return;
		fmt::print("\u001b[35mPPU/");
		fmt::vprint(format, fmt::make_format_args(args...));
		fmt::print("\u001b[0m\n");
	}

	static inline uint32 color_to_rgba32(Color const& color) {
		uint32 result = (color.red << 27u) | (color.green << 19u) | (color.blue << 11u) | 0xFF;
		return result;
	}

	inline OBJAttr get_obj_attrs(uint8 obj) const {
		return mem.oam.readT<OBJAttr>(obj * 8);
	}

	inline Color get_palette_color(uint8 palette_number, uint16 color) const {
		return mem.palette.readT<Color>(palette_number * 32 + color * 2);
	}

	inline Color get_obj_palette_color(uint8 palette_number, uint16 color) const {
		return mem.palette.readT<Color>(0x200 + palette_number * 32 + color * 2);
	}

	inline TextScreenData get_bg_text_data(uint32 screen_base, uint16 ly, uint16 dot) const {
		return mem.vram.readT<TextScreenData>(screen_base + (ly / 8) * 0x40 + (dot / 8) * 2);
	}

	inline uint8 get_bg_tile_dot(uint32 base, uint16 tile, uint8 ly_in_tile, uint8 dot_in_tile, bool depth_flag) const {
		const unsigned d = depth_flag ? 1 : 2;
		const unsigned offset_to_tile = tile * (64 / d);
		const unsigned offset_to_dot = ly_in_tile * (8 / d) + (dot_in_tile / d);

		uint8 byte = mem.vram.readT<uint8>(base + offset_to_tile + offset_to_dot);
		if(!depth_flag) {
			const bool is_right_pixel = (dot_in_tile % 2) != 0;
			if(is_right_pixel) byte >>= 4u;
			byte &= 0x0Fu;
		}

		return byte;
	}

	inline uint8 get_obj_tile_dot(uint16 tile, uint8 ly_in_tile, uint8 dot_in_tile, bool depth_flag) const {
		const uint32 base = 0x00010000;

		const unsigned d = depth_flag ? 1 : 2;
		const unsigned offset_to_tile = tile * 32;
		const unsigned offset_to_dot = ly_in_tile * (8 / d) + (dot_in_tile / d);

		uint8 byte = mem.vram.readT<uint8>(base + offset_to_tile + offset_to_dot);
		if(!depth_flag) {
			const bool is_right_pixel = (dot_in_tile % 2) != 0;
			if(is_right_pixel) byte >>= 4u;
			byte &= 0x0Fu;
		}

		return byte;
	}

	inline void colorbuffer_write_bg(uint8 x, uint8 color_number, uint8 priority, Color const& color) {
		if(x >= 240) {
			return;
		}

		if(color_number == 0) {
			return;
		}

		if(m_colorbuffer[2 * priority + 1][x].dirty) {
			return;
		}

		m_colorbuffer[2 * priority + 1][x] = {
			.dirty = true,
			.color_number = color_number,
			.color = color,
			.priority = priority,
		};
	}

	inline void colorbuffer_write_obj(uint8 x, uint8 color_number, uint8 priority, Color const& color) {
		if(x >= 240) {
			return;
		}

		if(color_number == 0) {
			return;
		}

		if(m_colorbuffer[2 * priority][x].dirty) {
			return;
		}

		m_colorbuffer[2 * priority][x] = {
			.dirty = true,
			.color_number = color_number,
			.color = color,
			.priority = priority,
		};
	}

	uint16& vcount() { return *mem.io.vcount; }
	uint16 const& vcount() const { return *mem.io.vcount; }

	void colorbuffer_blit();
	void objects_draw_line(uint16 ly);
	void objects_draw_obj(uint16 ly, OBJAttr obj);
public:
	PPU(ARM7TDMI&, MemoryLayout&);
	void cycle();
	bool frame_ready() const { return m_frame_ready; }
	void clear_frame_ready() { m_frame_ready = false; }

	void handle_key_irq();
	void handle_key_down(KeypadKey key);
	void handle_key_up(KeypadKey key);

	const uint32* framebuffer() const { return m_framebuffer; }

	void draw_scanline();
};
