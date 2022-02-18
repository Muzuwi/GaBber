#pragma once
#include "Bus/Common/IOReg.hpp"
#include "PPU.hpp"

template<unsigned n>
struct __BGExtraVars {
	__BGExtraVars(GaBber&) {}
	static_assert(n < 4, "Invalid BG number");
};

template<>
struct __BGExtraVars<2> {
	__BGExtraVars(GaBber& emu)
	    : m_dx(emu)
	    , m_dmx(emu)
	    , m_dy(emu)
	    , m_dmy(emu)
	    , m_refx(emu)
	    , m_refy(emu) {}

	IOReg16<0x04000020> m_dx;
	IOReg16<0x04000022> m_dmx;
	IOReg16<0x04000024> m_dy;
	IOReg16<0x04000026> m_dmy;
	IOReg32<0x04000028> m_refx;
	IOReg32<0x0400002C> m_refy;
};

template<>
struct __BGExtraVars<3> {
	__BGExtraVars(GaBber& emu)
	    : m_dx(emu)
	    , m_dmx(emu)
	    , m_dy(emu)
	    , m_dmy(emu)
	    , m_refx(emu)
	    , m_refy(emu) {}

	IOReg16<0x04000030> m_dx;
	IOReg16<0x04000032> m_dmx;
	IOReg16<0x04000034> m_dy;
	IOReg16<0x04000036> m_dmy;
	IOReg32<0x04000038> m_refx;
	IOReg32<0x0400003C> m_refy;
};

template<unsigned address>
class OffsetReg final : public IOReg16<address> {
	uint16 on_read() override;
	void on_write(uint16 val) override;
public:
	OffsetReg(GaBber& emu)
	    : IOReg16<address>(emu) {}
};

template<unsigned x, unsigned address>
class BGCNT final : public IOReg16<address> {
	uint16 on_read() override;
	void on_write(uint16 val) override;
public:
	BGCNT(GaBber& emu)
	    : IOReg16<address>(emu) {}

	BGxCNTReg* operator->() { return this->template as<BGxCNTReg>(); }

	BGxCNTReg const* operator->() const { return this->template as<BGxCNTReg>(); }
};

template<unsigned n>
struct BG : public __BGExtraVars<n> {
private:
	static_assert(n < 4, "Invalid BG number");
	static constexpr uint32 ctl_addr = 0x04000008 + n * 2;
	static constexpr uint32 xy_base = 0x04000010 + n * 4;
public:
	BG(GaBber& emu)
	    : __BGExtraVars<n>(emu)
	    , m_control(emu)
	    , m_xoffset(emu)
	    , m_yoffset(emu) {}

	BGCNT<n, ctl_addr> m_control;
	OffsetReg<xy_base> m_xoffset;
	OffsetReg<xy_base + 2> m_yoffset;
};
