#pragma once

#define NOMINMAX

#include <cstdint>
#include <assert.h>
#include <QDebug>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

// some definitions that are helpful

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

// useless bits in registers
typedef bool emptyBit;

struct Debugging {
	static bool interpreter;
	static bool bios;
	static bool cd;
	static bool interrupt;
	static bool gpu;
	static bool gte;
};

template <class T>
struct Point{
	T x;
	T y;

	glm::ivec2 toIVec2() const {
		return { static_cast<i32>(x), static_cast<i32>(y) };
	}
};

struct Color {
	u8 r;
	u8 g;
	u8 b;

	glm::uvec3 toUVec3() const {
		return { static_cast<u32>(r), static_cast<u32>(g), static_cast<u32>(b) };
	}
};

inline u8 bcdToHex(u8 val) {
	return ((val >> 4) * 10) + (val & 0xF);
}

inline u8 hexToBcd(u8 val) {
	return ((val / 10) << 4) | (val % 10);
}

static void throw_error(const char* error) {
	fflush(stdout);
	qFatal(error);
}