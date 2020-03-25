#include "Emulator.h"

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
	interrupt.init(&cpu);
	gpu.init(instance, surface);
	importBIOS();

	reset();

	// we need to let the cpu do its stuff before
	// According to Avocado, waiting until it gets to this address is enough
	while (cpu.getState()->pc != 0x80030000) {
		cpu.step();
	}
	printf("Bios startup done!\n");

	importEXE();
	//setDebugging(true);
}

void Emulator::importBIOS()
{
	u8* bios_content = new u8[Bios::BIOS_SIZE];
	FILE* file = fopen("SCPH1001.BIN", "rb");
	fread(bios_content,sizeof(u8) /* = 1 */,Bios::BIOS_SIZE, file);
	fclose(file);

	bios.load(bios_content);
	delete[] bios_content;
}

void Emulator::importEXE()
{
	// the ps1 exe header is 0x800 bytes long
	u8* header = new u8[0x800];
	FILE* file = fopen("prog_ps1.exe", "rb");
	fread(header, sizeof(u8), 0x800, file);

	// assert the header starts with "PS-X EXE" ascii
	constexpr char* beginning = "PS-X EXE";
	for (int i = 0; i < 8; i++) {
		assert((char)header[i] == beginning[i]);
	}

	u32* header32 = reinterpret_cast<u32*>(header);
	cpu.getState()->pc = header32[4];
	cpu.getState()->nextpc = cpu.getState()->pc + 4;
	cpu.getState()->registers[28] = header32[5];
	
	u32 copyDest = header32[6];
	u32 copySize = header32[7];
	assert((copySize & 0x7FF) == 0);

	// memfill is not useful here (the whole ram is made of 0)
	if (header32[12] != 0) {
		cpu.getState()->registers[29] = header32[12] + header32[13];
		cpu.getState()->registers[30] = header32[12] + header32[13];
	}
	else {
		// normally nothing should be done
		cpu.getState()->registers[29] = 0x801FFFF0;
		cpu.getState()->registers[30] = 0x801FFFF0;
	}

	u32* code = new u32[copySize >> 2];
	fread(code, sizeof(u8), copySize, file);
	for (u32 i = 0; i < (copySize >> 2); i++) {
		cpu.getMemory()->write32(copyDest + (i << 2), code[i]);
	}


	delete[] header;
	delete[] code;
	fclose(file);
	printf("Successfully loaded exe ps1 file!\n");
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

		if (scanline == GPU::scanlineVBlankStart) {
			gpu.drawFrame();
		}

		// a cpu cycle happens every 3 PS1 clock cycle
		for (int cpuCycle = 0; cpuCycle < GPU::cyclesPerScanline / 3; cpuCycle++) {
			int prevCycle = cpuCycle;
			for (; cpuCycle < std::max(GPU::cyclesPerScanline / 3,prevCycle + 100); cpuCycle++) {
				cpu.step();
			}
			// check for interrupts every 100 cycle
			controller.checkIRQ();
			interrupt.checkIRQ();
		}
	}
	
}

void Emulator::handleInput(ControllerKey key, bool isPressed)
{
	controller.handleInput(key, isPressed);
}
