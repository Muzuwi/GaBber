#include "Bus/IO/DMA.hpp"
#include "Bus/IO/IOContainer.hpp"
#include "CPU/ARM7TDMI.hpp"

void ARM7TDMI::dma_run_all() {
	if(dma_is_running<0>()) {
		dma_run<0>();
		return;
	}
	if(dma_is_running<1>()) {
		dma_run<1>();
		return;
	}
	if(dma_is_running<2>()) {
		dma_run<2>();
		return;
	}
	if(dma_is_running<3>()) {
		dma_run<3>();
		return;
	}
}

template<unsigned int x>
void ARM7TDMI::dma_run() {
	DMAx<x>& s = io().template dma_for_num<x>();

	if(!s.m_is_running) {
		return;
	}

	const unsigned n = s.m_count;
	while(s.m_count--) {
		//  In FIFO mode, a 32-bit transfer is forced
		const bool size_flag = (s.m_ctrl->start_timing == DMAStartTiming::Special && (x == 1 || x == 2))
		                               ? true
		                               : s.m_ctrl->transfer_size;

		const uint32 data = (size_flag) ? mem_read32(s.m_source_ptr) : mem_read16(s.m_source_ptr);
		if(size_flag) {
			mem_write32(s.m_destination_ptr, data);
		} else {
			mem_write16(s.m_destination_ptr, static_cast<uint16>(data));
		}

		DMASrcCtrl ctl = s.m_ctrl->src_ctl;
		if(ctl == DMASrcCtrl::Increment)
			s.m_source_ptr += size_flag ? 4 : 2;
		else if(ctl == DMASrcCtrl::Decrement)
			s.m_source_ptr -= size_flag ? 4 : 2;

		//  Destination address is not incremented in FIFO mode
		if(s.m_ctrl->start_timing == DMAStartTiming::Special && (x == 1 || x == 2)) {
			continue;
		}

		DMADestCtrl dest_ctl = s.m_ctrl->dest_ctl;
		if(dest_ctl == DMADestCtrl::Increment || dest_ctl == DMADestCtrl::Reload)
			s.m_destination_ptr += size_flag ? 4 : 2;
		else if(dest_ctl == DMADestCtrl::Decrement)
			s.m_destination_ptr -= size_flag ? 4 : 2;
	}

	s.m_is_running = false;

	if(s.m_ctrl->irq_on_finish) {
		raise_irq(DMA::irq_type<x>());
	}

	if(!s.m_ctrl->repeat || s.m_ctrl->start_timing == DMAStartTiming::Immediate) {
		s.m_ctrl->enable = false;
	}

	m_wait_cycles += 2 /*N*/ + 2 * (n - 1) /*S*/ + 2 /*I*/;
}

/*
 *  When the enable bit is toggled, the internal registers (SAD, DAD, CNT_L) are reloaded.
 */
template<unsigned x>
void ARM7TDMI::dma_on_enable() {
	DMAx<x>& s = io().template dma_for_num<x>();

	//  In FIFO mode, word count is fixed
	if(s.m_ctrl->start_timing == DMAStartTiming::Special && (x == 1 || x == 2)) {
		s.m_count = 4;
	} else {
		s.m_count = s.m_ctrl->word_count;
		if(s.m_count == 0) {
			s.m_count = DMA::max_count<x>();
		}
	}

	s.m_source_ptr = *s.m_source;
	s.m_destination_ptr = *s.m_destination;

	//  Start immediate transfers immediately
	if(s.m_ctrl->start_timing == DMAStartTiming::Immediate) {
		s.m_is_running = true;
	}
}

/*
 *  Called when a specified DMA transfer should be restarted (for start timings other than Immediate)
 */
template<unsigned int x>
void ARM7TDMI::dma_resume() {
	DMAx<x>& dma = io().template dma_for_num<x>();

	if(dma.m_is_running) {
		return;
	}

	dma.m_is_running = true;

	//  In FIFO mode, word count is fixed
	if(dma.m_ctrl->start_timing == DMAStartTiming::Special && (x == 1 || x == 2)) {
		dma.m_count = 4;
	} else {
		dma.m_count = dma.m_ctrl->word_count;
		if(dma.m_count == 0) {
			dma.m_count = DMA::max_count<x>();
		}
	}

	//  Reload DAD on restart
	if(dma.m_ctrl->dest_ctl == DMADestCtrl::Reload) {
		dma.m_destination_ptr = *dma.m_destination;
	}
}

void ARM7TDMI::dma_start_vblank() {
	DMAx<0>& dma0 = io().dma0;
	DMAx<1>& dma1 = io().dma1;
	DMAx<2>& dma2 = io().dma2;
	DMAx<3>& dma3 = io().dma3;

	if(dma0.m_ctrl->enable && dma0.m_ctrl->start_timing == DMAStartTiming::VBlank) {
		dma_resume<0>();
	}

	if(dma1.m_ctrl->enable && dma1.m_ctrl->start_timing == DMAStartTiming::VBlank) {
		dma_resume<1>();
	}

	if(dma2.m_ctrl->enable && dma2.m_ctrl->start_timing == DMAStartTiming::VBlank) {
		dma_resume<2>();
	}

	if(dma3.m_ctrl->enable && dma3.m_ctrl->start_timing == DMAStartTiming::VBlank) {
		dma_resume<3>();
	}
}

void ARM7TDMI::dma_start_hblank() {
	DMAx<0>& dma0 = io().dma0;
	DMAx<1>& dma1 = io().dma1;
	DMAx<2>& dma2 = io().dma2;
	DMAx<3>& dma3 = io().dma3;

	if(dma0.m_ctrl->enable && dma0.m_ctrl->start_timing == DMAStartTiming::HBlank) {
		dma_resume<0>();
	}

	if(dma1.m_ctrl->enable && dma1.m_ctrl->start_timing == DMAStartTiming::HBlank) {
		dma_resume<1>();
	}

	if(dma2.m_ctrl->enable && dma2.m_ctrl->start_timing == DMAStartTiming::HBlank) {
		dma_resume<2>();
	}

	if(dma3.m_ctrl->enable && dma3.m_ctrl->start_timing == DMAStartTiming::HBlank) {
		dma_resume<3>();
	}
}

void ARM7TDMI::dma_request_fifoA() {
	constexpr const uint32 address = 0x040000A0;
	DMAx<1>& dma1 = io().dma1;
	DMAx<2>& dma2 = io().dma2;

	if(dma1.m_ctrl->enable && dma1.m_ctrl->start_timing == DMAStartTiming::Special && *dma1.m_destination == address) {
		dma_resume<1>();
	}

	if(dma2.m_ctrl->enable && dma2.m_ctrl->start_timing == DMAStartTiming::Special && *dma2.m_destination == address) {
		dma_resume<2>();
	}
}

void ARM7TDMI::dma_request_fifoB() {
	constexpr const uint32 address = 0x040000A4;
	DMAx<1>& dma1 = io().dma1;
	DMAx<2>& dma2 = io().dma2;

	if(dma1.m_ctrl->enable && dma1.m_ctrl->start_timing == DMAStartTiming::Special && *dma1.m_destination == address) {
		dma_resume<1>();
	}

	if(dma2.m_ctrl->enable && dma2.m_ctrl->start_timing == DMAStartTiming::Special && *dma2.m_destination == address) {
		dma_resume<2>();
	}
}
template<unsigned int x>
bool ARM7TDMI::dma_is_running() {
	return io().template dma_for_num<x>().m_is_running;
}

template void ARM7TDMI::dma_on_enable<0>();
template void ARM7TDMI::dma_on_enable<1>();
template void ARM7TDMI::dma_on_enable<2>();
template void ARM7TDMI::dma_on_enable<3>();

template bool ARM7TDMI::dma_is_running<0>();
template bool ARM7TDMI::dma_is_running<1>();
template bool ARM7TDMI::dma_is_running<2>();
template bool ARM7TDMI::dma_is_running<3>();
