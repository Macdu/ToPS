#pragma once

#include "../definitions.h"

// all the GPU properties, some can be accessed through 0x1F801814 (GPUSTAT)
struct GPUProperties {
	// bit 0-3, multiplied by 64
	u32 texturePageXBase;
	// bit 4, multiplied by 256
	u32 texturePageYBase;

	// bit 5-6 (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)
	u32 semiTransparency;

	// bit 7-8 (0=4bit, 1=8bit, 2=15bit, 3=Reserved)
	u32 texturePageColors;

	// bit 9 Dither 24bit to 15bit (0=Off/strip LSBs, 1=Dither Enabled)
	bool dither24to15;

	// bit 10 Drawing to display area (0=Prohibited, 1=Allowed) 
	bool canDrawToDisplayArea;

	// bit 11 Set Mask-bit when drawing pixels (0=No, 1=Yes/Mask)
	bool doMaskWhenDrawing;

	// bit 12 Draw Pixels (0=Always, 1=Not to Masked areas)
	bool cannotDrawToMasked;

	// bit 14 "Reverseflag" (0=Normal, 1=Distorted)

	// bit 15 Texture Disable (0=Normal, 1=Disable Textures)
	bool disableTexture;

	// bit 16 Horizontal Resolution 2 (0=256/320/512/640, 1=368)
	bool horizontalRes2;

	// bit 17-18 Horizontal Resolution 1 (0=256, 1=320, 2=512, 3=640)
	bool horizontalRes1;

	// bit 19 Vertical Resolution         (0=240, 1=480, when Bit22=1)
	bool verticalRes;

	// bit 20 Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)
	bool isPAL;

	// bit 21 Display Area Color Depth    (0=15bit, 1=24bit)
	bool isDepthDisplay24;

	// bit 22 Vertical Interlace          (0=Off, 1=On)
	bool hasVerticalInterlace;

	// bit 23 Display Enable              (0=Enabled, 1=Disabled)
	bool isDisplayDisabled;

	// bit 24 Interrupt Request (IRQ1)    (0=Off, 1=IRQ)
	bool interrupRequest;

	// bit 25 DMA / Data Request, meaning depends on GP1(04h) DMA Direction:
	// for now set to 1

	// bit 26 Ready to receive Cmd Word   (0=No, 1=Ready)
	// for now set to 1

	// bit 27 Ready to send VRAM to CPU   (0=No, 1=Ready)
	// for now set to 1

	// bit 28 Ready to receive DMA Block  (0=No, 1=Ready)
	// for now set to 1

	// bit 29-30 DMA Direction (0=Off, 1=?, 2=CPUtoGP0, 3=GPUREADtoCPU)
	u32 dmaDirection;

	// bit 31 Drawing even/odd lines in interlace mode (0=Even or Vblank, 1=Odd)
	// for now set to 0

	// Textured Rectangle X-Flip
	bool textureRectangleFlipX;

	// Textured Rectangle Y-Flip
	bool textureRectangleFlipY;

	// halfword address in VRAM (0-1023)
	u32 displayAreaTopLeftX;
	// scanline number in VRAM (0-511)
	u32 displayAreaTopLeftY;

	// 12-bit value
	u32 horizontalDisplayStart;
	// 12-bit value
	u32 horizontalDisplayEnd;

	// 10-bit value
	u32 verticalDisplayStart;
	// 10-bit value
	u32 verticalDisplayEnd;

	// 10-bit values
	u32 drawingAreaTop;
	u32 drawingAreaLeft;
	u32 drawingAreaBottom;
	u32 drawingAreaRight;

	// 11-bit signed offsets (x and y)
	glm::ivec2 drawingOffset;

	// 5-bit values
	// Texcoord = (Texcoord AND (NOT (Mask*8))) OR ((Offset AND Mask)*8)
	u32 textureMaskX;
	u32 textureMaskY;
	u32 textureOffsetX;
	u32 textureOffsetY;

	void reset() {
		texturePageXBase = 0;
		texturePageYBase = 0;
		semiTransparency = 0;
		texturePageColors = 0;
		dither24to15 = false;
		canDrawToDisplayArea = false;
		doMaskWhenDrawing = false;
		cannotDrawToMasked = false;
		disableTexture = false;
		horizontalRes2 = 0;
		horizontalRes1 = 0;
		verticalRes = 0;
		isPAL = false;
		isDepthDisplay24 = false;
		hasVerticalInterlace = false;
		isDisplayDisabled = false;
		interrupRequest = false;
		dmaDirection = 0;
		textureRectangleFlipX = false;
		textureRectangleFlipY = false;
		displayAreaTopLeftX = 0;
		displayAreaTopLeftY = 0;
		horizontalDisplayStart = 0x200;
		horizontalDisplayEnd = 0xc00;
		verticalDisplayStart = 0x10;
		verticalDisplayEnd = 0x100;
		drawingAreaBottom = 0;
		drawingAreaLeft = 0;
		drawingAreaRight = 0;
		drawingAreaTop = 0;
		drawingOffset.x = 0;
		drawingOffset.y = 0;
		textureMaskX = 0;
		textureMaskY = 0;
		textureOffsetX = 0;
		textureOffsetY = 0;
	}

	// called after GP0 0xE1 cmd
	void setDrawModeSetting(u32 cmd) {
		texturePageXBase = (cmd & 0xF) * 64;
		texturePageYBase = (cmd & 0x10) << 4;
		semiTransparency = (cmd >> 5) & 3;
		texturePageColors = (cmd >> 7) & 3;
		dither24to15 = (cmd & (1 << 9)) != 0;
		canDrawToDisplayArea = (cmd & (1 << 10)) != 0;
		disableTexture = (cmd & (1 << 11)) != 0;
		textureRectangleFlipX = (cmd & (1 << 12)) != 0;
		textureRectangleFlipY = (cmd & (1 << 13)) != 0;
	}

	// called after GP1 0x08 cmd
	void setDisplayMode(u32 cmd) {
		horizontalRes1 = cmd & 3;
		verticalRes = (cmd & (1 << 2)) != 0;
		isPAL = (cmd & (1 << 3)) != 0;
		isDepthDisplay24 = (cmd & (1 << 4)) != 0;
		hasVerticalInterlace = (cmd & (1 << 5)) != 0;
		horizontalRes2 = (cmd & (1 << 6)) != 0;
		// reverseFlag not used
		assert((cmd & (1 << 7)) == 0);
	}

	u32 getGPUStat() {
		u32 res = 0;
		res |= texturePageXBase >> 6;
		res |= texturePageYBase >> 4;
		res |= semiTransparency << 5;
		res |= texturePageColors << 7;
		res |= dither24to15 << 9;
		res |= canDrawToDisplayArea << 10;
		res |= doMaskWhenDrawing << 11;
		res |= cannotDrawToMasked << 12;
		res |= disableTexture << 15;
		res |= horizontalRes2 << 16;
		res |= horizontalRes1 << 17;
		// workaround right now
		//res |= verticalRes << 19;
		res |= isPAL << 20;
		res |= isDepthDisplay24 << 21;
		res |= hasVerticalInterlace << 22;
		res |= isDisplayDisabled << 23;
		res |= interrupRequest << 24;
		// everything is ready;
		res |= 0b1111 << 25;
		res |= dmaDirection << 29;
		return res;
	}
};