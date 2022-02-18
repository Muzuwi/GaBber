#include "DMA.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Emulator/Bits.hpp"
#include "Emulator/GaBber.hpp"

template<unsigned int x>
uint32 DMACtrl<x>::on_read() {
	return this->m_register & readable_mask;
}

template<unsigned int x>
void DMACtrl<x>::on_write(uint32 new_value) {
	const uint32 old_value = this->m_register;
	this->m_register = new_value & writable_mask;

	//  When enable bit is toggled (0 -> 1), refresh the internal registers
	//  The enable bit is bit 31, as the control reg is merged with the word count reg
	if(!Bits::bit<31>(old_value) && Bits::bit<31>(this->m_register)) {
		this->cpu().template dma_on_enable<x>();
	}
}

template<unsigned int x>
uint32 DMASrc<x>::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABEBABE;
}

template<unsigned int x>
void DMASrc<x>::on_write(uint32 new_value) {
	this->m_register = new_value & DMA::source_mask<x>();
}

template<unsigned int x>
uint32 DMADest<x>::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABEBABE;
}
template<unsigned int x>
void DMADest<x>::on_write(uint32 new_value) {
	this->m_register = new_value & DMA::destination_mask<x>();
}

template uint32 DMACtrl<0>::on_read();
template uint32 DMACtrl<1>::on_read();
template uint32 DMACtrl<2>::on_read();
template uint32 DMACtrl<3>::on_read();
template void DMACtrl<0>::on_write(uint32 new_value);
template void DMACtrl<1>::on_write(uint32 new_value);
template void DMACtrl<2>::on_write(uint32 new_value);
template void DMACtrl<3>::on_write(uint32 new_value);
template uint32 DMASrc<0>::on_read();
template uint32 DMASrc<1>::on_read();
template uint32 DMASrc<2>::on_read();
template uint32 DMASrc<3>::on_read();
template void DMASrc<0>::on_write(uint32 new_value);
template void DMASrc<1>::on_write(uint32 new_value);
template void DMASrc<2>::on_write(uint32 new_value);
template void DMASrc<3>::on_write(uint32 new_value);
template uint32 DMADest<0>::on_read();
template uint32 DMADest<1>::on_read();
template uint32 DMADest<2>::on_read();
template uint32 DMADest<3>::on_read();
template void DMADest<0>::on_write(uint32 new_value);
template void DMADest<1>::on_write(uint32 new_value);
template void DMADest<2>::on_write(uint32 new_value);
template void DMADest<3>::on_write(uint32 new_value);
