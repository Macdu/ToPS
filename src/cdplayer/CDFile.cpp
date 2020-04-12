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
	needReload = true;
}

u8 CDFile::getData()
{
	// if we seeked to another place or read the whole sector
	if (needReload || currPos >= sector_end)
		loadSector();
	return data[currPos++];
}

void CDFile::loadSector()
{
	// if we read all of the sector, go to the next one
	if (currPos >= sector_end) {
		sector++;
	}
	currPos = 0;
	needReload = false;

	// set the file position to the wanted sector
	fseek(file, sector * sector_file_size, SEEK_SET);
	fread(data, sizeof(u8), sector_file_size, file);

	// we don't give back the header informations
	currPos = (sector_end == 0x800) ? 24 : 12;
	if (Debugging::cd)printf("CD: Sector read : 0x%x\n", sector);
}
