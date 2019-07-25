#pragma once

#include <queue>

#include "GPUProperties.h"
#include "renderer/VulkanRenderer.h"

class Emulator;

class GPU {
public:
	GPUProperties gpuProp;
	VulkanRenderer renderer;
	Emulator* emu;

	// tells if a frame is ready to be rendered
	bool isFrameReady = false;

	// GP0 command FIFO
	std::queue<u32> gp0Queue;

	// needs to be changed to be done during compile time
	void initGP0Opcodes();
	void reset();
	void init(vk::Instance instance, vk::SurfaceKHR surface);
	u32 getGPUStat();

	inline void pushVertex(const Point<i16>& point,const Color& color);
	void drawFrame();

	void pushCmdGP0(u32 val);

	// execute a gp0 cmd, assumes the whole command has already been sent
	void gp0(u32 cmd, u32 opcode);
	// execute a gp1 cmd
	void gp1(u32 cmd);

	void textured4Points();
	void monochrome4Points();
	void shaded4points();
	void shadedTriangle();

	void sendRectToFrameBuffer();
	void sendFrameBufferToCPU();

private:
	const Color defaultTextureColor = { 240,0,60 };
	bool isSendingImage;
	u32 currentImageSize;
	u32 totalImageSize;
	u16 imageTransfer[512 * 1024];
	Point<i16> imageTopLeft;
	Point<i16> imageExtent;

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
};