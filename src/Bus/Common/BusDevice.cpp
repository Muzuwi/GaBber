#include "BusDevice.hpp"
#include "BusInterface.hpp"

BusDevice::BusDevice(uint32 start, uint32 end) noexcept
    : m_start(start)
    , m_end(end) {
	BusInterface::register_device(*this);
}
