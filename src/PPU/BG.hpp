#pragma once
#include "PPU/Unions.hpp"
#include "MMU/IOReg.hpp"

class PPU;


template<unsigned n>
struct __BGExtraVars {
	static_assert(n < 4, "Invalid BG number");
};


template<>
struct __BGExtraVars<2> {
	IOReg16<0x04000020> m_dx;
	IOReg16<0x04000022> m_dmx;
	IOReg16<0x04000024> m_dy;
	IOReg16<0x04000026> m_dmy;
	IOReg32<0x04000028> m_refx;
	IOReg32<0x0400002C> m_refy;
};


template<>
struct __BGExtraVars<3> {
	IOReg16<0x04000030> m_dx;
	IOReg16<0x04000032> m_dmx;
	IOReg16<0x04000034> m_dy;
	IOReg16<0x04000036> m_dmy;
	IOReg32<0x04000038> m_refx;
	IOReg32<0x0400003C> m_refy;
};


template<unsigned address>
class OffsetReg final : public IOReg16<address> {
	void on_write(uint16 val) override {
		this->m_register = val & 0b111111111;
	}
};


template<unsigned address>
class BGCNT final : public IOReg16<address> {
public:
	BGxCNTReg* operator->() {
		return this->template as<BGxCNTReg>();
	}

	BGxCNTReg const* operator->() const {
		return this->template as<BGxCNTReg>();
	}
};


template<unsigned n>
struct BG : public __BGExtraVars<n> {
private:
	static_assert(n < 4, "Invalid BG number");
	static constexpr uint32 ctl_addr = 0x04000008 + n * 2;
	static constexpr uint32 xy_base = 0x04000010 + n * 4;
public:
	BGCNT<ctl_addr> m_control;
	OffsetReg<xy_base> m_xoffset;
	OffsetReg<xy_base+2> m_yoffset;
};


class Backgrounds {
	PPU& m_ppu;
	BG<0> m_bg0;
	BG<1> m_bg1;
	BG<2> m_bg2;
	BG<3> m_bg3;

	template<unsigned n>
	constexpr BG<n>& current_bg() {
		static_assert(n < 4, "Invalid BG number");
		if constexpr(n == 0)
			return m_bg0;
		else if constexpr(n == 1)
			return m_bg1;
		else if constexpr(n == 2)
			return m_bg2;
		else
			return m_bg3;
	}

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
public:
	Backgrounds(PPU& v)
	: m_ppu(v) {}

	void draw_scanline();

};
