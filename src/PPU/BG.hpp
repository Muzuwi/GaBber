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
	IOReg<0x04000020, _DummyReg<uint16>, IOAccess::W> m_dx;
	IOReg<0x04000022, _DummyReg<uint16>, IOAccess::W> m_dmx;
	IOReg<0x04000024, _DummyReg<uint16>, IOAccess::W> m_dy;
	IOReg<0x04000026, _DummyReg<uint16>, IOAccess::W> m_dmy;
	IOReg<0x04000028, _DummyReg<uint32>, IOAccess::W> m_refx;
	IOReg<0x0400002C, _DummyReg<uint32>, IOAccess::W> m_refy;
};


template<>
struct __BGExtraVars<3> {
	IOReg<0x04000030, _DummyReg<uint16>, IOAccess::W> m_dx;
	IOReg<0x04000032, _DummyReg<uint16>, IOAccess::W> m_dmx;
	IOReg<0x04000034, _DummyReg<uint16>, IOAccess::W> m_dy;
	IOReg<0x04000036, _DummyReg<uint16>, IOAccess::W> m_dmy;
	IOReg<0x04000038, _DummyReg<uint32>, IOAccess::W> m_refx;
	IOReg<0x0400003C, _DummyReg<uint32>, IOAccess::W> m_refy;
};


template<unsigned n>
struct BG : public __BGExtraVars<n> {
private:
	static_assert(n < 4, "Invalid BG number");
	static constexpr uint32 ctl_addr = 0x04000008 + n * 2;
	static constexpr uint32 xy_base = 0x04000010 + n * 4;
public:
	IOReg<ctl_addr, BGxCNTReg, IOAccess::RW> m_control;
	class : public IOReg<xy_base, _DummyReg<uint16>, IOAccess::RW> {
		void on_write(uint16 val) override {
			this->m_register.m_raw = val & 0b111111111;
		}

		std::string identify() const {
			return "BG" + std::to_string(n) + "HOFS";
		}
	} m_xoffset;
	class : public IOReg<xy_base + 2, _DummyReg<uint16>, IOAccess::RW> {
		void on_write(uint16 val) override {
			this->m_register.m_raw = val & 0b111111111;
		}

		std::string identify() const {
			return "BG" + std::to_string(n) + "VOFS";
		}
	} m_yoffset;
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
