#pragma once

#include <queue>

#include "../definitions.h"

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
	Interrupt* interrupt;
	u64* cpuClock;

	static constexpr bool cdDebugging = true;
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

	std::queue<u8> parameterQueue;

	u64 responseClock;
	CDCmdResponse response;
	bool isResponseOccuring;

	// Contains the second response if the cmd has it
	CDCmdResponse nextResponse;

	// quick access to the index value
	inline u8 index() { return indexRegister.content.index; };

	// send an INT3 response
	inline void sendNormalResponse() {
		response.type = 3;
		response.pos = 0;
		indexRegister.content.isTransmissionBusy = true;
	}

	// send an INT2 response
	inline void sendAdditionalResponse() {
		nextResponse.type = 2;
		nextResponse.pos = 0;
	}

	// send a CDPlayer command
	void sendCommand(u8 cmd);
	void cmdTest();
	void cmdGetStat();
	void cmdGetID();

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
	void init(Interrupt* interrupt, u64* cpuClock);
	// check if there is a response pending
	void checkIRQ();
};