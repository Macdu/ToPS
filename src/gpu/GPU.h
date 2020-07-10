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
	// return the horizontal res (used by timers)
	u32 getHorizontalRes();

	inline void pushVertexColor(const Point<i16>& point,const Color& color);
	inline void pushVertexTexture(const Point<i16>& point, const Point<i16>& textLoc,
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
	void monochromeTriangle();
	void shaded4points();
	void shadedTriangle();
	void rectangle();
	void line();
	void dot();
	// calling it with side=0 makes this function reads the width and height from gp0 parameters
	void sprite(i16 side);
	void rectangleVRAMCopy();

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
		// signed 11bit to 16bit expansion
		i16 x = word & 0x7FF;
		x <<= 5;
		x >>= 5;
		i16 y = (word >> 16) & 0xFF;
		y <<= 5;
		y >>= 5;
		return { x, y };
	}
	
	inline Color readColor() {
		u32 word = gp0Queue.front();
		gp0Queue.pop();
		return { (u8)word, (u8)(word >> 8), (u8)(word >> 16) };
	}

	inline Point<i16> extractTextLoc(const u32& word) {
		return { (i16)(u8)word, (i16)(u8)(word >> 8) };
	}

	// get the 4 points of a rectangle from the topLeft point, the width and the height
	template<typename T> std::array<Point<T>,4> getRectangle(Point<T> topLeft, Point<T> size) {
		auto add = std::plus<T>();
		return {
			topLeft,
			{add(topLeft.x,size.x), topLeft.y},
			{topLeft.x, add(topLeft.y, size.y)},
			{add(topLeft.x, size.x), add(topLeft.y, size.y)}
		};
	}

	vk::Rect2D currDrawingScissor;
	glm::ivec2 currDrawingOffset;
	// used to remove the drawing offset and scissors that are set for a specific out of border draw
	inline void setGlobalDrawingContext() {
		// I just "disable" the current scissor and then draw the rectangle
		currDrawingScissor = renderer.sceneRendering.currentScissor;
		// And the current drawing offset too
		currDrawingOffset = gpuProps.drawingOffset;
		gpuProps.drawingOffset = { 0,0 };
		renderer.sceneRendering.setScissor(renderer.sceneRendering.frameScissor);
	};

	inline void endGlobalDrawingContext() {
		gpuProps.drawingOffset = currDrawingOffset;
		renderer.sceneRendering.setScissor(currDrawingScissor);
	}

};