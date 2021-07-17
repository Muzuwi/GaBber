#pragma once
#include "IO/BG.hpp"

class PPU;
class Backgrounds {
	PPU& m_ppu;

	template<unsigned n>
	constexpr bool bg_enabled();

	template<unsigned n>
	void draw_textmode();

	void draw_mode0();
	void draw_mode1();
	void draw_mode2();
	void draw_mode3();
	void draw_mode4();
	void draw_mode5();

	template<unsigned n> constexpr BG<n>& current_bg();
public:
	Backgrounds(PPU& v)
	: m_ppu(v) {}

	void draw_scanline();

};
