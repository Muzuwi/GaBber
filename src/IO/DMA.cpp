#include "IO/DMA.hpp"
#include "Headers/Bits.hpp"
#include "Headers/GaBber.hpp"

template<unsigned int x>
void DMACtrl<x>::on_write(uint32 new_value) {
	const uint32 old_value = this->m_register;
	this->m_register = new_value & writable_mask;

	//  When enable bit is toggled (0 -> 1), refresh the internal registers
	//  The enable bit is bit 31, as the control reg is merged with the word count reg
	if(!Bits::bit<31>(old_value) && Bits::bit<31>(this->m_register)) {
		GaBber::instance().cpu().dma_on_enable<x>();
	}
}

template void DMACtrl<0>::on_write(uint32 new_value);
template void DMACtrl<1>::on_write(uint32 new_value);
template void DMACtrl<2>::on_write(uint32 new_value);
template void DMACtrl<3>::on_write(uint32 new_value);
