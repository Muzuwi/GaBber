#pragma once
#include <array>
#include <cassert>
#include "Headers/StdTypes.hpp"

template<const unsigned array_size>
class ReaderArray {
	std::array<uint8, array_size> m_array;
public:
	uint32 read32(uint32 offset) const {
		assert(offset < array_size && offset + 3 < array_size);
		return *reinterpret_cast<uint32*>((uint8*)&m_array[0] + offset);
	}

	uint16 read16(uint32 offset) const {
		assert(offset < array_size && offset + 1 < array_size);
		return *reinterpret_cast<uint16*>((uint8*)&m_array[0] + offset);
	}

	uint8 read8(uint32 offset) const {
		assert(offset < array_size);
		return m_array[offset];
	}

	void write32(uint32 offset, uint32 value) {
		assert(offset < array_size && offset + 3 < array_size);
		*reinterpret_cast<uint32*>((uint8*)&m_array[0] + offset) = value;
	}

	void write16(uint32 offset, uint16 value) {
		assert(offset < array_size && offset + 1 < array_size);
		*reinterpret_cast<uint16*>((uint8*)&m_array[0] + offset) = value;
	}

	void write8(uint32 offset, uint8 value) {
		assert(offset < array_size);
		m_array[offset] = value;
	}

	template<typename T>
	T readT(uint32 offset) {
		assert(offset < array_size && offset + sizeof(T) - 1 < array_size);
		return *reinterpret_cast<T*>((uint8*)&m_array[0] + offset);
	}

	std::array<uint8, array_size>& array() {
		return m_array;
	}

	constexpr unsigned size() const {
		return array_size;
	}
};