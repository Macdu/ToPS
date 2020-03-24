#pragma once

#include <queue>

#include "GPUProperties.h"
#include "renderer/VulkanRenderer.h"

class Emulator;

class GPU {
public:
	// values taken from Avocado emulator
	static constexpr int cyclesPerScanline = 3413;
	static constexpr int totalNTSCScanlines = 263;
	static constexpr int scanlineVBlankStart = 243;

	GPUProperties gpuProps;
	VulkanRenderer renderer;
	Emulator* emu;

	// GP0 command FIFO
	std::queue<u32> gp0Queue;

	// needs to be changed to be done during compile time
	void initGP0Opcodes();
	void reset();
	void init(vk::Instance instance, vk::SurfaceKHR surface);
	u32 getGPUStat();
	// return content of GPUREAD (0x1F801810) port
	u32 getGPURead();
	// set the current scanline being 
	void setScanline(int scanline);

	inline void pushVertexColor(const Point<i16>& point,const Color& color);
	inline void pushVertexTexture(const Point<i16>& point, const Point<u8>& textLoc,
		const u16& clutId, const u16& textPage);
	void drawFrame();

	void pushCmdGP0(u32 val);

	// execute a gp0 cmd, assumes the whole command has already been sent
	void gp0(u32 cmd, u32 opcode);
	// execute a gp1 cmd
	void gp1(u32 cmd);

	// Draws a monochrome rectangle directly to the framebuffer
	// Ignores all environment settings
	void rectangleDraw();

	void textured4Points();
	void monochrome4Points();
	void shaded4points();
	void shadedTriangle();
	void rectangle();
	void dot();
	void sprite();

	void sendRectToFrameBuffer();
	void sendFrameBufferToCPU();

private:
	// the total number of frames that have been rendered
	u64 totalFrames = 0;

	int currScanline = 0;

	const Color defaultTextureColor = { 240,0,60 };
	bool isSendingImage;
	u32 currentImageSize;
	u32 totalImageSize;
	u16 imageTransfer[512 * 1024];
	Point<i16> imageTopLeft;
	Point<i16> imageExtent;

	u32 gpuReadData[512 * 1024 / 2];
	u32 gpuReadDataSize = 0;
	u32 gpuReadDataCurr = 0;

	inline Point<i16> readPoint() {
		u32 word = gp0Queue.front();
		gp0Queue.pop();
		return { static_cast<i16>(word), static_cast<i16>(word >> 16) };
	}
	
	inline Color readColor() {
		u32 word = gp0Queue.front();
		gp0Queue.pop();
		return { (u8)word, (u8)(word >> 8), (u8)(word >> 16) };
	}

	inline Point<u8> extractTextLoc(const u32& word) {
		return { (u8)word, (u8)(word >> 8) };
	}

	// get the 4 points of a rectangle from the topLeft point, the width and the height
	template<typename T> inline std::array<Point<T>,4> getRectangle(Point<T> topLeft, Point<T> size) {
		auto add = std::plus<T>();
		return {
			topLeft,
			{add(topLeft.x,size.x), topLeft.y},
			{add(topLeft.x, size.x), add(topLeft.y, size.y)},
			{topLeft.x, add(topLeft.y, size.y)}
		};
	}
};