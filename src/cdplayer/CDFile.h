#pragma once

#include <cstdio>

#include "../definitions.h"

// represent the reading position of the CD
// there are 75 sectors in one second and 60 seconds in one minute
struct CDPos {
	u8 minutes;
	u8 seconds;
	u8 sectors;

	// convert the position to an absolute sector offset from the beginning of the disk
	u32 toSector() const {
		return (u32)bcdToHex(sectors) 
			+ (u32)bcdToHex(seconds) * 75 
			+ (u32)bcdToHex(minutes) * 75 * 60;
	}
};



// handle access to the CD bin/cue files
class CDFile {
public:

	u32 sector_end;

	void init(char* fileName);
	void destroy();

	void setSector(const CDPos pos);
	u8 getData();
private:
	char* fileName;
	FILE* file;

	u32 fileSize;

	// The offset from the beginning of the sector (12 or 24 bytes containing channel or subchannel info)
	u32 sectorOffset;
	u32 sector;
	u32 currPos;
	bool needReload;

	static constexpr u32 sector_file_size = 0x930;
	u8 data[sector_file_size];
	
	void loadSector();
};
