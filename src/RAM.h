#pragma once

#include "definitions.h"

class RAM {
public:
	static const int RAM_SIZE = 2048 * 1024;
	static const int SCRATCHPAD_SIZE = 1024;

	u32 load32Ram(u32 offset);
	u16 load16Ram(u32 offset);
	u8 load8Ram(u32 offset);
	u32 load32Scratchpad(u32 offset);
	void write32Ram(u32 offset, u32 value);
	void write16Ram(u32 offset, u16 value);
	void write8Ram(u32 offset, u8 value);
	void write32Scratchpad(u32 offset, u32 value);

private:
	u32 ram[RAM_SIZE / 4];
	u32 scratchpad[SCRATCHPAD_SIZE / 4];
};