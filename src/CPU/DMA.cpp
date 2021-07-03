#include "Headers/ARM7TDMI.hpp"
#include "CPU/DMA.hpp"


bool ARM7TDMI::dma_cycle_all() {
	dma_cycle<0>();
	if(dma_is_running<0>())
		return true;

	dma_cycle<1>();
	if(dma_is_running<1>())
		return true;

	dma_cycle<2>();
	if(dma_is_running<2>())
		return true;

	dma_cycle<3>();
	if(dma_is_running<3>())
		return true;

	return false;
}


template<unsigned int x>
void ARM7TDMI::dma_cycle() {
	DMAx<x>& s = m_dma.get_data<x>();

	if(!s.m_ctrl.reg().enable)
		return;
	if(!s.m_is_running && (s.m_ctrl.reg().start_timing == DMAStartTiming::Immediate))
		dma_start<x>();
	if(!s.m_is_running)
		return;

	//  Read cycle
	if(!s.m_fetched) {
		s.m_fetched_data = (s.m_ctrl.reg().transfer_size) ? mem_read32(s.m_source_ptr)
		                                                  : mem_read16(s.m_source_ptr);

		DMASrcCtrl ctl = s.m_ctrl.reg().src_ctl;
		if(ctl == DMASrcCtrl::Increment)
			s.m_source_ptr += s.m_ctrl.reg().transfer_size ? 4 : 2;
		else if(ctl == DMASrcCtrl::Decrement)
			s.m_source_ptr -= s.m_ctrl.reg().transfer_size ? 4 : 2;

		s.m_fetched = true;

		return;
	}
	//  Write cycle
	else {
		if(s.m_ctrl.reg().transfer_size) {
			mem_write32(s.m_destination_ptr, s.m_fetched_data);
		} else {
			mem_write16(s.m_destination_ptr, static_cast<uint16>(s.m_fetched_data));
		}

		DMADestCtrl ctl = s.m_ctrl.reg().dest_ctl;
		if(ctl == DMADestCtrl::Increment || ctl == DMADestCtrl::Reload)
			s.m_destination_ptr += s.m_ctrl.reg().transfer_size ? 4 : 2;
		else if(ctl == DMADestCtrl::Decrement)
			s.m_destination_ptr -= s.m_ctrl.reg().transfer_size ? 4 : 2;

		s.m_count--;
		s.m_fetched = false;
	}

	if(s.m_count == 0) {
//		fmt::print("DMA{}: Transfer finished\n", x);
		s.m_is_running = false;
		s.m_fetched = false;
		s.m_finished = true;

		if(s.m_ctrl.reg().irq_on_finish)
			raise_irq(DMA::irq_type<x>());

		if(!s.m_ctrl.reg().repeat)
			s.m_ctrl.reg().enable = false;

		if(s.m_ctrl.reg().dest_ctl == DMADestCtrl::Reload) {
			s.m_destination_ptr = s.m_original_destination_ptr;
		}
	}
}


template<unsigned int x>
void ARM7TDMI::dma_start() {
	DMAx<x>& s = m_dma.get_data<x>();

	//  FIXME: Split loading SAD,DAD,CTN_L from actually starting the dma
	s.m_is_running = true;
	s.m_finished = false;

	s.m_source_ptr = s.m_source.raw();
	s.m_destination_ptr = s.m_destination.raw();
	s.m_original_destination_ptr = s.m_destination.raw();

	s.m_count = s.m_ctrl.reg().word_count;
	if(s.m_count == 0)
		s.m_count = DMA::max_count<x>();

//	fmt::print("DMA{}: Transfer started - src={:08x}, dest={:08x}, size={}\n", x, m_source.raw(), m_destination.raw(), (m_ctl.reg().transfer_size ? 4 : 2) * m_ctl.reg().word_count);
}


//  FIXME: These probably don't work at all

void ARM7TDMI::dma_start_vblank() {
	DMAx<0>& dma0 = m_dma.m_dma0;
	DMAx<1>& dma1 = m_dma.m_dma1;
	DMAx<2>& dma2 = m_dma.m_dma2;
	DMAx<3>& dma3 = m_dma.m_dma3;

	if(dma0.m_ctrl.reg().enable && dma0.m_ctrl.reg().start_timing == DMAStartTiming::VBlank) {
		dma_start<0>();
		return;
	}

	if(dma1.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::VBlank) {
		dma_start<1>();
		return;
	}

	if(dma2.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::VBlank) {
		dma_start<2>();
		return;
	}

	if(dma3.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::VBlank) {
		dma_start<3>();
		return;
	}
}

void ARM7TDMI::dma_start_hblank() {
	DMAx<0>& dma0 = m_dma.m_dma0;
	DMAx<1>& dma1 = m_dma.m_dma1;
	DMAx<2>& dma2 = m_dma.m_dma2;
	DMAx<3>& dma3 = m_dma.m_dma3;

	if(dma0.m_ctrl.reg().enable && dma0.m_ctrl.reg().start_timing == DMAStartTiming::HBlank) {
		dma_start<0>();
		return;
	}

	if(dma1.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::HBlank) {
		dma_start<1>();
		return;
	}

	if(dma2.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::HBlank) {
		dma_start<2>();
		return;
	}

	if(dma3.m_ctrl.reg().enable && dma1.m_ctrl.reg().start_timing == DMAStartTiming::HBlank) {
		dma_start<3>();
		return;
	}
}
