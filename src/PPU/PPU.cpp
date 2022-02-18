#include "PPU/PPU.hpp"
#include "Bus/Common/MemoryLayout.hpp"
#include "CPU/ARM7TDMI.hpp"

PPU::PPU(GaBber& emu)
    : Module(emu)
    , m_backgrounds(*this) {}

bool PPU::is_HBlank() const {
	return io().dispstat->HBlank && vcount() >= 160;
}

bool PPU::is_VBlank() const {
	return vcount() >= 160 && vcount() <= 227;
}

static unsigned global_cycles { 0 };
static unsigned current_scanline_position { 0 };

void PPU::next_scanline() {
	current_scanline_position = 0;
	vcount()++;

	if(vcount() == io().dispstat->LYC) {
		io().dispstat->VCounter = true;
		if(io().dispstat->VCounter_IRQ) {
			cpu().raise_irq(IRQType::VCounter);
		}
	} else {
		io().dispstat->VCounter = false;
	}

	io().dispstat->HBlank = false;

	if(vcount() == 160) {
		io().dispstat->VBlank = true;
		cpu().dma_start_vblank();
		if(io().dispstat->VBlank_IRQ) {
			cpu().raise_irq(IRQType::VBlank);
		}
		m_frame_ready = true;
	} else if(vcount() == 228) {
		io().dispstat->VBlank = false;
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
		io().dispstat->HBlank = true;
		cpu().dma_start_hblank();
		if(io().dispstat->HBlank_IRQ) {
			cpu().raise_irq(IRQType::HBlank);
		}

		draw_scanline();
	} else if(current_scanline_position == 308) {
		next_scanline();
	}
}

void PPU::draw_scanline() {
	if(io().dispcnt->forced_blank) {
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
	io().keyinput.set(key, Keypad::State::Pressed);
	handle_key_irq();
}

void PPU::handle_key_up(KeypadKey key) {
	io().keyinput.set(key, Keypad::State::Released);
	handle_key_irq();
}

void PPU::handle_key_irq() {
	if(!io().keycnt.irq_enable())
		return;

	bool cond_and = io().keycnt.irq_condition();

	bool raise_irq = cond_and;
	for(unsigned i = 0; i < 10; ++i) {
		const auto key = static_cast<KeypadKey>(i);
		if(!io().keycnt.selected(key))
			continue;

		//  AND
		if(cond_and) {
			raise_irq &= io().keyinput.pressed(key);
		}
		//  OR
		else {
			raise_irq |= io().keyinput.pressed(key);
		}
	}

	if(raise_irq)
		cpu().raise_irq(IRQType::Keypad);
}

void PPU::objects_draw_line(uint16 ly) {
	if(!io().dispcnt->OBJ)
		return;

	for(unsigned i = 0; i < 128; ++i) {
		const auto obj = mem().oam.readT<OBJAttr>(PPU::obj_attr_offset(i));
		if(!obj.contains_line(ly))
			continue;
		if(!obj.is_enabled())
			continue;

		objects_draw_obj(ly, obj);
	}
}

void PPU::objects_draw_obj(uint16 ly, OBJAttr obj) {
	const auto get_obj_tile_dot = [this](uint16 tile, uint8 ly_in_tile, uint8 dot_in_tile, bool depth_flag) -> uint8 {
		const uint32 base = 0x00010000;

		const unsigned d = depth_flag ? 1 : 2;
		const unsigned offset_to_tile = tile * 32;
		const unsigned offset_to_dot = ly_in_tile * (8 / d) + (dot_in_tile / d);

		uint8 byte = mem().vram.readT<uint8>(base + offset_to_tile + offset_to_dot);
		if(!depth_flag) {
			const bool is_right_pixel = (dot_in_tile % 2) != 0;
			if(is_right_pixel)
				byte >>= 4u;
			byte &= 0x0Fu;
		}

		return byte;
	};

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
	if(io().dispcnt->obj_one_dim) {
		base_tile += tile_width * which_vertical_tile * color_depth_mult;
	} else {
		base_tile += 32 * which_vertical_tile;
	}

	for(unsigned i = 0; i < obj.width(); ++i) {
		if((obj.left() + i) % 512 >= 240) {
			continue;
		}

		const uint16 tile = base_tile + (xflip ? (tile_width - 1 - (i / 8)) : (i / 8)) * color_depth_mult;
		const unsigned x = xflip ? (7 - (i % 8)) : (i % 8);

		const uint8 dot = get_obj_tile_dot(tile, line_in_current_row, x, obj.attr0.color_mode);
		const auto palette = obj.attr0.color_mode ? 0 : obj.attr2.palette_number;
		const auto color = mem().palette.readT<Color>(PPU::obj_palette_color_offset(palette, dot));
		colorbuffer_write_obj(obj.attr1.pos_x + i, dot, obj.attr2.priority, color);
	}
}

void PPU::colorbuffer_blit() {
	const auto backdrop = mem().palette.readT<Color>(PPU::palette_color_offset(0, 0));

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
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(backdrop);
		} else {
			m_framebuffer[vcount() * 240 + i] = color_to_rgba32(dot.color);
		}
	}

	std::memset(&m_colorbuffer[0], 0x0, 8 * 240 * sizeof(Dot));
}

uint16& PPU::vcount() {
	return *io().vcount;
}

uint16 const& PPU::vcount() const {
	return *io().vcount;
}
