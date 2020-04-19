#include "GTE.h"

#include <intrin.h>
#include <algorithm>

#pragma intrinsic(__lzcnt)
#pragma intrinsic(__lzcnt16)

u32 GTE::countLeadingBits(i32 val)
{
	if (val < 0)val = ~val;
	/*
	unsigned long res;
	// we use some c++ builtin functions on MSVC
	if (_BitScanReverse(&res, (u32)val)) {
		return 31 - res;
	}
	else {
		// val is 0
		return 32;
	}
	*/
	return __lzcnt(val);
}

// I tried to constexpr it but the resulting assembly is kind of weird
const u8 tableUNR[] = { 
	0xff, 0xfd, 0xfb, 0xf9, 0xf7, 0xf5, 0xf3, 0xf1, 0xef, 0xee, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe3,
	0xe1, 0xdf, 0xdd, 0xdc, 0xda, 0xd8, 0xd6, 0xd5, 0xd3, 0xd1, 0xd0, 0xce, 0xcd, 0xcb, 0xc9, 0xc8,
	0xc6, 0xc5, 0xc3, 0xc1, 0xc0, 0xbe, 0xbd, 0xbb, 0xba, 0xb8, 0xb7, 0xb5, 0xb4, 0xb2, 0xb1, 0xb0,
	0xae, 0xad, 0xab, 0xaa, 0xa9, 0xa7, 0xa6, 0xa4, 0xa3, 0xa2, 0xa0, 0x9f, 0x9e, 0x9c, 0x9b, 0x9a,
	0x99, 0x97, 0x96, 0x95, 0x94, 0x92, 0x91, 0x90, 0x8f, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x87, 0x86, 
	0x85, 0x84, 0x83, 0x82, 0x81, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x75, 0x74, 
	0x73, 0x72, 0x71, 0x70, 0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 
	0x63, 0x62, 0x61, 0x60, 0x5f, 0x5e, 0x5d, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 
	0x54, 0x53, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x48, 
	0x47, 0x46, 0x45, 0x44, 0x43, 0x43, 0x42, 0x41, 0x40, 0x3f, 0x3f, 0x3e, 0x3d, 0x3c, 0x3c, 0x3b, 
	0x3a, 0x39, 0x39, 0x38, 0x37, 0x36, 0x36, 0x35, 0x34, 0x33, 0x33, 0x32, 0x31, 0x31, 0x30, 0x2f,
	0x2e, 0x2e, 0x2d, 0x2c, 0x2c, 0x2b, 0x2a, 0x2a, 0x29, 0x28, 0x28, 0x27, 0x26, 0x26, 0x25, 0x24, 
	0x24, 0x23, 0x22, 0x22, 0x21, 0x20, 0x20, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d, 0x1c, 0x1b, 0x1b, 0x1a, 
	0x19, 0x19, 0x18, 0x18, 0x17, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14, 0x13, 0x12, 0x12, 0x11, 0x11, 
	0x10, 0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0b, 0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08, 
	0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 
	0x00 };

u32 GTE::unrDivision()
{
	u32 h = planeDistance;
	u32 sz3 = (u16)screen[3].z;
	if (h >= sz3 * 2) {
		flag |= (u32)GTEFlags::DivideOverflow;
		return 0x1FFFF;
	}

	u8 z = __lzcnt16(sz3);
	u64 res = h << z;
	sz3 <<= z;
	u32 val = tableUNR[(sz3 - 0x7FC0) >> 7] + 0x101;;
	u32 tmp = (0x2000080 - (sz3 * val)) >> 8;
	tmp = (0x80 + (tmp * val)) >> 8;
	res = std::min<u64>(0x1FFFF, ((res * tmp) + 0x8000) >> 16);
	return res;

}

void GTE::rtps(int i)
{
	if (Debugging::gte && tempCmd.content.cmd == 0x01)
		printf("GTE: RTPS\n");

	glm::i64vec3 res = rotation * vector[i] + (translation << glm::i64vec3(12,12,12));
	checkVector(res);

	// Note: The command does saturate IR1,IR2,IR3 to -8000h..+7FFFh (regardless of lm bit).
	// When using RTP with sf=0, then the IR3 saturation flag (FLAG.22) 
	// gets set <only> if "MAC3 SAR 12" exceeds -8000h..+7FFFh 
	// (although IR3 is saturated when "MAC3" exceeds -8000h..+7FFFh).
	if (!tempCmd.content.shiftIR) {
		flag &= ~(u32)GTEFlags::IR3Overflow;
		saturate(res.z >> 12, 0x7FFF, GTEFlags::IR3Overflow, -0x8000, GTEFlags::IR3Overflow);
	}

	screen[0] = screen[1];
	screen[1] = screen[2];

	screen[2].z = screen[3].z;
	screen[3].z = saturate(res.z >> 12, 0xFFFF, GTEFlags::SZ3Overflow, 0, GTEFlags::SZ3Overflow);

	i64 planeComp = unrDivision();
	screen[2].x = saturate((planeComp * ir[1] + screenOffset.x) >> 16, 0x3FF, GTEFlags::SX2Overflow,
		-0x400, GTEFlags::SX2Overflow);
	screen[2].y = saturate((planeComp * ir[2] + screenOffset.y) >> 16, 0x3FF, GTEFlags::SY2Overflow,
		-0x400, GTEFlags::SY2Overflow);

	setMacIr(0, planeComp * depthQueingCoeff + depthQueingOffset);
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
	saturate(mac[0], (1LL << 31) - 1, GTEFlags::MAC0NegativeOverflow,
		-(1LL << 31), GTEFlags::MAC0NegativeOverflow);
	mac[0] = (i32)mac[0];
	
}

void GTE::ncs(int i)
{
	if (Debugging::gte && tempCmd.content.cmd == 0x1E)
		printf("GTE: NCDS\n");

	checkVector(light * vector[i]);
	checkVector(lightColor * ir.yzw + (backgroundColor << glm::i64vec3(12, 12, 12)));

	glm::i64vec3 res = ir.yzw;
	res >>= glm::i64vec3(4, 4, 4);

	res.r = saturate(res.r, 0xFF, GTEFlags::RedOverflow, 0x00, GTEFlags::RedOverflow);
	res.g = saturate(res.g, 0xFF, GTEFlags::GreenOverflow, 0x00, GTEFlags::GreenOverflow);
	res.b = saturate(res.b, 0xFF, GTEFlags::BlueOverflow, 0x00, GTEFlags::BlueOverflow);

	color[0] = color[1];
	color[1] = color[2];
	color[2] = glm::u8vec4(res, rgbc[3]);
}

void GTE::ncds(int i)
{
	if (Debugging::gte && tempCmd.content.cmd == 0x13)
		printf("GTE: NCDS\n");


	checkVector(light * vector[i]);
	checkVector(lightColor * ir.yzw + (backgroundColor << glm::i64vec3(12, 12, 12)));

	glm::i64vec3 res = ir.yzw;

	glm::i64vec3 col = glm::i64vec3((i64)rgbc.r << 4, (i64)rgbc.g << 4, (i64)rgbc.b << 4);
	glm::i64vec3 fcShifted = farColor << glm::i64vec3(12, 12, 12);
	glm::i64vec3 ir0 = glm::i64vec3(ir[0], ir[0], ir[0]);

	checkVector(fcShifted - col * res);
	checkVector(col * res + ir0 * ir.yzw);

	res = ir.yzw;
	res >>= glm::i64vec3(4, 4, 4);

	res.r = saturate(res.r, 0xFF, GTEFlags::RedOverflow, 0x00, GTEFlags::RedOverflow);
	res.g = saturate(res.g, 0xFF, GTEFlags::GreenOverflow, 0x00, GTEFlags::GreenOverflow);
	res.b = saturate(res.b, 0xFF, GTEFlags::BlueOverflow, 0x00, GTEFlags::BlueOverflow);

	color[0] = color[1];
	color[1] = color[2];
	color[2] = glm::u8vec4(res, rgbc[3]);
}

void GTE::avsz3()
{
	if (Debugging::gte)printf("GTE: AVSZ3\n");

	mac[0] = ((u16)screen[1].z + (u16)screen[2].z + (u16)screen[3].z) * avgZ3;
	saturate(mac[0], (1LL << 31) - 1, GTEFlags::MAC0NegativeOverflow,
		-(1LL << 31), GTEFlags::MAC0NegativeOverflow);
	otz = saturate(mac[0] >> 12, 0xFFFF, GTEFlags::SZ3Overflow, 0x0000, GTEFlags::SZ3Overflow);
	mac[0] = (i32)mac[0];
}

void GTE::checkVector(glm::i64vec3 vec)
{
	for (int i = 1; i <= 3; i++) {
		setMacIr(i, vec[i - 1]);
	}
}

inline void GTE::setMacIr(int i, i64 val)
{
	assert(i >= 0 && i <= 3);

	GTEFlags macPos, macNeg, irFlag;
	switch (i)
	{
	case 0:
		macPos = GTEFlags::MAC0PositiveOverflow;
		macNeg = GTEFlags::MAC0NegativeOverflow;
		irFlag = GTEFlags::IR0Overflow;
		break;
	case 1:
		macPos = GTEFlags::MAC1PositiveOverflow;
		macNeg = GTEFlags::MAC1NegativeOverflow;
		irFlag = GTEFlags::IR1Overflow;
		break;
	case 2:
		macPos = GTEFlags::MAC2PositiveOverflow;
		macNeg = GTEFlags::MAC2NegativeOverflow;
		irFlag = GTEFlags::IR2Overflow;
		break;
	case 3:
		macPos = GTEFlags::MAC3PositiveOverflow;
		macNeg = GTEFlags::MAC3NegativeOverflow;
		irFlag = GTEFlags::IR3Overflow;
		break;
	}

	if (i == 0) {
		saturate(val, (1LL << 31) - 1, macPos, -(1LL << 31), macNeg);
		mac[0] = (i32)val;
		ir[0] = saturate(val >> 12, 0x1000, irFlag, 0, irFlag);
	}
	else {
		saturate(val, (1LL << 43) - 1, macPos, -(1LL << 43), macNeg);
		if (tempCmd.content.shiftIR) {
			val >>= 12;
		}
		mac[i] = (i32)val;

		ir[i] = saturate(mac[i], 0x7FFF, irFlag, tempCmd.content.saturateIR ? 0 : -0x8000, irFlag);
	}
}

inline i64 GTE::saturate(i64 val, i64 max, GTEFlags overflow, i64 min, GTEFlags underflow)
{
	if (val > max) {
		val = max;
		flag |= (u32)overflow;
	}
	else if (val < min){
		val = min;
		flag |= (u32)underflow;
	}
	return val;
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
	flag = 0;
	switch (tempCmd.content.cmd) {
	case 0x01:
		rtps(0);
		break;
	case 0x06:
		nclip();
		break;
	case 0x13:
		ncds(0);
		break;
	case 0x1E:
		//ncs(0);
		break;
	case 0x2D:
		avsz3();
		break;
	case 0x30:
		rtpt();
		break;
	default:
		break;
		printf("COP2 cmd not recongnized : 0x%02x\n", cmd & 0x3F);
		throw_error("Unknown cop2 cmd!");
	}
}
