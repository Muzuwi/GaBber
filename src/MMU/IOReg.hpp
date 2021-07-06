#pragma once
#include <fmt/format.h>
#include "MMU/BusDevice.hpp"

template<typename T>
concept Register = requires(T v) {
	v.m_raw;
	v.m_reg;
	sizeof(v.m_raw) == sizeof(v.m_reg);
	sizeof(v.m_raw) > 0;
};

template<typename T>
union _DummyReg {
	T m_reg;
	T m_raw;
};

template<unsigned n>
struct __SizeStuff;

template<>
struct __SizeStuff<1> {
	typedef uint8 RawType;
};

template<>
struct __SizeStuff<2> {
	typedef uint16 RawType;
};

template<>
struct __SizeStuff<4> {
	typedef uint32 RawType;
};


template<uint32 base_address, class T, IOAccess access>
class IORegister : public BusDevice {
	T m_register;

	static_assert(sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1, "Unsupported register size");
	using RawType = __SizeStuff<sizeof(T)>;

	template<typename V>
	V _read_typed(uint32 offset) {
		if constexpr(sizeof(T) < sizeof(V) || access == IOAccess::W) {
			fmt::print("IOReg ${:08x}/ Incompatible read{} on register of size {}\n", this->start(), sizeof(V),
			           sizeof(T));
			return (1ul << (sizeof(V)*8)) - 1;
		} else {
			if (offset + sizeof(V) > sizeof(T)) {
				fmt::print("IOReg ${:08x}/ Out of bounds read{} on register of size {} with offset {:08x}\n",
				           this->start(), sizeof(V)*8, sizeof(T), offset);
				return (1ul << (sizeof(V)*8)) - 1;
			} else {
				//  FIXME: This is probably not portable
				return *reinterpret_cast<V*>(reinterpret_cast<uint8*>(&m_register.m_raw) + offset);
			}
		}
	}

	template<typename V>
	void _write_typed(uint32 offset, V value) {
		if constexpr(sizeof(T) < sizeof(V) || access == IOAccess::R) {
			fmt::print("IOReg ${:08x}/ Incompatible write{} on register of size {}\n", this->start(), sizeof(V),
			           sizeof(T));
			return;
		} else {
			if (offset + sizeof(V) > sizeof(T)) {
				fmt::print("IOReg ${:08x}/ Out of bounds write{} on register of size {} with offset {:08x}\n",
				           this->start(), sizeof(V)*8, sizeof(T), offset);
			} else {
				//  FIXME: This is probably not portable
				T new_value = m_register;
				*reinterpret_cast<V*>(reinterpret_cast<uint8*>(&new_value) + offset) = value;
				this->on_write(new_value);
			}
		}
	}
public:
	T& operator*() {
		return m_register;
	}

};













template<uint32 base_address, typename T, IOAccess access> requires Register<T>
class IOReg : public BusDevice {
protected:
	T m_register;

	using RawType = typeof(T::m_raw);
	using UnionType = typeof(T::m_reg);

	template<typename V>
	V _read_typed(uint32 offset) {
		if constexpr(sizeof(T) < sizeof(V) || access == IOAccess::W) {
			fmt::print("IOReg ${:08x}/ Incompatible read{} on register of size {}\n", this->start(), sizeof(V),
			           sizeof(T));
			return (1ul << (sizeof(V)*8)) - 1;
		} else {
			if (offset + sizeof(V) > sizeof(T)) {
				fmt::print("IOReg ${:08x}/ Out of bounds read{} on register of size {} with offset {:08x}\n",
				           this->start(), sizeof(V)*8, sizeof(T), offset);
				return (1ul << (sizeof(V)*8)) - 1;
			} else {
				//  FIXME: This is probably not portable
				return *reinterpret_cast<V*>(reinterpret_cast<uint8*>(&m_register.m_raw) + offset);
			}
		}
	}

	template<typename V>
	void _write_typed(uint32 offset, V value) {
		if constexpr(sizeof(T) < sizeof(V) || access == IOAccess::R) {
			fmt::print("IOReg ${:08x}/ Incompatible write{} on register of size {}\n", this->start(), sizeof(V),
			           sizeof(T));
			return;
		} else {
			if (offset + sizeof(V) > sizeof(T)) {
				fmt::print("IOReg ${:08x}/ Out of bounds write{} on register of size {} with offset {:08x}\n",
				           this->start(), sizeof(V)*8, sizeof(T), offset);
			} else {
				//  FIXME: This is probably not portable
				RawType new_value = m_register.m_raw;
				*reinterpret_cast<V*>(reinterpret_cast<uint8*>(&new_value) + offset) = value;
				this->on_write(new_value);
			}
		}
	}

public:
	IOReg(RawType val = {})
	: BusDevice(base_address, base_address+sizeof(T)), m_register() {
		m_register.m_raw = val;
	}

	uint32 read32(uint32 offset) override {
		return _read_typed<uint32>(offset);
	}

	uint16 read16(uint32 offset) override {
		return _read_typed<uint16>(offset);
	}

	uint8 read8(uint32 offset) override {
		return _read_typed<uint8>(offset);
	}

	void write32(uint32 offset, uint32 value) override {
		_write_typed<uint32>(offset, value);
	}

	void write16(uint32 offset, uint16 value) override {
		_write_typed<uint16>(offset, value);

	}

	void write8(uint32 offset, uint8 value) override {
		_write_typed<uint8>(offset, value);
	}

	RawType& raw() {
		return m_register.m_raw;
	}

	const RawType& raw() const {
		return m_register.m_raw;
	}

	UnionType& reg() {
		return m_register.m_reg;
	}

	const UnionType& reg() const {
		return m_register.m_reg;
	}

	T& union_type() {
		return m_register;
	}

	T const& union_type() const {
		return m_register;
	}

	virtual void on_write(RawType new_value) {
		m_register.m_raw = new_value;
	}

	void reload() override {
		m_register.m_raw = RawType {};
	}

	std::string identify() const override { return "I/O Register"; }

	IOReg& operator=(RawType const& val) {
		m_register.m_raw = val;
		return *this;
	}

};
