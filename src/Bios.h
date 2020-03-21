#pragma once

#include "definitions.h"

// implement the bios
class Bios {
public:
	static constexpr int BIOS_SIZE = 512 * 1024;

	void load(u8 content[]);

	u32 read32(u32 offset);
	u8 read8(u32 offset);

private:
	// contains the BIOS data
	u32 data[BIOS_SIZE / 4];
};