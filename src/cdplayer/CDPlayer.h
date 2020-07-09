#pragma once

#include <queue>

#include "../definitions.h"
#include "CDFile.h"

class Interrupt;

enum struct CDInterrupt : u8 {
	NoInterrupt = 0,
	// Received SECOND (or further) response to ReadS/ReadN (and Play+Report)
	SecondRespSpec = 1,
	// Received SECOND response (to various commands)
	SecondResponse = 2,
	// Received FIRST response (to any command)
	FirstResponse = 3,
	// when Play/Forward reaches end of disk (maybe also for Read?)
	DataEnd = 4,
	// Received error-code (in FIRST or SECOND response)
	ErrorCode = 5
};

struct CDCmdResponse {
	u8 content[16];
	u8 pos;
	u8 size;
	u8 type;
};

// implements a PSX CDPlayer
class CDPlayer {
private:
	CDFile cdFile;
	CDPos nextCDPos;

	Interrupt* interrupt;
	u64* cpuClock;

	// Bit3,4,5 are bound to 5bit counters; 
	// ie. the bits become true at specified amount of reads/writes, 
	// and thereafter once on every further 32 reads/writes.
	union {
		struct {
			u8 index : 2;
			// set when playing XA-ADPCM sound
			bool isADPFIFOBusy : 1;
			// triggered before writing 1st byte
			bool isParameterFIFOEmpty : 1;
			// triggered after writing 16 bytes
			bool isParameterFIFONotFull : 1;
			// triggered after reading LAST byte
			bool isResponseFIFONotEmpty : 1;
			// triggered after reading LAST byte
			bool isDataFIFONotEmpty : 1;
			bool isTransmissionBusy : 1;
		} content;
		u8 val;
	} indexRegister;

	// Write : 0x1F801802 - Index 1
	// Read : 0x1F801803 -Index 0-2
	u8 interruptEnableRegister;

	// Write : 0x1F801803 - Index1
	// Read : 0x1F801803 - Index1-3
	// Write 1 to aknowledge the interrupt
	// Response interrupts are queued (one needs to be acknowledged before the second one is sent)
	// After acknowledge, the Response Fifo is made empty, 
	// and if there's been a pending command, then that command gets send to the controller
	union {
		struct {
			CDInterrupt responseType : 3;
			bool unknown1 : 1;
			bool hasCommandStartIRQ : 1;
			// write 1 to bit 6 resets parameter FIFO
			bool unknown2 : 3;
		} content;

		u8 val;
	} interruptFlagRegister;

	enum struct CDStatusActivity : u8 {
		DoingNothing = 0,
		Reading = 1,
		Seeking = 2,
		Playing = 4
	};

	union {
		struct {
			// if is true, the error code is returned in the next byte
			bool hasError : 1;
			bool isMotorOn : 1;
			// if is true, the error code is returned in the next byte
			bool hasSeekError : 1;
			// set when GetID denied or when Setmode.bit4 is set
			bool hasIdError : 1;
			// if shell is closed, get set to 0 after GetStat
			bool isShellOpened : 1;
			CDStatusActivity activity : 3;
		} content;

		u8 val;
	} cdStat;

	// used for bit5 of sector size
	enum struct CdModeSectorSize : u8 {
		// size : 0x800
		DataOnly = 0,
		// size : 0x924, skips sync bytes
		WholeSector = 1
	};

	union {
		struct {
			bool allowCDA : 1;
			bool autoPause : 1;
			bool enableReportInterrupt : 1;
			bool enableXAFilter : 1;
			bool useless : 1;
			CdModeSectorSize sectorSize : 1;
			// 1=Send XA-ADPCM sectors to SPU Audio Input
			bool sendXA_ADPCM : 1;
			bool isDoubleSpeed : 1;
		} content;

		u8 val;
	} cdMode;

	// filter used when reading audio,
	// ignores sector if its file and channel are different from the filter one
	struct {
		u8 file;
		u8 channel;
	} filter;

	std::queue<u8> parameterQueue;

	u64 responseClock;
	std::queue<CDCmdResponse> responseQueue;
	CDCmdResponse currResponse;


	// Contains the clock for the next INT1 to send
	u64 nextDataClock;

	// quick access to the index value
	inline u8 index() { return indexRegister.content.index; };

	// send an INT3 response
	inline CDCmdResponse sendNormalResponse() {
		indexRegister.content.isTransmissionBusy = true;
		CDCmdResponse response;
		response.type = 3;
		response.pos = 0;
		return response;
	}

	// send an INT2 response
	inline CDCmdResponse sendAdditionalResponse() {
		CDCmdResponse response;
		response.type = 2;
		response.pos = 0;
		return response;
	}

	// send an INT1(stat)
	inline void sendINT1Stat() {
		auto response = sendNormalResponse();
		response.type = 1;
		response.size = 1;
		response.content[0] = cdStat.val;
		responseQueue.push(response);
	}

	// send an INT3(stat)
	inline void sendINT3Stat() {
		auto response = sendNormalResponse();
		response.size = 1;
		response.content[0] = cdStat.val;
		responseQueue.push(response);
	}

	// send an INT2(stat)
	inline void sendINT2Stat() {
		auto response = sendAdditionalResponse();
		response.size = 1;
		response.content[0] = cdStat.val;
		responseQueue.push(response);
	}

	// send a CDPlayer command
	void sendCommand(u8 cmd);
	void cmdTest();
	void cmdGetStat();
	void cmdGetID();
	void cmdSetMode();
	void cmdStop();
	void cmdInit();
	void cmdDemute();
	void cmdSetFilter();
	void cmdSetLoc();
	void cmdSeekL();
	void cmdRead();
	void cmdPause();

public:
	// Address 0x1F801800
	void setCDReg0(u8 val);
	// Address 0x1F801800
	u8 getCDReg0();
	// Address 0x1F801801
	void setCDReg1(u8 val);
	// Address 0x1F801801
	u8 getCDReg1();
	// Address 0x1F801802
	void setCDReg2(u8 val);
	// Address 0x1F801802
	u8 getCDReg2();
	// Address 0x1F801803
	void setCDReg3(u8 val);
	// Address 0x1F801803
	u8 getCDReg3();
	inline u8 readData() {
		return cdFile.getData();
	}

	void init(Interrupt* interrupt, u64* cpuClock);
	void destroy();
	// check if there is a response pending
	void checkIRQ();
};