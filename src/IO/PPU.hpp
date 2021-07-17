#pragma once
#include "Headers/StdTypes.hpp"
#include "MMU/IOReg.hpp"

enum class KeypadKey {
	A = 0,
	B,
	Sel,
	Start,
	Right,
	Left,
	Up,
	Down,
	R,
	L = 9
};


struct DISPCNTReg {
	uint8 video_mode    : 3;
	bool cgb_mode       : 1;
	uint8 frame_select  : 1;
	bool oam_in_HBlank  : 1;
	bool obj_one_dim    : 1;
	bool forced_blank   : 1;
	bool BG0            : 1;
	bool BG1            : 1;
	bool BG2            : 1;
	bool BG3            : 1;
	bool OBJ            : 1;
	bool window0        : 1;
	bool window1        : 1;
	bool objWindow      : 1;
} __attribute__((packed));


struct DISPSTATReg {
	bool VBlank         : 1;
	bool HBlank         : 1;
	bool VCounter       : 1;
	bool VBlank_IRQ     : 1;
	bool HBlank_IRQ     : 1;
	bool VCounter_IRQ   : 1;
	uint8 _unused       : 2;
	uint8 LYC           : 8;
} __attribute__((packed));


struct BGxCNTReg {
	uint8 priority          : 2;
	uint8 base_tile_block   : 2;
	uint8 _unused           : 2;
	bool mosaic             : 1;
	bool palette_flag       : 1;
	uint8 base_screen_block : 5;
	bool area_overflow      : 1;
	uint8 screen_size       : 2;
} __attribute__((packed));


union TextScreenData {
	struct {
		uint16 tile_number   : 10;
		bool horizontal_flip : 1;
		bool vertical_flip   : 1;
		uint8 palette_number : 4;
	} __attribute__((packed)) m_struct;
	uint16 _raw;

	TextScreenData(uint16 v)
	: _raw(v) {}
};


union Color {
	struct {
		uint8 red   : 5;
		uint8 green : 5;
		uint8 blue  : 5;
		uint8 _unused : 1;
	} __attribute__((packed));
	uint16 _raw;

	Color(Color const& c)
	: _raw(c._raw) {}

	Color(uint16 v)
	: _raw(v) {}

	Color()
	: _raw(0) {}
};


enum class OBJMode : uint8 {
	Normal = 0,
	SemiTransparent = 1,
	OBJWindow = 2,
	Prohibited = 3
};


enum class ColorMode : uint8 {
	p16x16 = 0,
	p256x1 = 1,
};


enum class OBJShape : uint8 {
	Square = 0,
	Horizontal = 1,
	Vertical = 2,
	Prohibited = 3
};


struct OBJAttr0 {
	uint8 pos_y : 8;
	bool rot_scale : 1;
	bool double_size_or_disable : 1;
	OBJMode obj_mode : 2;
	bool obj_mosaic : 1;
	bool color_mode : 1;
	OBJShape obj_shape : 2;
} __attribute__((packed));


struct OBJAttr1 {
	uint16 pos_x : 9;
	uint8 flags : 5;
	uint8 size : 2;
} __attribute__((packed));


struct OBJAttr2 {
	uint16 tile_number : 10;
	uint8 priority : 2;
	uint8 palette_number : 4;
} __attribute__((packed));


struct OBJAttr {
	OBJAttr0 attr0;
	OBJAttr1 attr1;
	OBJAttr2 attr2;

	static constexpr const inline uint8 size_lookup[4][3][2] {
		{ {8,8},    {16,8},     {8,16}  },
		{ {16,16},  {32,8},     {8,32}  },
		{ {32,32},  {32,16},    {16,32} },
		{ {64,64},  {64,32},    {32,64} },
	};

	inline uint8 width() const {
		return size_lookup[attr1.size][static_cast<uint8>(attr0.obj_shape)][0];
	}

	inline uint8 height() const {
		return size_lookup[attr1.size][static_cast<uint8>(attr0.obj_shape)][1];
	}

	inline bool contains_line(uint16 ly) const {
		return ly >= attr0.pos_y && ly < attr0.pos_y + height();
	}

	inline bool is_enabled() const {
		return attr0.rot_scale || !attr0.double_size_or_disable;    //  FIXME: Might be wrong (does rot_scale imply enabled?)
	}

} __attribute__((packed));


class DISPCNT : public IOReg16<0x04000000> {
public:
	DISPCNTReg* operator->() {
		return this->template as<DISPCNTReg>();
	}

	DISPCNTReg const* operator->() const {
		return this->template as<DISPCNTReg>();
	}
};

class GreenSwap : public IOReg16<0x04000002> {

};

class DISPSTAT : public IOReg16<0x04000004> {
public:
	DISPSTATReg* operator->() {
		return this->template as<DISPSTATReg>();
	}

	DISPSTATReg const* operator->() const {
		return this->template as<DISPSTATReg>();
	}
};

class VCOUNT : public IOReg16<0x04000006> {

};
