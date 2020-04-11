#pragma once

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE 
#include <glm/glm.hpp>

#include "../../definitions.h"

typedef glm::tmat3x3<i64> Matrix;

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
			u8 cmd : 6;
			emptyBit _useless0 : 4;
			bool saturateIR : 1;
			emptyBit _useless1 : 2;
			u8 translationVec : 2;
			u8 multiplyVec : 2;
			u8 multiplyMat : 2;
			bool shiftIR : 1;
			u8 fakeGTE : 5;
			emptyBit _useless2 : 7;
		} content;

		u32 val;
	} tempCmd;
	void rtps(int i);
	void rtpt();
	void nclip();
	void ncds(int i);
	void avsz3();

public:
	u32 getData(u32 reg);
	void setData(u32 reg, u32 val);
	u32 getControl(u32 reg);
	void setControl(u32 reg, u32 val);

	void sendCmd(u32 cmd);
};
