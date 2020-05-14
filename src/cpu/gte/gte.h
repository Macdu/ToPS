#pragma once

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE 
#include <glm/glm.hpp>

#include "../../definitions.h"

typedef glm::tmat3x3<i64> Matrix;

// all the flags that can be set when using GTE cmds
enum struct GTEFlags : u32 {
	IR0Overflow = 1 << 12,
	SY2Overflow = 1 << 13,
	SX2Overflow = 1 << 14,
	MAC0NegativeOverflow = 1 << 15,
	MAC0PositiveOverflow = 1 << 16,
	// happens on RTPS
	DivideOverflow = 1 << 17,
	SZ3Overflow = 1 << 18,
	BlueOverflow = 1 << 19,
	GreenOverflow = 1 << 20,
	RedOverflow = 1 << 21,
	// saturated to +0000h..+7FFFh (lm=1) or to -8000h..+7FFFh (lm=0)
	IR3Overflow = 1 << 22,
	IR2Overflow = 1 << 23,
	IR1Overflow = 1 << 24,
	MAC3NegativeOverflow = 1 << 25,
	MAC2NegativeOverflow = 1 << 26,
	MAC1NegativeOverflow = 1 << 27,
	MAC3PositiveOverflow = 1 << 28,
	MAC2PositiveOverflow = 1 << 29,
	MAC1PositiveOverflow = 1 << 30,
	// bit 31 is (Bit30..23, and 18..13 ORed together)
};

// Implements the PS1 GTE (geometry transformation engine)
// which does all the 3D computations (the gpu only handles 2D drawing)
class GTE {
private:
	//data registers

	glm::i64vec3 vector[3];
	glm::u8vec4 rgbc;
	// z average value
	u64 otz;
	// intermediate value
	glm::i64vec4 ir;
	// screen XY pos
	glm::i64vec3 screen[4];
	// Characteristic color
	glm::u8vec4 color[3];
	// prohibited value
	u64 res1;
	// sum of products
	glm::i64vec4 mac;
	// Leading zeros count source data
	i64 lzcs;
	// Leading zeros count result 
	u32 lzcr;

	Matrix rotation;
	glm::i64vec3 translation;
	// light source matrix
	Matrix light;
	Matrix lightColor;
	glm::i64vec3 backgroundColor;
	glm::i64vec3 farColor;
	glm::i64vec2 screenOffset;
	// Projection plane distance (H)
	u64 planeDistance;
	// DQA
	i64 depthQueingCoeff;
	// DQB
	i64 depthQueingOffset;
	// Z3 average scale factor (normally 1/3)
	i64 avgZ3;
	// Z4 average scale factor (normally 1/4)
	i64 avgZ4;

	// contains all calculations errors
	u32 flag;


	// Counts the number of leading zeros / ones if negative
	u32 countLeadingBits(i32 val);

	// store temporary the cmd sent to the gte
	union {
		struct {
			u32 cmd : 6;
			u32 : 4;
			u32 saturateIR : 1;
			u32 : 2;
			u32 translationVec : 2;
			u32 multiplyVec : 2;
			u32 multiplyMat : 2;
			u32 shiftIR : 1;
			u32 fakeGTE : 5;
			u32 : 7;
		} content;

		u32 val;
	} tempCmd;

	static_assert(sizeof(tempCmd) == sizeof(u32));

	// fast, but less accurate division mechanism (based on Unsigned Newton-Raphson (UNR) algorithm)
	// Returns (((H*20000h/SZ3)+1)/2) 
	u32 unrDivision();
	void rtps(int i);
	void rtpt();
	void nclip();
	void ncs(int i);
	void ncds(int i);
	void avsz3();

	// tools for flag handling
	// check this vector for saturation flags
	// then set mac and ir accordingly
	void checkVector(glm::i64vec3 vec);
	inline void setMacIr(int i, i64 val);
	inline i64 saturate(i64 val,i64 max, GTEFlags overflow,i64 min, GTEFlags underflow);

public:
	u32 getData(u32 reg);
	void setData(u32 reg, u32 val);
	u32 getControl(u32 reg);
	void setControl(u32 reg, u32 val);

	void sendCmd(u32 cmd);
};
