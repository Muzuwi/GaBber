#pragma once
#include "MMU/BusDevice.hpp"
#include "Headers/StdTypes.hpp"

template<unsigned address_base, unsigned address_end, unsigned msize>
class ArrayMem : public BusDevice {
protected:
	Array<uint8, msize> m_buffer;
private:
	template<unsigned size>
	IOAccess access_for_size() const {
		if constexpr(size == 4)
			return access32();
		else if constexpr(size == 2)
			return access16();
		else if constexpr(size == 1)
			return access8();
		else
			return access8();
	}


	template<class R>
	inline Optional<R> read_safe(uint32 offset) const {
		if (access_for_size<sizeof(R)>() == IOAccess::W) {
			fmt::print("\u001b[91mArrayMem{{0x{:08x}}}: Unsupported read{} from device\u001b[0m\n", address_base, sizeof(R)*8);
			return {};
		}

		offset = handle_offset(offset, sizeof(R));
		if(offset < msize && offset + sizeof(R) <= msize) {
			return {*reinterpret_cast<R const*>(&m_buffer[offset])};
		} else {
			fmt::print("\u001b[91mArrayMem{{0x{:08x}}}: Out of bounds read{} for offset={} size={}\u001b[0m\n", address_base, sizeof(R)*8, offset, sizeof(R));
			fmt::print("\tHint: The read occured {} bytes {} the container end \n", (offset > msize) ? (offset-msize) : (msize-offset), (offset > msize) ? "past" : "before");
			return {};
		}
	}


	template<class R>
	inline void write_safe(uint32 offset, R val) {
		if (access_for_size<sizeof(R)>() == IOAccess::R) {
			fmt::print("\u001b[91mArrayMem{{0x{:08x}}}: Unsupported write{} to device\u001b[0m\n", address_base, sizeof(R)*8);
			return;
		}

		offset = handle_offset(offset, sizeof(R));
		if(offset < msize && offset + sizeof(R) <= msize) {
			*reinterpret_cast<R*>(&m_buffer[offset]) = val;
		} else {
			fmt::print("\u001b[91mArrayMem{{0x{:08x}}}: Out of bounds write{} for offset={} val={:x}\u001b[0m\n", address_base, sizeof(R)*8, offset, val);
			fmt::print("\tHint: The write occured {} bytes {} the container end \n", (offset > msize) ? (offset-msize) : (msize-offset), (offset > msize) ? "past" : "before");
		}
	}

public:
	ArrayMem()
	: BusDevice(address_base, address_end) {
		std::memset(&m_buffer[0], 0x0, msize);
	}

	virtual uint32 handle_offset(uint32 offset, size_t) const {
		return offset;
	}

	virtual IOAccess access32() const = 0;
	virtual IOAccess access16() const = 0;
	virtual IOAccess access8() const = 0;

	template<class T>
	inline Optional<T> readT(uint32 offset) const {
		return read_safe<T>(offset);
	}

	uint32 read32(uint32 offset) override {
		const Optional<uint32> v = read_safe<uint32>(offset);
		if(!v.has_value())
			return (1ul << (32ul)) - 1;
		return *v;
	}

	uint16 read16(uint32 offset) override {
		const Optional<uint16> v = read_safe<uint16>(offset);
		if(!v.has_value())
			return (1ul << (16ul)) - 1;
		return *v;
	}

	uint8 read8(uint32 offset) override {
		const Optional<uint8> v = read_safe<uint8>(offset);
		if(!v.has_value())
			return (1ul << (8ul)) - 1;
		return *v;
	}

	void write32(uint32 offset, uint32 value) override {
		write_safe<uint32>(offset, value);
	}

	void write16(uint32 offset, uint16 value) override {
		write_safe<uint16>(offset, value);
	}

	void write8(uint32 offset, uint8 value) override {
		write_safe<uint8>(offset, value);
	}

	uint8 const* raw() const {
		return &m_buffer[0];
	}

	size_t bytes() const {
		return msize;
	}
};
