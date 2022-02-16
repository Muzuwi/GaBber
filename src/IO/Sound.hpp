#pragma once
#include <array>
#include <deque>
#include "Headers/Bits.hpp"
#include "MMU/IOReg.hpp"

struct SND1CNTL {
	uint8 sweep_shift    : 3;
	bool sweep_decreases : 1;
	uint8 sweep_time     : 3;
	uint16 _unused       : 9;
};

struct SND1CNTH {
	uint8 length        : 6;
	uint8 duty          : 2;
	uint8 envelope_step : 3;
	uint8 envelope_inc  : 1;
	uint8 envelope_vol  : 4;
};

struct SND1CNTX {
	uint16 frequency : 11;
	uint8 _unused1   : 3;
	bool length_flag : 1;
	bool initial     : 1;
	uint16 _unused2  : 16;
};

struct SND2CNTL {
	uint8 length        : 6;
	uint8 duty          : 2;
	uint8 envelope_step : 3;
	uint8 envelope_inc  : 1;
	uint8 envelope_vol  : 4;
	uint16 _unused      : 16;
};

struct SND2CNTH {
	uint16 frequency : 11;
	uint8 _unused1   : 3;
	bool length_flag : 1;
	bool initial     : 1;
	uint16 _unused2  : 16;
};

struct SNDCNT_L {
	uint8 volume_r         : 3;
	uint8 _unused1         : 1;
	uint8 volume_l         : 3;
	uint8 _unused2         : 1;
	uint8 channel_enable_r : 4;
	uint8 channel_enable_l : 4;
};

struct SNDCNTH {
	uint8 psg_volume    : 2;
	uint8 volumeA       : 1;
	uint8 volumeB       : 1;
	uint8 _unused       : 4;
	bool enable_right_A : 1;
	bool enable_left_A  : 1;
	uint8 timer_sel_A   : 1;
	bool _resetA        : 1;
	bool enable_right_B : 1;
	bool enable_left_B  : 1;
	uint8 timer_sel_B   : 1;
	bool _resetB        : 1;
};

struct SNDBIAS {
	uint8 _unused1  : 1;
	uint16 bias     : 9;
	uint8 _unused2  : 4;
	uint8 sampling  : 2;
	uint16 _unused3 : 16;
};

struct SND3CNTL {
	uint8 _unused1 : 5;
	bool dimension : 1;
	bool bank      : 1;
	bool enabled   : 1;
	uint8 _unused2 : 8;
};

struct SND3CNTH {
	uint8 length      : 8;
	uint8 _unused1    : 5;
	uint8 volume      : 2;
	bool force_volume : 1;
};

struct SND3CNTX {
	uint16 rate      : 11;
	uint8 _unused1   : 3;
	bool length_flag : 1;
	bool initial     : 1;
	uint16 _unused2  : 16;
};

struct SND4CNTL {
	uint8 length        : 6;
	uint8 _unused1      : 2;
	uint8 envelope_step : 3;
	uint8 envelope_inc  : 1;
	uint8 envelope_vol  : 4;
	uint16 _unused2     : 16;
};

struct SND4CNTH {
	uint8 r            : 3;
	bool counter_7bits : 1;
	uint8 s            : 4;
	uint8 _unused1     : 6;
	bool length_flag   : 1;
	bool initial       : 1;
	uint16 _unused2    : 16;
};

class SoundCtlL final : public IOReg16<0x04000080> {
	uint16 on_read() override {
		return this->m_register & 0xFF77u;
	}
	void on_write(T new_value) override {
		this->m_register = new_value & 0xFF77u;
	}
public:
	SNDCNT_L* operator->() {
		return this->template as<SNDCNT_L>();
	}

	SNDCNT_L const* operator->() const {
		return this->template as<SNDCNT_L>();
	}
};

class SoundCtlH final : public IOReg16<0x04000082> {
protected:
	static constexpr const uint16 writeable_mask = 0xFF0F;
	static constexpr const uint16 readable_mask = 0x770F;

	void on_write(uint16 new_value) override;
	uint16 on_read() override {
		return m_register & readable_mask;
	}
public:
	SNDCNTH* operator->() {
		return this->template as<SNDCNTH>();
	}

	SNDCNTH const* operator->() const {
		return this->template as<SNDCNTH>();
	}
};

class SoundCtlX final : public IOReg32<0x04000084> {
protected:
	static constexpr const uint32 writeable_mask = 0x00000080;
	static constexpr const uint32 readable_mask = 0x0000008F;

	void on_write(uint32 new_value) override;
	uint32 on_read() override;
};

class SoundBias final : public IOReg32<0x04000088> {
protected:
	static constexpr const uint32 writeable_mask = 0x0000C3FE;
	static constexpr const uint32 readable_mask = 0x0000C3FE;

	void on_write(uint32 new_value) override {
		m_register = new_value & writeable_mask;
	}
	uint32 on_read() override {
		return m_register & readable_mask;
	}
};

class Sound1CtlL final : public IOReg16<0x04000060> {
	void on_write(uint16 new_value) override {
		this->m_register = new_value & ~0xFF80u;
	}
	uint16 on_read() override {
		return this->m_register & ~0xFF80u;
	}
public:
	SND1CNTL* operator->() {
		return this->template as<SND1CNTL>();
	}

	SND1CNTL const* operator->() const {
		return this->template as<SND1CNTL>();
	}
};

class Sound1CtlH final : public IOReg16<0x04000062> {
	void on_write(uint16 new_value) override;
	uint16 on_read() override {
		return this->m_register & ~0x3F;
	}
public:
	SND1CNTH* operator->() {
		return this->template as<SND1CNTH>();
	}

	SND1CNTH const* operator->() const {
		return this->template as<SND1CNTH>();
	}
};

class Sound1CtlX final : public IOReg32<0x04000064> {
	void on_write(uint32 new_value) override;
	uint32 on_read() override {
		return 0x4000;
	}
public:
	SND1CNTX* operator->() {
		return this->template as<SND1CNTX>();
	}

	SND1CNTX const* operator->() const {
		return this->template as<SND1CNTX>();
	}
};

class Sound2CtlL final : public IOReg32<0x04000068> {
protected:
	void on_write(uint32 new_value) override;
	uint32 on_read() override {
		return this->m_register & 0x0000FFC0;
	}
public:
	SND2CNTL* operator->() {
		return this->template as<SND2CNTL>();
	}

	SND2CNTL const* operator->() const {
		return this->template as<SND2CNTL>();
	}
};

class Sound2CtlH final : public IOReg32<0x0400006c> {
	void on_write(uint32 new_value) override;
	uint32 on_read() override {
		return 0x4000;
	}
public:
	SND2CNTH* operator->() {
		return this->template as<SND2CNTH>();
	}

	SND2CNTH const* operator->() const {
		return this->template as<SND2CNTH>();
	}
};

class Sound3CtlL final : public IOReg16<0x04000070> {
protected:
	static constexpr const uint32 writeable_mask = 0x00E0;
	static constexpr const uint32 readable_mask = 0x00E0;

	void on_write(uint16 new_value) override;
	uint16 on_read() override {
		return m_register & readable_mask;
	}
public:
	SND3CNTL* operator->() {
		return this->template as<SND3CNTL>();
	}

	SND3CNTL const* operator->() const {
		return this->template as<SND3CNTL>();
	}
};

class Sound3CtlH final : public IOReg16<0x04000072> {
protected:
	static constexpr const uint32 writeable_mask = 0xE0FF;
	static constexpr const uint32 readable_mask = 0xE000;

	void on_write(uint16 new_value) override {
		m_register = new_value & writeable_mask;
	}
	uint16 on_read() override {
		return m_register & readable_mask;
	}
public:
	SND3CNTH* operator->() {
		return this->template as<SND3CNTH>();
	}

	SND3CNTH const* operator->() const {
		return this->template as<SND3CNTH>();
	}
};

class Sound3CtlX final : public IOReg32<0x04000074> {
protected:
	static constexpr const uint32 writeable_mask = 0x0000C7FF;
	static constexpr const uint32 readable_mask = 0x00004000;

	void on_write(uint32 new_value) override;
	uint32 on_read() override {
		return m_register & readable_mask;
	}
public:
	SND3CNTX* operator->() {
		return this->template as<SND3CNTX>();
	}

	SND3CNTX const* operator->() const {
		return this->template as<SND3CNTX>();
	}
};

class Sound3Bank final : public BusDevice {
	std::array<uint8, 16> m_bank0;
	std::array<uint8, 16> m_bank1;

	template<class R>
	inline R read_safe(uint32 offset, std::array<uint8, 16>& bank) const {
		if(offset < 16 && offset + sizeof(R) <= 16) {
			return *reinterpret_cast<R const*>(&bank[offset]);
		} else {
			fmt::print("WaveRAM/ Out of bounds read for offset={}\n", offset);
			return {};
		}
	}

	template<class R>
	inline void write_safe(uint32 offset, R val, std::array<uint8, 16>& bank) {
		if(offset < 16 && offset + sizeof(R) <= 16) {
			*reinterpret_cast<R*>(&bank[offset]) = val;
		} else {
			fmt::print("WaveRAM/ Out of bounds write for offset={}\n", offset);
		}
	}

	std::array<uint8, 16>& current_bank();
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

	std::array<uint8, 16> const& bank0() const {
		return m_bank0;
	}

	std::array<uint8, 16> const& bank1() const {
		return m_bank1;
	}
};

class Sound4CtlL final : public IOReg32<0x04000078> {
protected:
	static constexpr const uint32 writeable_mask = 0x0000FF3F;
	static constexpr const uint32 readable_mask = 0x0000FF00;

	void on_write(uint32 new_value) override {
		m_register = new_value & writeable_mask;
	}
	uint32 on_read() override {
		return m_register & readable_mask;
	}
public:
	SND4CNTL* operator->() {
		return this->template as<SND4CNTL>();
	}

	SND4CNTL const* operator->() const {
		return this->template as<SND4CNTL>();
	}
};

class Sound4CtlH final : public IOReg32<0x0400007C> {
protected:
	static constexpr const uint32 writeable_mask = 0x0000C0FF;
	static constexpr const uint32 readable_mask = 0x000040FF;

	void on_write(uint32 new_value) override {
		m_register = new_value & writeable_mask;
	}
	uint32 on_read() override {
		return m_register & readable_mask;
	}
public:
	SND4CNTH* operator->() {
		return this->template as<SND4CNTH>();
	}

	SND4CNTH const* operator->() const {
		return this->template as<SND4CNTH>();
	}
};

class SoundFifoA final : public IOReg32<0x040000A0> {
	std::deque<uint8> m_fifo;
	void add_sample(uint8 byte) {
		m_fifo.push_back(byte);
	}
protected:
	void on_write(uint32 new_value) override {
		add_sample(Bits::byte_le<0>(new_value));
		add_sample(Bits::byte_le<1>(new_value));
		add_sample(Bits::byte_le<2>(new_value));
		add_sample(Bits::byte_le<3>(new_value));
	}
	uint32 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
public:
	std::deque<uint8>& fifo() {
		return m_fifo;
	}
};

class SoundFifoB final : public IOReg32<0x040000A4> {
	std::deque<uint8> m_fifo;
	void add_sample(uint8 byte) {
		m_fifo.push_back(byte);
	}
protected:
	void on_write(uint32 new_value) override {
		add_sample(Bits::byte_le<0>(new_value));
		add_sample(Bits::byte_le<1>(new_value));
		add_sample(Bits::byte_le<2>(new_value));
		add_sample(Bits::byte_le<3>(new_value));
	}
	uint32 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
public:
	std::deque<uint8>& fifo() {
		return m_fifo;
	}
};
