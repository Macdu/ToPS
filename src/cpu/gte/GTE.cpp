#include "GTE.h"

#include <intrin.h>
#include <algorithm>

#pragma intrinsic(_BitScanReverse)

u32 GTE::countLeadingBits(i32 val)
{
	if (val < 0)val = ~val;
	unsigned long res;
	// we use some c++ builtin functions on MSVC
	if (_BitScanReverse(&res, (u32)val)) {
		return 31 - res;
	}
	else {
		// val is 0
		return 32;
	}
}

void GTE::rtps(int i)
{
	if (Debugging::gte && tempCmd.content.cmd == 0x01)
		printf("GTE: RTPS\n");
	//TODO: handle flags
	glm::i64vec3 res = rotation * vector[i] + (translation << glm::i64vec3(12,12,12));
	i64 s3z = res.z >> 12;

	screen[0] = screen[1];
	screen[1] = screen[2];

	screen[2].z = screen[3].z;
	screen[3].z = s3z;

	if (tempCmd.content.shiftIR) {
		res >>= glm::i64vec3(12, 12, 12);
	}
	mac.yzw = res;
	ir.yzw = res;

	i64 planeComp = (((planeDistance << 17) / s3z) + 1) / 2;
	screen[2].x = (planeComp * ir[1] + screenOffset.x) >> 16;
	screen[2].y = (planeComp * ir[2] + screenOffset.y) >> 16;

	mac[0] = (planeComp * depthQueingCoeff + depthQueingOffset);
	ir[0] = mac[0] >> 12;
}

void GTE::rtpt()
{
	if (Debugging::gte)printf("GTE: RTPT\n");
	// rtpt repeats three times rtps
	for (int i = 0; i < 3; i++)
		rtps(i);
}

void GTE::nclip()
{
	if (Debugging::gte)printf("GTE: NCLIP\n");
	// nclip gives 
	// (screen0 - screen1)^(screen1 - screen2).z
	// which is the order the 3 vertices are
	auto v0 = screen[0] - screen[1];
	auto v1 = screen[1] - screen[2];
	mac[0] = v0.x * v1.y - v0.y * v1.x;
	
}

void GTE::ncds(int i)
{
	if (Debugging::gte && tempCmd.content.cmd == 0x13)
		printf("GTE: NCDS\n");

	// only used to set flags
	/*
	auto res1 = light * vector[i];
	auto res2 = backgroundColor << glm::i64vec3(12, 12, 12) 
		+ lightColor * *reinterpret_cast<glm::i64vec3*>(ir);
	*/
	glm::i64vec3 col = glm::i64vec3((i64)rgbc.r << 4, (i64)rgbc.g << 4, (i64)rgbc.b << 4);
	glm::i64vec3 fcShifted = farColor << glm::i64vec3(12, 12, 12);
	glm::i64vec3 ir0 = glm::i64vec3(ir[0], ir[0], ir[0]);
	auto res = col * ir.yzw + ir0 * (fcShifted - col * ir.yzw);
	if (tempCmd.content.shiftIR) {
		res >>= glm::i64vec3(12, 12, 12);
	}

	mac.yzw = res;
	ir.yzw = res;
	res >>= glm::i64vec3(4, 4, 4);
	color[0] = color[1];
	color[1] = color[2];
	color[2] = glm::u8vec4(res, rgbc[3]);
}

void GTE::avsz3()
{
	if (Debugging::gte)printf("GTE: AVSZ3\n");

	mac[0] = (screen[0].z + screen[1].z + screen[2].z) * avgZ3;
	otz = mac[0] >> 12;
}

u32 GTE::getData(u32 reg)
{
	switch (reg & 0x1F) {
	case 0:
		return (u32)(u16)vector[0].x | (((u32)(u16)vector[0].y) << 16);
	case 1:
		return (u32)vector[0].z;
	case 2:
		return (u32)(u16)vector[1].x | (((u32)(u16)vector[1].y) << 16);
	case 3:
		return (u32)vector[1].z;
	case 4:
		return (u32)(u16)vector[2].x | (((u32)(u16)vector[2].y) << 16);
	case 5:
		return (u32)vector[2].z;
	case 6:
		return *reinterpret_cast<u32*>(&rgbc);
	case 7:
		return (u32)otz;
	case 8:
		return (u32)ir[0];
	case 9:
		return (u32)ir[1];
	case 10:
		return (u32)ir[2];
	case 11: 
		return (u32)ir[3];
	case 12:
		return (u32)(u16)screen[0].x | ((u32)(u16)screen[0].y << 16);
	case 13:
		return (u32)(u16)screen[1].x | ((u32)(u16)screen[1].y << 16);
	case 14:
	case 15:
		return (u32)(u16)screen[2].x | ((u32)(u16)screen[2].y << 16);
	case 16:
		return (u32)(u16)screen[0].z;
	case 17:
		return (u32)(u16)screen[1].z;
	case 18:
		return (u32)(u16)screen[2].z;
	case 19:
		return (u32)(u16)screen[3].z;
	case 20:
		return *reinterpret_cast<u32*>(&color[0]);
	case 21:
		return *reinterpret_cast<u32*>(&color[1]);
	case 22:
		return *reinterpret_cast<u32*>(&color[2]);
	case 23:
		return (u32)res1;
	case 24:
		return (u32)mac[0];
	case 25:
		return (u32)mac[1];
	case 26:
		return (u32)mac[2];
	case 27:
		return (u32)mac[3];
	case 28:
	case 29:
		// converts color to 15bit
		return (u32)std::clamp<i16>((i16)ir[1] >> 7, 0, 0x1F)
			| (u32)std::clamp<i16>((i16)ir[2] >> 7, 0, 0x1F) << 5
			| (u32)std::clamp<i16>((i16)ir[3] >> 7, 0, 0x1F) << 10;
	case 30:
		return (u32)lzcs;
	case 31:
		return lzcr;
	}
}

void GTE::setData(u32 reg, u32 val)
{
	switch (reg & 0x1F) {
	case 0:
		vector[0].x = (i16)val;
		vector[0].y = (i16)(val >> 16);
		break;
	case 1:
		vector[0].z = (i16)val;
		break;
	case 2:
		vector[1].x = (i16)val;
		vector[1].y = (i16)(val >> 16);
		break;
	case 3:
		vector[1].z = (i16)val;
		break;
	case 4:
		vector[2].x = (i16)val;
		vector[2].y = (i16)(val >> 16);
		break;
	case 5:
		vector[2].z = (i16)val;
		break;
	case 6:
		*reinterpret_cast<u32*>(&rgbc) = val;
		break;
	case 7:
		otz = (u16)val;
		break;
	case 8:
		ir[0] = (i16)val;
		break;
	case 9:
		ir[1] = (i16)val;
		break;
	case 10:
		ir[2] = (i16)val;
		break;
	case 11:
		ir[3] = (i16)val;
		break;
	case 12:
		screen[0].x = (i16)val;
		screen[0].y = (i16)(val >> 16);
		break;
	case 13:
		screen[1].x = (i16)val;
		screen[1].y = (i16)(val >> 16);
		break;
	case 14:
		screen[2].x = (i16)val;
		screen[2].y = (i16)(val >> 16);
		break;
	case 15:
		for (int i = 0; i < 2; i++) {
			screen[i].x = screen[i + 1].x;
			screen[i].y = screen[i + 1].y;
		}

		screen[2].x = (i16)val;
		screen[2].y = (i16)(val >> 16);
		break;
	case 16:
		screen[0].z = (i16)val;
		break;
	case 17:
		screen[1].z = (i16)val;
		break;
	case 18:
		screen[2].z = (i16)val;
		break;
	case 19:
		screen[3].z = (i16)val;
		break;
	case 20:
		*reinterpret_cast<u32*>(&color[0]) = val;
		break;
	case 21:
		*reinterpret_cast<u32*>(&color[1]) = val;
		break;
	case 22:
		*reinterpret_cast<u32*>(&color[2]) = val;
		break;
	case 23:
		res1 = val;
		break;
	case 24:
		mac[0] = (i32)val;
		break;
	case 25:
		mac[1] = (i32)val;
		break;
	case 26:
		mac[2] = (i32)val;
		break;
	case 27:
		mac[3] = (i32)val;
		break;
	case 28:
		ir[1] = (i32)((val & 0x1F) << 7);
		ir[2] = (i32)(((val >> 5) & 0x1F) << 7);
		ir[3] = (i32)(((val >> 10) & 0x1F) << 7);
		break;
	case 29:
		break;
	case 30:
		lzcs = (i32)val;
		lzcr = countLeadingBits((i32)val);
		break;
	case 31:
		break;
	}
}

u32 GTE::getControl(u32 reg)
{
	switch (reg & 0x1F) {
	case 0:
		return (u32)(u16)rotation[0][0] | ((u32)(u16)rotation[1][0] << 16);
	case 1:
		return (u32)(u16)rotation[2][0] | ((u32)(u16)rotation[0][1] << 16);
	case 2:
		return (u32)(u16)rotation[1][1] | ((u32)(u16)rotation[2][1] << 16);
	case 3:
		return (u32)(u16)rotation[0][2] | ((u32)(u16)rotation[1][2] << 16);
	case 4:
		return (u32)(i32)rotation[2][2];
	case 5:
		return (u32)translation.x;
	case 6:
		return (u32)translation.y;
	case 7:
		return (u32)translation.z;
	case 8:
		return (u32)(u16)light[0][0] | ((u32)(u16)light[1][0] << 16);
	case 9:
		return (u32)(u16)light[2][0] | ((u32)(u16)light[0][1] << 16);
	case 10:
		return (u32)(u16)light[1][1] | ((u32)(u16)light[2][1] << 16);
	case 11:
		return (u32)(u16)light[0][2] | ((u32)(u16)light[1][2] << 16);
	case 12:
		return (u32)(i32)light[2][2];
	case 13:
		return (u32)backgroundColor.r;
	case 14:
		return (u32)backgroundColor.g;
	case 15:
		return (u32)backgroundColor.b;
	case 16:
		return (u32)(u16)lightColor[0][0] | ((u32)(u16)lightColor[1][0] << 16);
	case 17:
		return (u32)(u16)lightColor[2][0] | ((u32)(u16)lightColor[0][1] << 16);
	case 18:
		return (u32)(u16)lightColor[1][1] | ((u32)(u16)lightColor[2][1] << 16);
	case 19:
		return (u32)(u16)lightColor[0][2] | ((u32)(u16)lightColor[1][2] << 16);
	case 20:
		return (u32)(i32)lightColor[2][2];
	case 21:
		return (u32)farColor.r;
	case 22:
		return (u32)farColor.g;
	case 23:
		return (u32)farColor.b;
	case 24:
		return (u32)screenOffset.x;
	case 25:
		return (u32)screenOffset.y;
	case 26:
		// BUG : even though it is unsigned, H is signed-expended to 32 bits
		return (u32)(i32)(i16)planeDistance;
	case 27:
		return (u32)(i32)depthQueingCoeff;
	case 28:
		return (u32)depthQueingOffset;
	case 29:
		return (u32)(i32)avgZ3;
	case 30:
		return (u32)(i32)avgZ4;
	case 31:
		u32 bit31 = (u32)((flag & 0x7F87E000) != 0);
		return flag | (bit31 << 31);
	}
}

void GTE::setControl(u32 reg, u32 val)
{
	switch (reg & 0x1F) {
	case 0:
		rotation[0][0] = (i16)val;
		rotation[1][0] = (i16)(val >> 16);
		break;
	case 1:
		rotation[2][0] = (i16)val;
		rotation[0][1] = (i16)(val >> 16);
		break;
	case 2:
		rotation[1][1] = (i16)val;
		rotation[2][1] = (i16)(val >> 16);
		break;
	case 3:
		rotation[0][2] = (i16)val;
		rotation[1][2] = (i16)(val >> 16);
		break;
	case 4:
		rotation[2][2] = (i16)val;
		break;
	case 5:
		translation.x = (i32)val;
		break;
	case 6:
		translation.y = (i32)val;
		break;
	case 7:
		translation.z = (i32)val;
		break;
	case 8:
		light[0][0] = (i16)val;
		light[1][0] = (i16)(val >> 16);
		break;
	case 9:
		light[2][0] = (i16)val;
		light[0][1] = (i16)(val >> 16);
		break;
	case 10:
		light[1][1] = (i16)val;
		light[2][1] = (i16)(val >> 16);
		break;
	case 11:
		light[0][2] = (i16)val;
		light[1][2] = (i16)(val >> 16);
		break;
	case 12:
		light[2][2] = (i16)val;
		break;
	case 13:
		backgroundColor.x = (i32)val;
		break;
	case 14:
		backgroundColor.y = (i32)val;
		break;
	case 15:
		backgroundColor.z = (i32)val;
		break;
	case 16:
		lightColor[0][0] = (i16)val;
		lightColor[1][0] = (i16)(val >> 16);
		break;
	case 17:
		lightColor[2][0] = (i16)val;
		lightColor[0][1] = (i16)(val >> 16);
		break;
	case 18:
		lightColor[1][1] = (i16)val;
		lightColor[2][1] = (i16)(val >> 16);
		break;
	case 19:
		lightColor[0][2] = (i16)val;
		lightColor[1][2] = (i16)(val >> 16);
		break;
	case 20:
		lightColor[2][2] = (i16)val;
		break;
	case 21:
		farColor.x = (i32)val;
		break;
	case 22:
		farColor.y = (i32)val;
		break;
	case 23:
		farColor.z = (i32)val;
		break;
	case 24:
		screenOffset.x = (i32)val;
		break;
	case 25:
		screenOffset.y = (i32)val;
		break;
	case 26:
		planeDistance = (u16)val;
		break;
	case 27:
		depthQueingCoeff = (i16)val;
		break;
	case 28:
		depthQueingOffset = (i32)val;
		break;
	case 29:
		avgZ3 = (i16)val;
		break;
	case 30:
		avgZ4 = (i16)val;
		break;
	case 31:
		flag = val & 0x7FFFF000;
		break;
	}
}

void GTE::sendCmd(u32 cmd)
{
	// keep the interesting part of the cmd
	//cmd &= (1 << 25) - 1;
	tempCmd.val = cmd;
	switch (tempCmd.content.cmd) {
	case 0x06:
		nclip();
		break;
	case 0x13:
		ncds(0);
		break;
	case 0x2D:
		avsz3();
		break;
	case 0x30:
		rtpt();
		break;
	default:
		printf("COP2 cmd not recongnized : 0x%02x\n", cmd & 0x3F);
		throw_error("Unknown cop2 cmd!");
	}
}
