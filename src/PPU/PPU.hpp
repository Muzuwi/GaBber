#pragma once
#include <fmt/format.h>
#include "Emulator/Module.hpp"
#include "Emulator/StdTypes.hpp"
#include "PPU/BG.hpp"

enum class KeypadKey;

class PPU : Module {
	friend class Backgrounds;

	struct Dot {
		bool dirty;
		uint8 color_number;
		Color color;
		uint8 priority;
	};
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

	static constexpr inline uint32 color_to_rgba32(Color const& color) {
		uint32 result = (color.red << 27u) | (color.green << 19u) | (color.blue << 11u) | 0xFF;
		return result;
	}

	static constexpr inline uint32 obj_attr_offset(uint8 obj) { return obj * 8; }

	static constexpr inline uint32 palette_color_offset(uint8 palette_number, uint16 color) {
		return palette_number * 32 + color * 2;
	}

	static constexpr inline uint32 obj_palette_color_offset(uint8 palette_number, uint16 color) {
		return 0x200 + palette_number * 32 + color * 2;
	}

	static constexpr inline uint32 bg_text_data_offset(uint32 screen_base, uint16 ly, uint16 dot) {
		return screen_base + (ly / 8) * 0x40 + (dot / 8) * 2;
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

	uint16& vcount();
	uint16 const& vcount() const;

	void colorbuffer_blit();
	void objects_draw_line(uint16 ly);
	void objects_draw_obj(uint16 ly, OBJAttr obj);
public:
	PPU(GaBber&);
	void cycle();
	bool frame_ready() const { return m_frame_ready; }
	void clear_frame_ready() { m_frame_ready = false; }

	void handle_key_irq();
	void handle_key_down(KeypadKey key);
	void handle_key_up(KeypadKey key);

	const uint32* framebuffer() const { return m_framebuffer; }

	void draw_scanline();
};
