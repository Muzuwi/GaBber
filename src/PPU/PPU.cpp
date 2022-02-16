#include "PPU/PPU.hpp"
#include <optional>
#include "CPU/ARM7TDMI.hpp"
#include "MMU/BusInterface.hpp"

PPU::PPU(ARM7TDMI& a, MemoryLayout& mem_)
    : cpu(a)
    , mem(mem_)
    , m_backgrounds(*this) {
}

bool PPU::is_HBlank() const {
	return mem.io.dispstat->HBlank && vcount() >= 160;
}

bool PPU::is_VBlank() const {
	return vcount() >= 160 && vcount() <= 227;
}

static unsigned global_cycles { 0 };
static unsigned current_scanline_position { 0 };

void PPU::next_scanline() {
	current_scanline_position = 0;
	vcount()++;

	if(vcount() == mem.io.dispstat->LYC) {
		mem.io.dispstat->VCounter = true;
		if(mem.io.dispstat->VCounter_IRQ) {
			cpu.raise_irq(IRQType::VCounter);
		}
	} else {
		mem.io.dispstat->VCounter = false;
	}

	mem.io.dispstat->HBlank = false;

	if(vcount() == 160) {
		mem.io.dispstat->VBlank = true;
		cpu.dma_start_vblank();
		if(mem.io.dispstat->VBlank_IRQ) {
			cpu.raise_irq(IRQType::VBlank);
		}
		m_frame_ready = true;
	} else if(vcount() == 228) {
		mem.io.dispstat->VBlank = false;
		vcount() = 0;
	}
}

void PPU::cycle() {
	global_cycles++;

	static unsigned pixel_cycles { 0 };
	if(++pixel_cycles != 4)
		return;

	pixel_cycles = 0;
	current_scanline_position++;

	if(current_scanline_position == 240 && !is_VBlank()) {
		mem.io.dispstat->HBlank = true;
		cpu.dma_start_hblank();
		if(mem.io.dispstat->HBlank_IRQ) {
			cpu.raise_irq(IRQType::HBlank);
		}

		draw_scanline();
	} else if(current_scanline_position == 308) {
		next_scanline();
	}
}

void PPU::draw_scanline() {
	if(mem.io.dispcnt->forced_blank) {
		for(unsigned x = 0; x < 240; ++x) {
			m_framebuffer[vcount() * 240 + x] = 0xFFFFFFFF;
		}
		return;
	}

	m_backgrounds.draw_scanline();
	objects_draw_line(vcount());
	colorbuffer_blit();
}

void PPU::handle_key_down(KeypadKey key) {
	mem.io.keyinput.set(key, Keypad::State::Pressed);
	handle_key_irq();
}

void PPU::handle_key_up(KeypadKey key) {
	mem.io.keyinput.set(key, Keypad::State::Released);
	handle_key_irq();
}

void PPU::handle_key_irq() {
	if(!mem.io.keycnt.irq_enable())
		return;

	bool cond_and = mem.io.keycnt.irq_condition();

	bool raise_irq = cond_and;
	for(unsigned i = 0; i < 10; ++i) {
		const auto key = static_cast<KeypadKey>(i);
		if(!mem.io.keycnt.selected(key))
			continue;

		//  AND
		if(cond_and) {
			raise_irq &= mem.io.keyinput.pressed(key);
		}
		//  OR
		else {
			raise_irq |= mem.io.keyinput.pressed(key);
		}
	}

	if(raise_irq)
		cpu.raise_irq(IRQType::Keypad);
}

void PPU::objects_draw_line(uint16 ly) {
	if(!mem.io.dispcnt->OBJ)
		return;

	for(unsigned i = 0; i < 128; ++i) {
		std::optional<OBJAttr> obj = get_obj_attrs(i);
		if(!obj.has_value())
			continue;
		if(!obj->contains_line(ly))
			continue;
		if(!obj->is_enabled())
			continue;

		objects_draw_obj(ly, *obj);
	}
}

void PPU::objects_draw_obj(uint16 ly, OBJAttr obj) {
	//  FIXME: Include other missed flags (mosaic...)
	const bool yflip = !obj.attr0.rot_scale && (obj.attr1.flags & (1u << 4u));
	const bool xflip = !obj.attr0.rot_scale && (obj.attr1.flags & (1u << 3u));

	const uint8 tile_width = obj.width() / 8;

	uint8 obj_line;
	if(obj.top() > obj.bottom()) {
		obj_line = ly + (256 - obj.attr0.pos_y);
	} else {
		obj_line = ly - obj.attr0.pos_y;
	}

	//  Vertical flip
	if(yflip) {
		obj_line = (obj.attr0.pos_y + obj.height()) - ly;
	}

	const uint8 line_in_current_row = obj_line % 8;
	const uint8 which_vertical_tile = obj_line / 8;
	const uint8 color_depth_mult = (obj.attr0.color_mode ? 2 : 1);

	uint16 base_tile = obj.attr2.tile_number;
	if(mem.io.dispcnt->obj_one_dim) {
		base_tile += tile_width * which_vertical_tile * color_depth_mult;
	} else {
		base_tile += 32 * which_vertical_tile;
	}

	for(unsigned i = 0; i < obj.width(); ++i) {
		if((obj.left() + i) % 512 >= 240) {
			continue;
		}

		const uint16 tile = base_tile + (xflip ? (tile_width - 1 - (i / 8))
		                                       : (i / 8)) *
		                                        color_depth_mult;
		const unsigned x = xflip ? (7 - (i % 8))
		                         : (i % 8);

		const uint8 dot = get_obj_tile_dot(tile, line_in_current_row, x, obj.attr0.color_mode);
		const auto palette = obj.attr0.color_mode ? 0 : obj.attr2.palette_number;
		const std::optional<Color> color = get_obj_palette_color(palette, dot);
		assert(color.has_value());
		colorbuffer_write_obj(obj.attr1.pos_x + i, dot, obj.attr2.priority, *color);
	}
}

void PPU::colorbuffer_blit() {
	const std::optional<Color> backdrop = get_palette_color(0, 0);
	assert(backdrop.has_value());

	for(unsigned i = 0; i < 240; ++i) {
		Dot dot;
		dot.dirty = false;
		for(int priority = 7; priority >= 0; --priority) {
			auto const& other = m_colorbuffer[priority][i];
			if(other.dirty && other.color_number != 0) {
				dot = other;
			}
		}

		if(!dot.dirty) {
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(*backdrop);
		} else {
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(dot.color);
		}
	}

	std::memset(
	        &m_colorbuffer[0],
	        0x0,
	        8 * 240 * sizeof(Dot));
}
