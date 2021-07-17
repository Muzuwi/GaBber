#include "Headers/ARM7TDMI.hpp"
#include "MMU/BusInterface.hpp"
#include "PPU/PPU.hpp"

PPU::PPU(ARM7TDMI& a,BusInterface& b)
: cpu(a), mmu(b), m_backgrounds(*this) {
}

bool PPU::is_HBlank() const {
	return m_lcd_ctl.m_dispstat->HBlank && vcount() >= 160;
}

bool PPU::is_VBlank() const {
	return vcount() >= 160 && vcount() <= 227;
}

static unsigned global_cycles {0};
static unsigned current_scanline_position {0};

void PPU::next_scanline() {
	current_scanline_position = 0;
	vcount()++;

	if(vcount() == 160) {
		m_lcd_ctl.m_dispstat->HBlank = false;
		m_lcd_ctl.m_dispstat->VBlank = true;
		cpu.dma_start_vblank();
		if(m_lcd_ctl.m_dispstat->VBlank_IRQ) {
			cpu.raise_irq(IRQType::VBlank);
		}
		m_frame_ready = true;
	} else if(vcount() == 228) {
		m_lcd_ctl.m_dispstat->VBlank = false;
		vcount() = 0;
	}
}

void PPU::cycle() {
	global_cycles++;

	static unsigned pixel_cycles {0};
	if(++pixel_cycles != 4)
		return;

	pixel_cycles = 0;
	current_scanline_position++;

	if(current_scanline_position == 240 && !is_VBlank()) {
		m_lcd_ctl.m_dispstat->HBlank = true;
		cpu.dma_start_hblank();
		if(m_lcd_ctl.m_dispstat->HBlank_IRQ) {
			cpu.raise_irq(IRQType::HBlank);
		}

		draw_scanline();
	} else if(current_scanline_position == 308) {
		next_scanline();
	}
}

void PPU::draw_scanline() {
	m_backgrounds.draw_scanline();
	objects_draw_line(vcount());
	colorbuffer_blit();
}

void PPU::handle_key_down(KeypadKey key) {
	m_keypad.set(key, Keypad::State::Pressed);
	handle_key_irq();
}

void PPU::handle_key_up(KeypadKey key) {
	m_keypad.set(key, Keypad::State::Released);
	handle_key_irq();
}

void PPU::handle_key_irq() {
	if(!m_keypadcnt.irq_enable())
		return;

	bool cond_and = m_keypadcnt.irq_condition();

	bool raise_irq = cond_and;
	for(unsigned i = 0; i < 10; ++i) {
		const auto key = static_cast<KeypadKey>(i);
		if(!m_keypadcnt.selected(key))
			continue;

		//  AND
		if(cond_and) {
			raise_irq &= m_keypad.pressed(key);
		}
		//  OR
		else {
			raise_irq |= m_keypad.pressed(key);
		}
	}

	if(raise_irq)
		cpu.raise_irq(IRQType::Keypad);
}

void PPU::objects_draw_line(uint16 ly) {
	if(!m_lcd_ctl.m_dispcnt->OBJ)
		return;

	for(unsigned i = 0; i < 128; ++i) {
		Optional<OBJAttr> obj = get_obj_attrs(i);
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

	uint8 obj_line = ly - obj.attr0.pos_y;
	//  Vertical flip
	if(yflip) {
		obj_line = (obj.attr0.pos_y + obj.height()) - ly;
	}

	const uint8 which_vertical_tile = obj_line / 8;

	uint16 base_tile = obj.attr2.tile_number;
	if(m_lcd_ctl.m_dispcnt->obj_one_dim) {
		base_tile += (tile_width-1) * which_vertical_tile;
	} else {
		base_tile += 32 * which_vertical_tile;
	}

	for(unsigned i = 0; i < obj.width(); ++i) {
		const uint16 tile = base_tile + (xflip ? (tile_width - 1 - (i / 8))
											   : (i / 8));
		const unsigned x = xflip ? (7 - (i % 8))
								 : (i % 8);

		const uint8 dot = get_obj_tile_dot(tile, obj_line, x, obj.attr0.color_mode);
		const auto palette = obj.attr0.color_mode ? 0 : obj.attr2.palette_number;
		const Optional<Color> color = get_obj_palette_color(palette, dot);
		assert(color.has_value());

 		colorbuffer_write(obj.attr1.pos_x + i, dot, obj.attr2.priority, *color);
	}
}

void PPU::colorbuffer_blit() {
	const Optional<Color> backdrop = get_palette_color(0, 0);
	assert(backdrop.has_value());

	for(unsigned i = 0; i < 240; ++i) {
		auto& dot = m_colorbuffer[i];
		if(!dot.dirty) {
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(*backdrop);
		} else {
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(dot.color);
		}
	}

	for(auto& dot : m_colorbuffer) {
		dot.dirty = false;
	}
}
