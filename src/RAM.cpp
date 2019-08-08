#include "RAM.h"

u32 RAM::load32Ram(u32 offset)
{
	return ram[offset >> 2];
}

u16 RAM::load16Ram(u32 offset)
{
	return reinterpret_cast<u16*>(ram)[offset >> 1];
}

u8 RAM::load8Ram(u32 offset)
{
	return reinterpret_cast<u8*>(ram)[offset];
}

u32 RAM::load32Scratchpad(u32 offset)
{
	return scratchpad[offset >> 2];
}

void RAM::write32Ram(u32 offset, u32 value)
{
	ram[offset >> 2] = value;
}

void RAM::write16Ram(u32 offset, u16 value)
{
	reinterpret_cast<u16*>(ram)[offset >> 1] = value;
}

void RAM::write8Ram(u32 offset, u8 value)
{
	reinterpret_cast<u8*>(ram)[offset] = value;
}

void RAM::write32Scratchpad(u32 offset, u32 value)
{
	scratchpad[offset >> 2] = value;
}
