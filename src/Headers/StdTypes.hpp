#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <vector>
#include <list>
#include <memory>
#include <array>
#include <optional>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

#define kB (1024)
#define MB (1024 * 1024)

#define kBit (1024 / 8)
#define Mbit (1024 * 1024 / 8)

#define ASSERT_NOT_REACHED() assert(false && "ASSERT_NOT_REACHED failed");

template<class T>
using Vector = std::vector<T>;

template<class T>
using List = std::list<T>;

template<class T, auto size>
using Array = std::array<T, size>;

template<class T>
using Optional = std::optional<T>;

template<class T>
using Ref = std::reference_wrapper<T>;
