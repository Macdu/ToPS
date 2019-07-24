#pragma once

#include "definitions.h"

class Bios;
class Emulator;
class RAM;
class DMA;
class GPU;
struct CPUState;

class Memory {

public:
	void init(Emulator* cpu);
	u32 read32(u32 addr) const;
	u16 read16(u32 addr) const;
	u8 read8(u32 addr) const;
	void write32(u32 addr, u32 value);
	void write16(u32 addr, u16 value);
	void write8(u32 addr, u8 value);

private:
	u32 removeRegion(u32 addr) const;
	Bios* bios;
	RAM* ram;
	DMA* dma;
	CPUState* state;
	GPU* gpu;
};