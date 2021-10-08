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

struct SND3CNTL {
	uint8 _unused1  : 5;
	bool  dimension : 1;
	bool  bank      : 1;
	bool  enabled   : 1;
	uint8 _unused2  : 8;
};

struct SND3CNTH {
	uint8 length        : 8;
	uint8 _unused1      : 5;
	uint8 volume        : 2;
	bool force_volume   : 1;
};

struct SND3CNTX {
	uint16  rate        : 11;
	uint8 _unused1      : 3;
	bool length_flag    : 1;
	bool initial        : 1;
	uint16 _unused2     : 16;
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

class Sound3CtlL final : public IOReg16<0x04000070> {
public:
	SND3CNTL* operator->() {
		return this->template as<SND3CNTL>();
	}

	SND3CNTL const* operator->() const {
		return this->template as<SND3CNTL>();
	}
};
class Sound3CtlH final : public IOReg16<0x04000072> {
public:
	SND3CNTH* operator->() {
		return this->template as<SND3CNTH>();
	}

	SND3CNTH const* operator->() const {
		return this->template as<SND3CNTH>();
	}
};
class Sound3CtlX final : public IOReg32<0x04000074> {
public:
	SND3CNTX* operator->() {
		return this->template as<SND3CNTX>();
	}

	SND3CNTX const* operator->() const {
		return this->template as<SND3CNTX>();
	}
};
class Sound3Bank final : public BusDevice {
	Array<uint8, 16> m_bank0;
	Array<uint8, 16> m_bank1;

	template<class R>
	inline R read_safe(uint32 offset, Array<uint8, 16>& bank) const {
		if(offset < 16 && offset + sizeof(R) <= 16) {
			return *reinterpret_cast<R const*>(&bank[offset]);
		} else {
			fmt::print("WaveRAM/ Out of bounds read for offset={}\n", offset);
			return {};
		}
	}

	template<class R>
	inline void write_safe(uint32 offset, R val, Array<uint8, 16>& bank) {
		if(offset < 16 && offset + sizeof(R) <= 16) {
			*reinterpret_cast<R*>(&bank[offset]) = val;
		} else {
			fmt::print("WaveRAM/ Out of bounds write for offset={}\n", offset);
		}
	}

	Array<uint8, 16>& current_bank();
public:
	Sound3Bank()
	: BusDevice(0x04000090, 0x040000A0) {}

	uint32 read32(uint32 offset) override {
		return read_safe<uint32>(offset, current_bank());
	}

	uint16 read16(uint32 offset) override {
		return read_safe<uint16>(offset, current_bank());
	}

	uint8 read8(uint32 offset) override {
		return read_safe<uint8>(offset, current_bank());
	}

	void write32(uint32 offset, uint32 value) override {
		write_safe<uint32>(offset, value, current_bank());
	}

	void write16(uint32 offset, uint16 value) override {
		write_safe<uint16>(offset, value, current_bank());
	}

	void write8(uint32 offset, uint8 value) override {
		write_safe<uint8>(offset, value, current_bank());
	}

	Array<uint8, 16> const& bank0() const {
		return m_bank0;
	}

	Array<uint8, 16> const& bank1() const {
		return m_bank1;
	}

};