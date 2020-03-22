#pragma once

#include "definitions.h"

class RAM {
public:
	// the first 2 MB of RAM are mirrored 3 other times
	static constexpr int RAM_SIZE = 2048 * 1024 * 4;
	static constexpr int SCRATCHPAD_SIZE = 1024;

	u32 load32Ram(u32 offset);
	u16 load16Ram(u32 offset);
	u8 load8Ram(u32 offset);
	u32 load32Scratchpad(u32 offset);
	u16 load16Scratchpad(u16 offset);
	u8 load8Scratchpad(u32 offset);
	void write32Ram(u32 offset, u32 value);
	void write16Ram(u32 offset, u16 value);
	void write8Ram(u32 offset, u8 value);
	void write32Scratchpad(u32 offset, u32 value);
	void write16Scratchpad(u32 offset, u16 value);
	void write8Scratchpad(u32 offset, u8 value);

private:
	u32 ram[RAM_SIZE / 4];
	u32 scratchpad[SCRATCHPAD_SIZE / 4];
	static constexpr int ram_mask = 2048 * 1024 - 1;
};