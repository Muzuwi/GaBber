#include "BusDevice.hpp"
#include "BusInterface.hpp"

BusDevice::BusDevice(GaBber& emu, uint32 start, uint32 end) noexcept
    : Module(emu)
    , m_start(start)
    , m_end(end) {
	bus().register_device(*this);
}
