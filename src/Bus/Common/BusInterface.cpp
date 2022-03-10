#include "BusInterface.hpp"
#include <iostream>
#include "Bus/IO/IOContainer.hpp"
#include "BusDevice.hpp"
#include "Debugger/Debugger.hpp"
#include "Emulator/GaBber.hpp"

BusInterface::BusInterface(GaBber& emu)
    : Module(emu) {}

void BusInterface::debug() {
	log("Device dump:");
	for(auto& device : m_devices) {
		log("Device @{}: start={:08x} end={:08x} (size={})", (void*)device, device->start(), device->end(),
		    device->size());
	}
}

bool BusInterface::register_device(BusDevice& dev) {
	for(auto& device : m_devices) {
		bool device_overlaps_center = dev.start() >= device->start() && dev.end() <= device->end();

		if(device_overlaps_center) {
			log("Registering device start={:08x} end={:08x} that overlaps with device start={:08x} end={:08x}\n",
			    dev.start(), dev.end(), device->start(), device->end());
		}
		//		assert(!device_overlaps_center);
	}

	log("Register device {}, {:08x}-{:08x}\n", (void*)&dev, dev.start(), dev.end());
	m_devices.push_back(&dev);
	std::sort(m_devices.begin(), m_devices.end(), [](BusDevice*& a, BusDevice*& b) { return a->start() < b->start(); });
	return false;
}

uint32 ensure_align(uint32 address, uint8 alignment) {
	uint32 ret = address;
	if(address % alignment != 0) {
		ret = address - (address % alignment);
		//  std::cerr << "MMU/ Unaligned address " << std::hex << address << std::dec << " for alignment " <<
		//  std::to_string(alignment)
		//            << "! Quietly correcting to " << std::hex << ret << std::dec << "\n";
		//  ASSERT_NOT_REACHED();
	}

	return ret;
}

uint32 BusInterface::read32(uint32 address) {
	address = ensure_align(address, 4);

	auto dev = find_device(address, 4);
	if(!dev) {
		const uint16 lower = read16(address);
		const uint16 higher = read16(address + 2);
		return (static_cast<uint32>(higher) << 16u) | static_cast<uint32>(lower);
	}

	m_last_wait_cycles = dev->waitcycles32();
	return dev->read32(address - dev->start());
}

void BusInterface::write32(uint32 address, uint32 value) {
	address = ensure_align(address, 4);

	auto dev = find_device(address, 4);
	if(!dev) {
		write16(address, value & 0xFFFFu);
		write16(address + 2, value >> 16u);
		return;
	}

	m_last_wait_cycles = dev->waitcycles32();
	dev->write32(address - dev->start(), value);
}

uint16 BusInterface::read16(uint32 address) {
	address = ensure_align(address, 2);

	auto dev = find_device(address, 2);
	if(!dev) {
		const uint8 lower = read8(address);
		const uint8 higher = read8(address + 1);
		return (static_cast<uint16>(higher) << 8u) | static_cast<uint16>(lower);
	}

	m_last_wait_cycles = dev->waitcycles16();
	return dev->read16(address - dev->start());
}

void BusInterface::write16(uint32 address, uint16 value) {
	address = ensure_align(address, 2);

	auto dev = find_device(address, 2);
	if(!dev) {
		write8(address, value & 0xFFu);
		write8(address + 1, value >> 8u);
		return;
	}

	m_last_wait_cycles = dev->waitcycles16();
	dev->write16(address - dev->start(), value);
}

uint8 BusInterface::read8(uint32 address) {
	auto* dev = find_device(address, 1);
	if(!dev) {
		this->log("Undefined byte read from {:08x}", address);
		debugger().on_undefined_access(address);
		return 0xFF;
	}

	m_last_wait_cycles = dev->waitcycles8();
	return dev->read8(address - dev->start());
}

void BusInterface::write8(uint32 address, uint8 value) {
	auto* dev = find_device(address, 1);
	if(!dev) {
		log("Undefined byte write to {:08x}, byte: {:02x}", address, value);
		debugger().on_undefined_access(address);
		return;
	}

	m_last_wait_cycles = dev->waitcycles8();
	dev->write8(address - dev->start(), value);
}

BusDevice* BusInterface::find_device(uint32 address, size_t size) {
	static BusDevice* cache { nullptr };
	if(cache && cache->contains(address) && cache->contains(address + size - 1)) {
		return cache;
	}

	for(auto& dev : m_devices) {
		if(dev->contains(address) && dev->contains(address + size - 1)) {
			cache = dev;
			return dev;
		}
	}
	return nullptr;
}

uint8 BusInterface::peek(uint32 address) {
	auto* dev = find_device(address, 1);
	if(!dev) {
		return 0xFF;
	}

	return dev->read8(address - dev->start());
}

void BusInterface::poke(uint32 addr, uint8 val) {
	auto* dev = find_device(addr, 1);
	if(!dev) {
		return;
	}
	dev->write8(addr - dev->start(), val);
}

void BusInterface::reload() {
	for(auto& dev : m_devices) {
		dev->reload();
	}
}

unsigned BusInterface::waits32(uint32 address, AccessType type) {
	switch(region_from_address(address)) {
		case Region::BIOS:
		case Region::IWRAM:
		case Region::IO:
		case Region::OAM: {
			return 1;
		}
		case Region::WRAM: {
			//  FIXME: TODO
			return 6;
		}
		case Region::VRAM:
		case Region::PAL: {
			return 2;
		}
		case Region::PAKROM0: {
			return waits16(address, type) + waits16(address + 2, AccessType::Seq);
		}
		case Region::PAKROM1: {
			return waits16(address, type) + waits16(address + 2, AccessType::Seq);
		}
		case Region::PAKROM2: {
			return waits16(address, type) + waits16(address + 2, AccessType::Seq);
		}
		case Region::PAKSRAM: {
			return io().waitctl.sram_wait();
		}
		default: ASSERT_NOT_REACHED();
	}
}

unsigned BusInterface::waits16(uint32 address, AccessType type) {
	switch(region_from_address(address)) {
		case Region::BIOS:
		case Region::IWRAM:
		case Region::IO:
		case Region::OAM: {
			return 1;
		}
		case Region::WRAM: {
			//  FIXME: TODO
			return 3;
		}
		case Region::VRAM:
		case Region::PAL: {
			return 1;
		}
		case Region::PAKROM0: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait0_sequential() + 1
			                               : io().waitctl.wait0_nonsequential() + 1;
		}
		case Region::PAKROM1: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait1_sequential() + 1
			                               : io().waitctl.wait1_nonsequential() + 1;
		}
		case Region::PAKROM2: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait2_sequential() + 1
			                               : io().waitctl.wait2_nonsequential() + 1;
		}
		case Region::PAKSRAM: {
			return io().waitctl.sram_wait() + 1;
		}
		default: ASSERT_NOT_REACHED();
	}
}

unsigned BusInterface::waits8(uint32 address, AccessType type) {
	switch(region_from_address(address)) {
		case Region::BIOS:
		case Region::IWRAM:
		case Region::IO:
		case Region::OAM: {
			return 1;
		}
		case Region::WRAM: {
			//  FIXME: TODO
			return 3;
		}
		case Region::VRAM:
		case Region::PAL: {
			return 1;
		}
		case Region::PAKROM0: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait0_sequential() + 1
			                               : io().waitctl.wait0_nonsequential() + 1;
		}
		case Region::PAKROM1: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait1_sequential() + 1
			                               : io().waitctl.wait1_nonsequential() + 1;
		}
		case Region::PAKROM2: {
			if((address % 128 * kB) == 0) {
				type = AccessType::NonSeq;
			}
			return type == AccessType::Seq ? io().waitctl.wait2_sequential() + 1
			                               : io().waitctl.wait2_nonsequential() + 1;
		}
		case Region::PAKSRAM: {
			return io().waitctl.sram_wait() + 1;
		}
		default: ASSERT_NOT_REACHED();
	}
}
