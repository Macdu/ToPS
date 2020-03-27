#pragma once

#include "definitions.h"

class Interrupt;
class Timers;
class Bios;
class Emulator;
class RAM;
class DMA;
class GPU;
class Controller;
struct CPUState;

class Memory {

public:
	void init(Emulator* emu);
	u32 read32(u32 addr) const;
	u16 read16(u32 addr) const;
	u8 read8(u32 addr) const;
	void write32(u32 addr, u32 value);
	void write16(u32 addr, u16 value);
	void write8(u32 addr, u8 value);

private:
	u32 removeRegion(u32 addr) const;
	Interrupt* interrupt;
	Bios* bios;
	RAM* ram;
	DMA* dma;
	CPUState* state;
	GPU* gpu;
	Controller* controller;
	Timers* timers;
};