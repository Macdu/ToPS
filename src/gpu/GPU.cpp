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

	gp0_opcodes_length[0x68] = 2;

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
	gpuProps.reset();
	gpuReadDataSize = 0;
	isSendingImage = false;
}

void GPU::init(vk::Instance instance, vk::SurfaceKHR surface)
{
	renderer.setInstance(instance);
	renderer.setSurface(surface);
	renderer.setWidth(1024);
	renderer.setHeight(512);
	renderer.gpuProps = &gpuProps;
	renderer.initVulkan();

	// read to render screen rendering
	renderer.sceneRendering.verticesToRenderSize = 6;
	renderer.sceneRendering.verticesToRender[0] = {
		{0,0},
		{0,0,0},
		{0,0},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesToRender[1] = {
		{1024,0},
		{0,0,0},
		{1024,0},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesToRender[2] = {
		{0,512},
		{0,0,0},
		{0,512},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesToRender[3] = {
		{1024,0},
		{0,0,0},
		{1024,0},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesToRender[4] = {
		{0,512},
		{0,0,0},
		{0,512},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesToRender[5] = {
		{1024,512},
		{0,0,0},
		{1024,512},
		0,
		(1 << 8) // 15-bit render
	};
	renderer.sceneRendering.verticesRenderScissors.resize(1);
	renderer.sceneRendering.verticesRenderScissors[0] =
		{ 5, {{0,0}, {1024,512}} };

	reset();
}

u32 GPU::getGPUStat() {
	return gpuProps.getGPUStat();
}

u32 GPU::getGPURead()
{
	if (gpuReadDataSize == 0) {
		// return random data
		return ~0;
	}
	u32 res = gpuReadData[gpuReadDataCurr];
	gpuReadDataCurr++;
	if (gpuReadDataCurr == gpuReadDataSize) {
		gpuReadDataSize = 0;
	}
	return res;
}

void GPU::setScanline(int scanline)
{
	assert(scanline >= 0 && scanline < totalNTSCScanlines);
	if (scanline >= scanlineVBlankStart) {
		gpuProps.drawingOdd = false;
	}
	else {
		// depends if we are in interlaced mode or not
		if (gpuProps.hasVerticalInterlace) {
			gpuProps.drawingOdd = (bool)(scanline % 2);
		}
		// otherwise it changes at the next frame
	}
}

inline void GPU::pushVertexColor(const Point<i16>& point,const Color & color)
{
	renderer.sceneRendering
		.verticesToRender[renderer.sceneRendering.verticesToRenderSize++] =
	{ 
		point.toIVec2() + gpuProps.drawingOffset, 
		color.toUVec3(), 
		{0,0},0, // not used
		1 << 16 // color rendering  
	};
}

inline void GPU::pushVertexTexture(const Point<i16>& point, const Point<u8>& textLoc, const u16& clutId, const u16& textPage)
{
	renderer.sceneRendering
		.verticesToRender[renderer.sceneRendering.verticesToRenderSize++] =
	{
		point.toIVec2() + gpuProps.drawingOffset,
		{42,42,42},
		{(uint)textLoc.x, (uint)textLoc.y},
		(uint)clutId,
		(uint)textPage
	};
}

void GPU::drawFrame() {
	//printf("Drawing frame.\n");
	renderer.drawFrame();
	// keep only the read to render screen rendering
	renderer.sceneRendering.verticesToRenderSize = 6;
	renderer.sceneRendering.verticesRenderScissors.resize(1);
	totalFrames++;

	// set the drawOdd bit if needed
	if (!gpuProps.hasVerticalInterlace) {
		gpuProps.drawingOdd = (bool)(totalFrames % 2);
	}
}

void GPU::pushCmdGP0(u32 val)
{
	if (isSendingImage) {
		// special case, ignore it for now
		reinterpret_cast<u32*>(imageTransfer)[currentImageSize >> 1] = val;
		currentImageSize += 2;
		if (currentImageSize == totalImageSize) {
			isSendingImage = false;
			renderer.sceneRendering.transferImage(imageTransfer, imageTopLeft, imageExtent);
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

	case 0x68:
		// dot
		dot();
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
		gpuProps.setDrawModeSetting(cmd);
		break;

	case 0xE2:
		// Texture Window setting
		gpuProps.textureMaskX = cmd & ((1 << 5) - 1);
		gpuProps.textureMaskY = (cmd >> 5) & ((1 << 5) - 1);
		gpuProps.textureOffsetX = (cmd >> 10) & ((1 << 5) - 1);
		gpuProps.textureOffsetY = (cmd >> 15) & ((1 << 5) - 1);
		break;

	case 0xE3:
		// Set Drawing Area top left (X1,Y1)
		gpuProps.drawingAreaLeft = cmd & ((1 << 10) - 1);
		gpuProps.drawingAreaTop = (cmd >> 10) & ((1 << 10) - 1);
		renderer.sceneRendering.updateDrawingArea();
		break;

	case 0xE4:
		// Set Drawing Area bottom right (X2,Y2)
		gpuProps.drawingAreaRight = cmd & ((1 << 10) - 1);
		gpuProps.drawingAreaBottom = (cmd >> 10) & ((1 << 10) - 1);
		renderer.sceneRendering.updateDrawingArea();
		break;

	case 0xE5:
		// Set Drawing Offset (X,Y)
		gpuProps.drawingOffset.x = cmd & ((1 << 11) - 1);
		if (gpuProps.drawingOffset.x >= (1 << 10)) {
			// 11-bit sign extension
			gpuProps.drawingOffset.x = -(((1 << 11) - 1) - gpuProps.drawingOffset.x + 1);
		}
		gpuProps.drawingOffset.y = (cmd >> 11) & ((1 << 11) - 1);
		if (gpuProps.drawingOffset.y >= (1 << 10)) {
			// 11-bit sign extension
			gpuProps.drawingOffset.y = -(((1 << 11) - 1) - gpuProps.drawingOffset.y + 1);
		}

		break;

	case 0xE6:
		// Mask Bit Setting
		gpuProps.doMaskWhenDrawing = cmd & 1;
		gpuProps.cannotDrawToMasked = (cmd & 2) != 0;
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
		gpuProps.reset();
		renderer.sceneRendering.updateDrawingArea();
		break;

	case 0x01:
		// Reset command buffer
		while (!gp0Queue.empty()) {
			gp0Queue.pop();
		}
		break;

	case 0x02:
		// acknowledge GPU IRQ interrupt
		gpuProps.interrupRequest = false;
		break;

	case 0x03:
		// Display Enable
		gpuProps.isDisplayDisabled = cmd & 1;
		break;

	case 0x04:
		//  DMA Direction / Data Request
		gpuProps.dmaDirection = cmd & 3;
		break;

	case 0x05:
		// Start of Display area (in VRAM)
		gpuProps.displayAreaTopLeftX = cmd & ((1 << 10)-1);
		gpuProps.displayAreaTopLeftY = (cmd >> 10) & ((1 << 9) - 1);
		break;

	case 0x06:
		// Horizontal Display range (on Screen)
		gpuProps.horizontalDisplayStart = cmd & ((1 << 12) - 1);
		gpuProps.horizontalDisplayEnd = (cmd >> 12) & ((1 << 12) - 1);
		break;

	case 0x07:
		// Vertical Display range (on Screen)
		gpuProps.verticalDisplayStart = cmd & ((1 << 10) - 1);
		gpuProps.verticalDisplayEnd = (cmd >> 10) & ((1 << 10) - 1);
		break;

	case 0x08:
		// Display mode
		gpuProps.setDisplayMode(cmd);
		break;

	case 0x10: {
		u32 retVal;
		// GPU info
		switch (cmd & ((1 << 24) - 1)) {
		case 7:
			// GPU Type
			// no$cash says it should be 2
			retVal = 2;
			break;
		default:
			throw_error("Unimplemented GPU_info type!\n");
			break;
		}
		gpuReadDataSize = 1;
		gpuReadDataCurr = 0;
		gpuReadData[0] = retVal;
		break;
	}

	default:
		printf("Opcode 0x%02x for cmd 0x%08x GP1 not valid\n", opcode, cmd);
		throw_error("Unrecognized opcode!");
	}
}

void GPU::textured4Points()
{
	Point<i16> vertices[4];
	Point<u8> textLocs[4];
	u16 clutID;
	u16 textPage;
	u32 word;
	// Command + color vertex 0
	gp0Queue.pop();
	vertices[0] = readPoint();
	// CULT ID + texture coordinates vertex 0
	word = gp0Queue.front();
	gp0Queue.pop();
	clutID = word >> 16;
	textLocs[0] = extractTextLoc(word);

	vertices[1] = readPoint();

	// Texture page + texture coordinates vertex 1
	word = gp0Queue.front();
	gp0Queue.pop();
	textPage = word >> 16;
	textLocs[1] = extractTextLoc(word);

	vertices[2] = readPoint();
	// Texture coordinates vertex 2
	word = gp0Queue.front();
	gp0Queue.pop();
	textLocs[2] = extractTextLoc(word);

	vertices[3] = readPoint();
	// Texture coordinates vertex 4
	word = gp0Queue.front();
	gp0Queue.pop();
	textLocs[3] = extractTextLoc(word);
	for (int i = 0; i < 3; i++) {
		pushVertexTexture(vertices[i], textLocs[i], clutID, textPage);
	}
	for (int i = 1; i < 4; i++) {
		pushVertexTexture(vertices[i], textLocs[i], clutID, textPage);
	}
	printf("Draw textured quad : (%d,%d) (%d,%d) -> (%d,%d) (%d,%d)\n",
		textLocs[0].x, textLocs[0].y, textLocs[3].x, textLocs[3].y,
		vertices[0].x, vertices[0].y, vertices[3].x, vertices[3].y);
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
		pushVertexColor(vertices[i], color);
	}
	for (int i = 1; i < 4; i++) {
		pushVertexColor(vertices[i], color);
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
		pushVertexColor(vertices[i], colors[i]);
	}
	for (int i = 1; i < 4; i++) {
		pushVertexColor(vertices[i], colors[i]);
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
		pushVertexColor(vertices[i], colors[i]);
	}
	printf("Draw shaded triangle\n");
}

void GPU::dot()
{
	Color color = readColor();
	Point<i16> vertex = readPoint();
	// for (int i = 0; i < 3; i++) {
	// 	pushVertexColor(vertex, color);
	// }
	pushVertexColor(vertex, color);
	pushVertexColor({ vertex.x + 2, vertex.y }, color);
	pushVertexColor({ vertex.x, vertex.y + 2 }, color);
}

void GPU::sendRectToFrameBuffer()
{
	readColor();
	imageTopLeft = readPoint();
	imageExtent = readPoint();
	printf("Send image to framebuffer (%d,%d) [%d,%d]\n",
		imageTopLeft.x, imageTopLeft.y, imageExtent.x, imageExtent.y);
	u32 size = ((u32)imageExtent.x) * imageExtent.y;
	// make sure the number of pixels sent is even
	size = (size + 1) & ~1;
	assert(size != 0);
	currentImageSize = 0;
	totalImageSize = size;
	isSendingImage = true;
}

void GPU::sendFrameBufferToCPU()
{
	readColor();
	auto topLeft = readPoint();
	auto extent = readPoint();
	u32 size = ((u32)imageExtent.x) * imageExtent.y;
	// make sure the number of pixels sent is even
	size = (size + 1) & ~1;
	printf("Framebuffer sent to CPU\n");
	gpuReadDataCurr = 0;
	gpuReadDataSize = size >> 1;
	renderer.sceneRendering.readFramebuffer(gpuReadData, topLeft, extent);
}
