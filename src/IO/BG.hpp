#pragma once
#include "IO/PPU.hpp"
#include "MMU/IOReg.hpp"

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

	uint16 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABE;
	}
};

template<unsigned x, unsigned address>
class BGCNT final : public IOReg16<address> {
	void on_write(uint16 val) override {
		constexpr const uint32 mask = (x == 0 || x == 1) ? 0xDFFF
		                                                 : 0xFFFF;
		this->m_register = val & mask;
	}
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
	BGCNT<n, ctl_addr> m_control;
	OffsetReg<xy_base> m_xoffset;
	OffsetReg<xy_base + 2> m_yoffset;
};
