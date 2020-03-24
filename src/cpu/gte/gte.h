#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "../../definitions.h"

typedef glm::mat<3,3,i16> Matrix;

// Implements the PS1 GTE (geometry transformation engine)
// which does all the 3D computations (the gpu only handles 2D drawing)
class GTE {
private:
	//data registers

	glm::i16vec3 vector[3];
	glm::u8vec4 rgbc;
	// z average value
	u16 otz;
	// intermediate value
	i16 ir[4];
	// screen XY pos
	glm::ivec3 screen[4];
	// Characteristic color
	glm::u8vec4 color[3];
	// prohibited value
	u32 res1;
	// sum of products
	i32 mac[4];
	// Leading zeros count source data
	i32 lzcs;
	// Leading zeros count result 
	u32 lzcr;

	Matrix rotation;
	glm::i32vec3 translation;
	// light source matrix
	Matrix light;
	Matrix lightColor;
	glm::i32vec3 backgroundColor;
	glm::i32vec3 farColor;
	glm::i32vec2 screenOffset;
	// Projection plane distance (H)
	u16 planeDistance;
	// DQA
	i16 depthQueingCoeff;
	// DQB
	i32 depthQueingOffset;
	// Z3 average scale factor (normally 1/3)
	i16 avgZ3;
	// Z4 average scale factor (normally 1/4)
	i16 avgZ4;

	// contains all calculations errors
	u32 flag;


	// Counts the number of leading zeros / ones if negative
	u32 countLeadingBits(i32 val);

public:
	u32 getData(u32 reg);
	void setData(u32 reg, u32 val);
	u32 getControl(u32 reg);
	void setControl(u32 reg, u32 val);
};