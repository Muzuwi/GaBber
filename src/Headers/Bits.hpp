#pragma once
#include "Headers/StdTypes.hpp"

namespace Bits {
	/*
	 *  Sign-extends an N-bit signed integer to a 32-bit signed integer
	 */
	template<size_t N>
	constexpr uint32 sign_extend(uint32 value) {
		static_assert(N < sizeof(uint32) * 8);

		const auto mask = ((1 << N) - 1);
		const auto shift_count = 32 - N;

		auto u = (static_cast<uint32>(value) & mask);
		return ((signed int)(u << shift_count)) >> shift_count;
	}

	/*
	 *  Rotates the specified 32-bit value right 'c' times
	 */
	inline uint32 rotr32 (uint32 n, unsigned int c) {
		const unsigned int mask = (std::numeric_limits<uint32>::digits - 1);
		c &= mask;
		return (n>>c) | (n<<( (-c)&mask ));
	}

	/*
	 *  Gets the n-th bit of the specified 32-bit value
	 */
	template<unsigned n>
	constexpr bool bit(uint32 val) {
		static_assert(n >= 0 && n < 32);
		return val & (1u << n);
	}

	/*
	 *  Gets the n-th byte of the specified 32-bit value, endianness is assumed to be little.
	 */
	template<unsigned n>
	constexpr uint8 byte_le(uint32 val) {
		static_assert(n < 4);
		return (val >> (n*8)) & 0xFFu;
	}
}