#pragma once
#include <cassert>
#include "BusDevice.hpp"

template<uint32 base_address, unsigned reg_size, typename RegType>
class __IORegister : public BusDevice {
protected:
	static_assert(reg_size == 4 || reg_size == 2 || reg_size == 1, "Unsupported register size");
	using T = RegType;

	T m_register;

	template<typename V>
	V _read_typed(uint32 offset) {
		constexpr unsigned struct_size = sizeof(V);
		assert(reg_size >= struct_size);
		assert(offset + struct_size <= reg_size);

		typedef __attribute__((may_alias)) T Source;
		typedef __attribute__((may_alias)) V Target;

		Source value = on_read();
		return *reinterpret_cast<Target const*>(reinterpret_cast<uint8 const*>(&value) + offset);
	}

	template<typename V>
	void _write_typed(uint32 offset, V value) {
		constexpr unsigned struct_size = sizeof(V);
		assert(reg_size >= struct_size);
		assert(offset + struct_size <= reg_size);

		typedef __attribute__((may_alias)) T Source;
		typedef __attribute__((may_alias)) V Target;

		Source new_value = m_register;
		*reinterpret_cast<Target*>(reinterpret_cast<uint8*>(&new_value) + offset) = value;
		this->on_write(new_value);
	}

	virtual void on_write(T new_value) { m_register = new_value; }

	virtual T on_read() { return m_register; }
public:
	__IORegister() noexcept
	    : BusDevice(base_address, base_address + reg_size)
	    , m_register() {
		this->reload();
	}

	T& operator*() { return m_register; }

	T const& operator*() const { return m_register; }

	template<class S>
	S* as() {
		static_assert(sizeof(S) == reg_size,
		              "Template argument for 'as<T>()' must be of the same size as the register.");
		return reinterpret_cast<S*>(&m_register);
	}

	template<class S>
	S const* as() const {
		static_assert(sizeof(S) == reg_size,
		              "Template argument for 'as<T>() const' must be of the same size as the register.");
		return reinterpret_cast<S const*>(&m_register);
	}

	uint32 read32(uint32 offset) override { return _read_typed<uint32>(offset); }

	uint16 read16(uint32 offset) override { return _read_typed<uint16>(offset); }

	uint8 read8(uint32 offset) override { return _read_typed<uint8>(offset); }

	void write32(uint32 offset, uint32 value) override { _write_typed<uint32>(offset, value); }

	void write16(uint32 offset, uint16 value) override { _write_typed<uint16>(offset, value); }

	void write8(uint32 offset, uint8 value) override { _write_typed<uint8>(offset, value); }

	void reload() override { m_register = T {}; }
};

template<uint32 base_address>
using IOReg32 = __IORegister<base_address, 4, uint32>;

template<uint32 base_address>
using IOReg16 = __IORegister<base_address, 2, uint16>;

template<uint32 base_address>
using IOReg8 = __IORegister<base_address, 1, uint8>;
