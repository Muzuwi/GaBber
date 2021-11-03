#pragma once
#include "Headers/StdTypes.hpp"

enum class BackupCartType {
	EEPROM,
	SRAM32K,
	FLASH64K,
	FLASH128K,
};

class BackupCart {
public:
	explicit BackupCart()
	{ }

	virtual ~BackupCart() = default;

	virtual uint8 read(uint32 offset) = 0;
	virtual void write(uint32 offset, uint8 value) = 0;

	virtual void from_vec(Vector<uint8>&&) = 0;
	virtual Vector<uint8> const& to_vec() = 0;

};
