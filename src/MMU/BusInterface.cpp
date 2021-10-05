#include <iostream>
#include "MMU/BusInterface.hpp"
#include "MMU/BusDevice.hpp"

Vector<BusDevice*> BusInterface::s_devices {};

//#define SPLIT_DEBUG


void BusInterface::debug() {
	log("Device dump:");
	for(auto& device: s_devices) {
		log("Device @{}: start={:08x} end={:08x} (size={})", (void*)device, device->start(), device->end(), device->size());
	}
}

bool BusInterface::register_device(BusDevice& dev) {
	for(auto& device : s_devices) {
		bool device_overlaps_center = dev.start() >= device->start() && dev.end() <= device->end();

		if(device_overlaps_center) {
			log("Registering device start={:08x} end={:08x} that overlaps with device start={:08x} end={:08x}\n",
				dev.start(), dev.end(),
			    device->start(), device->end());
		}
//		assert(!device_overlaps_center);
	}

	log("Register device {}, {:08x}-{:08x}\n", (void*)&dev,dev.start(), dev.end());
	s_devices.push_back(&dev);
	std::sort(s_devices.begin(), s_devices.end(), [](BusDevice*& a, BusDevice*& b) {
		return a->start() < b->start();
	});
	return false;
}

uint32 ensure_align(uint32 address, uint8 alignment) {
    uint32 ret = address;
    if(address % alignment != 0) {
        ret = address - (address%alignment);
//        std::cerr << "MMU/ Unaligned address " << std::hex << address << std::dec << " for alignment " << std::to_string(alignment)
//                  << "! Quietly correcting to " << std::hex << ret << std::dec << "\n";
//        ASSERT_NOT_REACHED();
    }

    return ret;
}

uint32 BusInterface::read32(uint32 address) {
	address = ensure_align(address, 4);
//	address = _handle_memory_images(address);

	uint32 data {0};
	uint32 read = 0;
	while(read < 4) {
		const auto current_address = address + read;

		auto* dev = find_device(current_address);
		if(!dev) {
			this->log("Undefined read to {:08x}", current_address);
			data |= (0xFF << read*8);
			read++;
			continue;
		}

		const auto address_in_device = (current_address - dev->start());
		if(read == 0 && dev->size() >= 4) {
			m_last_wait_cycles = dev->waitcycles32();
			data = dev->read32(address_in_device);
#ifdef SPLIT_DEBUG
			log("read32 from {} -> {:08x}", (void*)dev, data);
#endif
			read += 4;
		} else if((read % 2 == 0) && dev->size() >= 2) {
			m_last_wait_cycles = dev->waitcycles32();
			uint16 read_value = dev->read16(address_in_device);
#ifdef SPLIT_DEBUG
			log("read16 from {} -> {:04x}", (void*)dev, data);
#endif
			data |= (read_value << (read*8));
			read += 2;
		} else {
			m_last_wait_cycles = dev->waitcycles32();
			uint8 read_value = dev->read8(address_in_device);
#ifdef SPLIT_DEBUG
			log("read8 from {} -> {:02x}", (void*)dev, read_value);
#endif
			data |= (read_value << (read*8));
			read += 1;
		}
	}

	return data;
}

uint16 BusInterface::read16(uint32 address) {
	address = ensure_align(address, 2);
//	address = _handle_memory_images(address);

	uint16 data {0};
	uint32 read = 0;
	while(read < 2) {
		const auto current_address = address + read;

		auto* dev = find_device(current_address);
		if(!dev) {
			this->log("Undefined read to {:08x}", current_address);
			data |= (0xFF << read*8);
			read++;
			continue;
		}

		const auto address_in_device = (current_address - dev->start());
		if(read == 0 && dev->size() >= 2) {
			m_last_wait_cycles = dev->waitcycles16();
			data = dev->read16(address_in_device);
#ifdef SPLIT_DEBUG
			log("read16 from {} -> {:04x}", (void*)dev, data);
#endif
			read += 2;
		} else {
			m_last_wait_cycles = dev->waitcycles16();
			uint8 read_value = dev->read8(address_in_device);
#ifdef SPLIT_DEBUG
			log("read8 from {} -> {:02x}", (void*)dev, read_value);
#endif
			data |= (read_value << (read*8));
			read += 1;
		}
	}

	return data;
}

uint8 BusInterface::read8(uint32 address) {
//	address = _handle_memory_images(address);

	auto* dev = find_device(address);
	if(!dev) {
		this->log("Undefined read8 from {:08x}", address);
		return 0xFF;
	}

	m_last_wait_cycles = dev->waitcycles8();
	return dev->read8(address - dev->start());
}

void BusInterface::write32(uint32 address, uint32 value) {
	address = ensure_align(address, 4);
//	address = _handle_memory_images(address);

//	log("Write32 {:08x} <- {:08x}", address, value);

	uint32 written = 0;
	while(written < 4) {
		const auto current_address = address + written;

		auto* dev = find_device(current_address);
		if(!dev) {
			this->log("Undefined write32 to {:08x}, word: {:08x}", current_address, value);
			written++;
			continue;
		}

		const auto address_in_device = (current_address - dev->start());
		if(written == 0 && dev->size() >= 4) {
#ifdef SPLIT_DEBUG
			log("write32 to {} <- {:08x}", (void*)dev, value);
#endif
			m_last_wait_cycles = dev->waitcycles32();
			dev->write32(address_in_device, value);
			written += 4;
		} else if((written % 2 == 0) && dev->size() >= 2) {
			uint16 write_value = (value >> written*8) & 0xFFFFu;
#ifdef SPLIT_DEBUG
			log("write16 to {} <- {:04x}, written: {}", (void*)dev, write_value, written);
#endif
			m_last_wait_cycles = dev->waitcycles32();
			dev->write16(address_in_device, write_value);
			written += 2;
		} else {
			uint8 write_value = (value >> written*8) & 0xFFu;
#ifdef SPLIT_DEBUG
			log("write8 to {} <- {:02x}", (void*)dev, write_value);
#endif
			m_last_wait_cycles = dev->waitcycles32();
			dev->write8(address_in_device, write_value);
			written += 1;
		}
	}
}

void BusInterface::write16(uint32 address, uint16 value) {
	address = ensure_align(address, 2);
//	address = _handle_memory_images(address);

//	log("Write16 {:08x} <- {:04x}", address, value);

	uint32 written = 0;
	while(written < 2) {
		const auto current_address = address + written;

		auto* dev = find_device(current_address);
		if(!dev) {
			this->log("Undefined write16 to {:08x}, word: {:08x}", current_address, value);
			written++;
			continue;
		}

		const auto address_in_device = (current_address - dev->start());
		if(written == 0 && dev->size() >= 2) {
#ifdef SPLIT_DEBUG
			log("write16 to {} <- {:04x}", (void*)dev, value);
#endif
			m_last_wait_cycles = dev->waitcycles16();
			dev->write16(address_in_device, value);
			written += 2;
		} else {
			uint8 write_value = (value >> written*8) & 0xFFu;
#ifdef SPLIT_DEBUG
			log("write8 to {} <- {:02x}", (void*)dev, write_value);
#endif
			m_last_wait_cycles = dev->waitcycles16();
			dev->write8(address_in_device, write_value);
			written += 1;
		}
	}
}

void BusInterface::write8(uint32 address, uint8 value) {
//	address = _handle_memory_images(address);

//	log("Write8 {:08x} <- {:02x}", address, value);

	auto* dev = find_device(address);
	if(!dev) {
//		this->log("Undefined write8 to {:08x}, byte: {:02x}", address, value);
		return;
	}

	m_last_wait_cycles = dev->waitcycles8();
	dev->write8(address - dev->start(), value);
}

uint8 BusInterface::peek(uint32 address) {
//	address = _handle_memory_images(address);

	auto* dev = find_device(address);
	if(!dev) {
		return 0xFF;
	}

	return dev->read8(address - dev->start());
}

BusDevice* BusInterface::find_device(uint32 address) {
	static BusDevice* cache {nullptr};

	if(cache && cache->contains(address)) {
		cache_hit++;
		return cache;
	} else {
		cache_miss++;
	}

	for(auto& dev : s_devices) {
		if(dev->contains(address)) {
			cache = dev;
			return dev;
		}
	}
	return nullptr;
}


void BusInterface::poke(uint32 addr, uint8 val) {
	auto* dev = find_device(addr);
	if(!dev) return;
	dev->write8(addr - dev->start(), val);
}


void BusInterface::reload_all() {
	for(auto& dev : s_devices) {
		dev->reload();
	}
}
