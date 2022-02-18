#pragma once
#include "Bus/Common/IOReg.hpp"
#include "Emulator/StdTypes.hpp"

enum class IRQType : uint8 {
	VBlank = 0,
	HBlank,
	VCounter,
	Timer0,
	Timer1,
	Timer2,
	Timer3,
	Serial,
	DMA0,
	DMA1,
	DMA2,
	DMA3,
	Keypad,
	GamePak,
	Reserved14,
	Reserved15 = 15
};

class IE final : public IOReg16<0x04000200> {
protected:
	uint16 on_read() override;
	void on_write(uint16 val) override;
};

class IF final : public IOReg16<0x04000202> {
protected:
	uint16 on_read() override;
	void on_write(uint16 val) override;
};

class IME final : public IOReg32<0x04000208> {
protected:
	uint32 on_read() override;
	void on_write(uint32 new_value) override;
public:
	constexpr inline bool enabled() const { return m_register & 1u; }
};

class HALTCNT final : public IOReg8<0x04000301> {
	void on_write(uint8 val) override;
public:
	bool m_halt { false };
	bool m_stop { false };
};

class POSTFLG final : public IOReg8<0x04000300> {};

class WaitControl final : public IOReg32<0x04000204> {
protected:
	static constexpr const uint32 writeable_mask = 0x00005FFF;
	static constexpr const uint32 readable_mask = 0x00005FFF;

	uint32 on_read() override;
	void on_write(uint32 new_value) override;
};

class MemCtl final : public IOReg32<0x04000800> {
protected:
	static constexpr const uint32 writeable_mask = 0xFF00002F;
	static constexpr const uint32 readable_mask = 0xFF00002F;

	uint32 on_read() override;
	void on_write(uint32 new_value) override;
};

template<unsigned address>
class EmptyReg : public IOReg32<address> {
protected:
	void on_write(uint32) override {}

	uint32 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
};
