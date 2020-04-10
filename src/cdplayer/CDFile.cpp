#include "CDFile.h"

void CDFile::init(char* fileName)
{
	sector_end = 0x800;
	this->fileName = fileName;
	file = fopen(fileName, "rb");
	if (file == NULL) {
		throw_error("CD file does not exist.");
	}

	// get the file size
	fseek(file, 0L, SEEK_END);
	fileSize = (u32)ftell(file);
	rewind(file);
}

void CDFile::destroy()
{
	fclose(file);
}

void CDFile::setSector(const CDPos pos)
{
	// ignore the first 2 seconds
	sector = pos.toSector() - 150;
}
