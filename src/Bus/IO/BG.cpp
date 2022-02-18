#include "BG.hpp"

template<unsigned int address>
uint16 OffsetReg<address>::on_read() {
	//  FIXME: unreadable I/O register
	return 0xBABE;
}

template<unsigned int address>
void OffsetReg<address>::on_write(uint16 val) {
	this->m_register = val & 0b111111111;
}

template<unsigned int x, unsigned int address>
uint16 BGCNT<x, address>::on_read() {
	return this->m_register;
}

template<unsigned int x, unsigned int address>
void BGCNT<x, address>::on_write(uint16 val) {
	constexpr const uint32 mask = (x == 0 || x == 1) ? 0xDFFF : 0xFFFF;
	this->m_register = val & mask;
}

template uint16 OffsetReg<67108880u>::on_read();
template void OffsetReg<67108880u>::on_write(uint16);
template uint16 OffsetReg<67108882u>::on_read();
template void OffsetReg<67108882u>::on_write(uint16);
template uint16 OffsetReg<67108884u>::on_read();
template void OffsetReg<67108884u>::on_write(uint16);
template uint16 OffsetReg<67108886u>::on_read();
template void OffsetReg<67108886u>::on_write(uint16);
template uint16 OffsetReg<67108888u>::on_read();
template void OffsetReg<67108888u>::on_write(uint16);
template uint16 OffsetReg<67108890u>::on_read();
template void OffsetReg<67108890u>::on_write(uint16);
template uint16 OffsetReg<67108892u>::on_read();
template void OffsetReg<67108892u>::on_write(uint16);
template uint16 OffsetReg<67108894u>::on_read();
template void OffsetReg<67108894u>::on_write(uint16);

template uint16 BGCNT<0, 67108872u>::on_read();
template void BGCNT<0, 67108872u>::on_write(uint16);
template uint16 BGCNT<1, 67108874u>::on_read();
template void BGCNT<1, 67108874u>::on_write(uint16);
template uint16 BGCNT<2, 67108876u>::on_read();
template void BGCNT<2, 67108876u>::on_write(uint16);
template uint16 BGCNT<3, 67108878u>::on_read();
template void BGCNT<3, 67108878u>::on_write(uint16);
