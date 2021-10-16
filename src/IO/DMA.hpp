#pragma once
#include "MMU/IOReg.hpp"
#include "IO/Interrupt.hpp"

enum class DMADestCtrl : uint8 {
	Increment = 0,
	Decrement = 1,
	Fixed = 2,
	Reload = 3
};

enum class DMASrcCtrl : uint8 {
	Increment = 0,
	Decrement = 1,
	Fixed = 2,
	Prohibited = 3
};

enum class DMAStartTiming : uint8 {
	Immediate = 0,
	VBlank = 1,
	HBlank = 2,
	Special = 3
};

struct DMACtrlReg {
	uint16 word_count   : 16;
	uint8 _unused       : 5;
	DMADestCtrl dest_ctl      : 2;
	DMASrcCtrl src_ctl        : 2;
	uint8 repeat        : 1;
	uint8 transfer_size : 1;
	uint8 game_pak_drq  : 1;
	DMAStartTiming start_timing  : 2;
	uint8 irq_on_finish : 1;
	uint8 enable        : 1;
} __attribute__((packed));


namespace DMA {
	template<unsigned x>
	static constexpr IRQType irq_type() {
		static_assert(x < 4, "Invalid DMA number");
		if constexpr (x == 0)
			return IRQType::DMA0;
		else if constexpr(x == 1)
			return IRQType::DMA1;
		else if constexpr(x == 2)
			return IRQType::DMA2;
		else
			return IRQType::DMA3;
	}

	template<unsigned x>
	static constexpr uint32 reg_base() {
		static_assert(x < 4, "Invalid DMA number");
		return 0x40000b0 + 12 * x;
	}

	template<unsigned x>
	static constexpr uint32 source_mask() {
		static_assert(x < 4, "Invalid DMA number");
		if constexpr(x == 0)
			return 0x7ffffff;
		else
			return 0xfffffff;
	}

	template<unsigned x>
	static constexpr uint32 destination_mask() {
		static_assert(x < 4, "Invalid DMA number");
		if constexpr(x == 0 || x == 1 || x == 2)
			return 0x7ffffff;
		else
			return 0xfffffff;
	}

	template<unsigned x>
	static constexpr unsigned max_count() {
		static_assert(x < 4, "Invalid DMA number");
		if constexpr(x == 3)
			return 0x10000;
		else
			return 0x4000;
	}
}


template<unsigned x>
class DMASrc final : public IOReg32<DMA::reg_base<x>()> {
protected:
	void on_write(uint32 new_value) override {
		this->m_register = new_value & DMA::source_mask<x>();
	}
	uint32 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
};

template<unsigned x>
class DMADest final : public IOReg32<DMA::reg_base<x>() + 4> {
protected:
	void on_write(uint32 new_value) override {
		this->m_register = new_value & DMA::destination_mask<x>();
	}
	uint32 on_read() override {
		//  FIXME: unreadable I/O register
		return 0xBABEBABE;
	}
};

template<unsigned x>
class DMACtrl final : public IOReg32<DMA::reg_base<x>() + 8> {
protected:
	static constexpr const uint32 count_mask    = (x == 3) ? 0xFFFF : 0x3FFF;
	static constexpr const uint32 bit11_mask    = (x == 3) ? 0x08000000 : 0x00000000;
	static constexpr const uint32 writable_mask = 0xf7e00000u | count_mask | bit11_mask;
	static constexpr const uint32 readable_mask = 0xf7e00000u | bit11_mask;

	void on_write(uint32 new_value) override {
		this->m_register = new_value & writable_mask;
	}

	uint32 on_read() override {
		return this->m_register & readable_mask;
	}
public:
	DMACtrlReg* operator->() {
		return this->template as<DMACtrlReg>();
	}

	DMACtrlReg const* operator->() const {
		return this->template as<DMACtrlReg>();
	}
};


template<unsigned x>
struct DMAx final {
	DMASrc<x> m_source;
	DMADest<x> m_destination;
	DMACtrl<x> m_ctrl;

	uint32 m_original_destination_ptr {};
	bool m_is_running {false};
	uint32 m_destination_ptr {};
	uint32 m_source_ptr {};
	unsigned m_count {};

	uint32 m_fetched_data {};
	bool m_fetched {false};
	bool m_finished {false};
};
