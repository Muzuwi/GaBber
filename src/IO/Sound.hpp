#pragma once

struct SND1CNTL {
	uint8 sweep_shift : 3;
	bool  sweep_decreases : 1;
	uint8 sweep_time : 3;
	uint16 _unused : 9;
};

struct SND1CNTH {
	uint8 length        : 6;
	uint8 duty          : 2;
	uint8 envelope_step : 3;
	uint8 envelope_inc  : 1;
	uint8 envelope_vol  : 4;
};

struct SND1CNTX {
	uint16 frequency    : 11;
	uint8 _unused1      : 3;
	bool  length_flag   : 1;
	bool  initial       : 1;
	uint16 _unused2     : 16;
};

struct SNDCNT_L {
	uint8 volume_r : 3;
	uint8 _unused1 : 1;
	uint8 volume_l : 3;
	uint8 _unused2 : 1;
	uint8 channel_enable_r : 4;
	uint8 channel_enable_l : 4;
};

struct SNDBIAS {
	uint8 _unused1  : 1;
	uint16 bias     : 9;
	uint8 _unused2  : 4;
	uint8 sampling  : 2;
	uint16 _unused3 : 16;
};


class SoundCtlL final : public IOReg16<0x04000080> {
public:
	SNDCNT_L* operator->() {
		return this->template as<SNDCNT_L>();
	}

	SNDCNT_L const* operator->() const {
		return this->template as<SNDCNT_L>();
	}
};

class Sound1CtlL final : public IOReg16<0x04000060> {
public:
	SND1CNTL* operator->() {
		return this->template as<SND1CNTL>();
	}

	SND1CNTL const* operator->() const {
		return this->template as<SND1CNTL>();
	}
};

class Sound1CtlH final : public IOReg16<0x04000062> {
public:
	SND1CNTH* operator->() {
		return this->template as<SND1CNTH>();
	}

	SND1CNTH const* operator->() const {
		return this->template as<SND1CNTH>();
	}
};

class Sound1CtlX final : public IOReg32<0x04000064> {
public:
	SND1CNTX* operator->() {
		return this->template as<SND1CNTX>();
	}

	SND1CNTX const* operator->() const {
		return this->template as<SND1CNTX>();
	}
};

class Sound2CtlL final : public IOReg16<0x04000068> {
public:
	SND1CNTH* operator->() {
		return this->template as<SND1CNTH>();
	}

	SND1CNTH const* operator->() const {
		return this->template as<SND1CNTH>();
	}
};

class Sound2CtlH final : public IOReg32<0x0400006c> {
public:
	SND1CNTX* operator->() {
		return this->template as<SND1CNTX>();
	}

	SND1CNTX const* operator->() const {
		return this->template as<SND1CNTX>();
	}
};
