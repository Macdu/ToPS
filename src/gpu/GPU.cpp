#include "GPU.h"

#include <algorithm>

u16 gp0_opcodes_length[256];

void GPU::initGP0Opcodes()
{
	std::fill_n(gp0_opcodes_length, 256, -1);
	gp0_opcodes_length[0x00] = 1;
	gp0_opcodes_length[0x01] = 1;

	gp0_opcodes_length[0x2C] = 9;
	gp0_opcodes_length[0x28] = 5;
	gp0_opcodes_length[0x30] = 6;
	gp0_opcodes_length[0x38] = 8;

	gp0_opcodes_length[0xA0] = 3;
	gp0_opcodes_length[0xC0] = 3;

	gp0_opcodes_length[0xE1] = 1;
	gp0_opcodes_length[0xE2] = 1;
	gp0_opcodes_length[0xE3] = 1;
	gp0_opcodes_length[0xE4] = 1;
	gp0_opcodes_length[0xE5] = 1;
	gp0_opcodes_length[0xE6] = 1;

}

void GPU::reset() {
	initGP0Opcodes();
	gpuProp.reset();
	isSendingImage = false;
	imageSizeLeft = 0;
}

void GPU::init(vk::Instance instance, vk::SurfaceKHR surface)
{
	renderer.setInstance(instance);
	renderer.setSurface(surface);
	renderer.setWidth(1024);
	renderer.setHeight(512);
	renderer.initVulkan();
	reset();
}

u32 GPU::getGPUStat() {
	return gpuProp.getGPUStat();
}

inline void GPU::pushVertex(const Point<i16>& point,const Color & color)
{
	renderer.sceneRendering.verticesToRender.push_back(
		{ point.toIVec2() + gpuProp.drawingOffset, color.toUVec3() });
}

void GPU::drawFrame() {
	printf("Drawing frame.\n");
	if (renderer.sceneRendering.verticesToRender.size() != 0) {
		renderer.drawFrame();
		renderer.sceneRendering.verticesToRender.clear();
	}
}

void GPU::pushCmdGP0(u32 val)
{
	if (isSendingImage) {
		// special case, ignore it for now
		imageSizeLeft--;
		if (imageSizeLeft == 0) {
			isSendingImage = false;
		}
		return;
	}

	gp0Queue.push(val);
	u32 cmd = gp0Queue.front();
	u32 opcode = cmd >> 24;

	if (gp0_opcodes_length[opcode] == (u16)(-1)) {
		printf("Opcode 0x%02x for cmd 0x%08x GP0 not valid\n", opcode, cmd);
		throw_error("Unrecognized opcode!");
	}

	assert(gp0_opcodes_length[opcode] >= gp0Queue.size());

	if (gp0_opcodes_length[opcode] == gp0Queue.size()) {
		// we have a full command, we can empty the buffer
		if(gp0_opcodes_length[opcode] == 1)gp0Queue.pop();
		gp0(cmd, opcode);

		// for now we can empty the buffer after
		//while (!gp0Queue.empty())
		//	gp0Queue.pop();
		assert(gp0Queue.size() == 0);
	}
}

void GPU::gp0(u32 cmd, u32 opcode)
{
	switch (opcode) {
	case 0x00:
		// NOP
		break;

	case 0x01:
		// Clear cache
		break;

	case 0x28:
		// monochrome 4 point polygon
		monochrome4Points();
		break;

	case 0x2C:
		// textured 4 points quad
		textured4Points();
		break;

	case 0x30:
		// shaded triangle
		shadedTriangle();
		break;

	case 0x38:
		// shaded 4 point polygon
		shaded4points();
		break;

	case 0xA0:
		// send image to frame buffer
		sendRectToFrameBuffer();
		break;

	case 0xC0:
		// send framebuffer content back to GPUREAD port
		sendFrameBufferToCPU();
		break;

	case 0xE1:
		// Draw Mode setting (aka "Texpage")
		gpuProp.setDrawModeSetting(cmd);
		break;

	case 0xE2:
		// Texture Window setting
		gpuProp.textureMaskX = cmd & ((1 << 5) - 1);
		gpuProp.textureMaskY = (cmd >> 5) & ((1 << 5) - 1);
		gpuProp.textureOffsetX = (cmd >> 10) & ((1 << 5) - 1);
		gpuProp.textureOffsetY = (cmd >> 15) & ((1 << 5) - 1);
		break;

	case 0xE3:
		// Set Drawing Area top left (X1,Y1)
		gpuProp.drawingAreaLeft = cmd & ((1 << 10) - 1);
		gpuProp.drawingAreaTop = (cmd >> 10) & ((1 << 10) - 1);
		break;

	case 0xE4:
		// Set Drawing Area bottom right (X2,Y2)
		gpuProp.drawingAreaRight = cmd & ((1 << 10) - 1);
		gpuProp.drawingAreaBottom = (cmd >> 10) & ((1 << 10) - 1);
		break;

	case 0xE5:
		// Set Drawing Offset (X,Y)
		gpuProp.drawingOffset.x = cmd & ((1 << 11) - 1);
		if (gpuProp.drawingOffset.x >= (1 << 10)) {
			// 11-bit sign extension
			gpuProp.drawingOffset.x = -(((1 << 11) - 1) - gpuProp.drawingOffset.x + 1);
		}
		gpuProp.drawingOffset.y = (cmd >> 11) & ((1 << 11) - 1);
		if (gpuProp.drawingOffset.y >= (1 << 10)) {
			// 11-bit sign extension
			gpuProp.drawingOffset.y = -(((1 << 11) - 1) - gpuProp.drawingOffset.y + 1);
		}

		// hack used right now to draw
		isFrameReady = true;
		break;

	case 0xE6:
		// Mask Bit Setting
		gpuProp.doMaskWhenDrawing = cmd & 1;
		gpuProp.cannotDrawToMasked = (cmd & 2) != 0;
		break;

	default:
		// this should never happen
		assert(false);
		break;
	}
}

void GPU::gp1(u32 cmd)
{
	u32 opcode = cmd >> 24;
	switch (opcode) {
	case 0x00:
		// Reset
		while (!gp0Queue.empty()) {
			gp0Queue.pop();
		}
		gpuProp.reset();
		break;

	case 0x01:
		// Reset command buffer
		while (!gp0Queue.empty()) {
			gp0Queue.pop();
		}
		break;

	case 0x02:
		// acknowledge GPU IRQ interrupt
		gpuProp.interrupRequest = false;
		break;

	case 0x03:
		// Display Enable
		gpuProp.isDisplayDisabled = cmd & 1;
		break;

	case 0x04:
		//  DMA Direction / Data Request
		gpuProp.dmaDirection = cmd & 3;
		break;

	case 0x05:
		// Start of Display area (in VRAM)
		gpuProp.displayAreaTopLeftX = cmd & ((1 << 10)-1);
		gpuProp.displayAreaTopLeftY = (cmd >> 10) & ((1 << 9) - 1);
		break;

	case 0x06:
		// Horizontal Display range (on Screen)
		gpuProp.horizontalDisplayStart = cmd & ((1 << 12) - 1);
		gpuProp.horizontalDisplayEnd = (cmd >> 12) & ((1 << 12) - 1);
		break;

	case 0x07:
		// Vertical Display range (on Screen)
		gpuProp.verticalDisplayStart = cmd & ((1 << 10) - 1);
		gpuProp.verticalDisplayEnd = (cmd >> 10) & ((1 << 10) - 1);
		break;

	case 0x08:
		// Display mode
		gpuProp.setDisplayMode(cmd);
		break;

	default:
		printf("Opcode 0x%02x for cmd 0x%08x GP1 not valid\n", opcode, cmd);
		throw_error("Unrecognized opcode!");
	}
}

void GPU::textured4Points()
{
	// textures are not supported right now
	Point<i16> vertices[4];
	// Command + color vertex 0
	gp0Queue.pop();
	vertices[0] = readPoint();
	// CULT ID + texture coordinates vertex 0
	gp0Queue.pop();
	vertices[1] = readPoint();
	// Texture page + texture coordinates vertex 1
	gp0Queue.pop();
	vertices[2] = readPoint();
	// Texture coordinates vertex 2
	gp0Queue.pop();
	vertices[3] = readPoint();
	// Texture coordinates vertex 4
	gp0Queue.pop();
	for (int i = 0; i < 3; i++) {
		pushVertex(vertices[i], defaultTextureColor);
	}
	for (int i = 1; i < 4; i++) {
		pushVertex(vertices[i], defaultTextureColor);
	}
	printf("Draw textured quad\n");
}

void GPU::monochrome4Points()
{
	Point<i16> vertices[4];
	Color color = readColor();

	vertices[0] = readPoint();
	vertices[1] = readPoint();
	vertices[2] = readPoint();
	vertices[3] = readPoint();
	for (int i = 0; i < 3; i++) {
		pushVertex(vertices[i], color);
	}
	for (int i = 1; i < 4; i++) {
		pushVertex(vertices[i], color);
	}
	printf("Draw monochrome 4-points\n");
}

void GPU::shaded4points()
{
	Point<i16> vertices[4];
	Color colors[4];

	colors[0] = readColor();
	vertices[0] = readPoint();
	colors[1] = readColor();
	vertices[1] = readPoint();
	colors[2] = readColor();
	vertices[2] = readPoint();
	colors[3] = readColor();
	vertices[3] = readPoint();
	
	for (int i = 0; i < 3; i++) {
		pushVertex(vertices[i], colors[i]);
	}
	for (int i = 1; i < 4; i++) {
		pushVertex(vertices[i], colors[i]);
	}
	printf("Draw shaded 4-points\n");
}

void GPU::shadedTriangle()
{
	Point<i16> vertices[3];
	Color colors[3];

	colors[0] = readColor();
	vertices[0] = readPoint();
	colors[1] = readColor();
	vertices[1] = readPoint();
	colors[2] = readColor();
	vertices[2] = readPoint();

	for (int i = 0; i < 3; i++) {
		pushVertex(vertices[i], colors[i]);
	}
	printf("Draw shaded triangle\n");
}

void GPU::sendRectToFrameBuffer()
{
	readColor();
	auto topLeft = readPoint();
	auto widthHeight = readPoint();
	u32 size = ((u32)widthHeight.x) * widthHeight.y;
	// make sure the number of pixels sent is even
	size = (size + 1) & ~1;
	assert(size != 0);
	isSendingImage = true;
	imageSizeLeft = size >> 1;
}

void GPU::sendFrameBufferToCPU()
{
	readColor();
	auto topLeft = readPoint();
	auto widthHeight = readPoint();
	u32 size = ((u32)widthHeight.x) * widthHeight.y;
	// make sure the number of pixels sent is even
	size = (size + 1) & ~1;
	printf("Framebuffer sent to CPU\n");
}
