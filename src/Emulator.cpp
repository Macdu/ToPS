#include "Emulator.h"

#include <Windows.h>
#include <cstdio>

#include "definitions.h"
#include "qt/RenderWindow.h"

void Emulator::init(RenderWindow* window, vk::Instance instance, vk::SurfaceKHR surface)
{
	this->renderSurface = window;
	cpu.init(this);
	dma.ram = &ram;
	dma.gpu = &gpu;
	gpu.emu = this;
	gpu.init(instance, surface);
	importBIOS();
	
	reset();
}

void Emulator::importBIOS()
{
	u8* bios_content = new u8[Bios::BIOS_SIZE];
	FILE* file = fopen("SCPH1001.BIN", "r");
	fread(bios_content,sizeof(u8) /* = 1 */,Bios::BIOS_SIZE, file);
	fclose(file);

	bios.load(bios_content);
	delete[] bios_content;
}

void Emulator::reset()
{
	cpu.reset();
	dma.reset();
	gpu.reset();
}

void Emulator::destroy()
{
	gpu.renderer.cleanup();
}

void Emulator::renderFrame()
{
	for (int scanline = 0; scanline < GPU::totalNTSCScanlines; scanline++) {
		gpu.setScanline(scanline);
		// a cpu cycle happens every 3 PS1 clock cycle
		for (int cpuCycle = 0; cpuCycle < GPU::cyclesPerScanline / 3; cpuCycle++) {
			cpu.step();
		}
	}
	// drawing frame when VBlank start may be better
	gpu.drawFrame();
}
