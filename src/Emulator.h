#pragma once

#include "cpu/CPU.h"
#include "Bios.h"
#include "RAM.h"
#include "DMA.h"
#include "gpu/GPU.h"
#include "Controller.h"
#include "Interrupt.h"

class RenderWindow;

class Emulator {
public:

	void init(RenderWindow* window, vk::Instance instance, vk::SurfaceKHR surface);

	// import the bios
	void importBIOS();

	// import an exe file
	void importEXE();

	// reset the emulator
	void reset();

	// called when the window is closed
	void destroy();

	// Executes the emulator until a frame is rendered
	void renderFrame();

	// Executes when a key is pressed or released
	void handleInput(ControllerKey key, bool isPressed);

	Bios* getBios() { return &bios; };
	RAM* getRam() { return &ram; };
	CPU* getCPU() { return &cpu; };
	DMA* getDMA() { return &dma; };
	GPU* getGPU() { return &gpu; };
	Controller* getController() { return &controller; };
	Interrupt* getInterrupt() { return &interrupt; };

private:
	CPU cpu;
	Interrupt interrupt;
	Bios bios;
	RAM ram;
	DMA dma;
	GPU gpu;
	Controller controller;
	RenderWindow* renderSurface;
};