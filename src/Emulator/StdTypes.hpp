#pragma once
#include <cstddef>
#include <cstdint>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

#define kB (1024)
#define MB (1024 * 1024)

#define ASSERT_NOT_REACHED() __builtin_unreachable()
