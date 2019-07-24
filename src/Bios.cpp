#include "Bios.h"

#include <cstring>

void Bios::load(u8 content[])
{
	memcpy(data, content, BIOS_SIZE);
}

u32 Bios::read32(u32 offset)
{
	return data[offset >> 2];
}

u8 Bios::read8(u32 offset)
{
	return reinterpret_cast<u8*>(data)[offset];
}
