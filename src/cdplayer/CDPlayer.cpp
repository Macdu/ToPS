#include "CDPlayer.h"

#include "../Interrupt.h"

void CDPlayer::setCDReg0(u8 val)
{
	// only bits 0-1 can be written to
	indexRegister.content.index = val & 3;
}

u8 CDPlayer::getCDReg0()
{
	return indexRegister.val;
}

void CDPlayer::setCDReg1(u8 val)
{
	switch (index())
	{
	case 0:
		sendCommand(val);
		break;
	case 3:
		// volume
		break;
	default:
		throw_error("Unimplemented CDReg1 modification!");
		break;
	}
}

u8 CDPlayer::getCDReg1()
{
	switch (index()) {
	case 1:
		// returns response
		if (currResponse.pos == currResponse.size) {
			return 0;
		}
		else {
			u8 res = currResponse.content[currResponse.pos++];
			if (currResponse.pos == currResponse.size) {
				// everything was read
				indexRegister.content.isResponseFIFONotEmpty = false;
			}
			return res;
		}
	default:
		throw_error("Unimplemented CDReg1 access!");
		return 0;
	}
}

void CDPlayer::setCDReg2(u8 val)
{
	switch (index()) {
	case 0:
		parameterQueue.push(val);
		indexRegister.content.isParameterFIFOEmpty = false;
		break;
	case 1:
		interruptEnableRegister = val;
		break;
	case 2:
	case 3:
		// volume
		break;
	default:
		throw_error("Unimplemented CDReg2 modification!");
		break;
	}
}

u8 CDPlayer::getCDReg2()
{
	switch (index())
	{
	default:
		throw_error("Unimplemented CDReg2 access!");
		return 0;
	}
}

void CDPlayer::setCDReg3(u8 val)
{
	switch (index()) {
	case 0:
		// TODO: remove
		break;
	case 1:
		interruptFlagRegister.val &= ~val;
		if ((val & 0b111) != 0 && currResponse.type != 0) {
			// clear the response fifo
			currResponse.type = 0;
			interruptFlagRegister.content.responseType = CDInterrupt::NoInterrupt;
			if (!responseQueue.empty()) {
				// set a new delay
				responseClock = *cpuClock + 1000;
			}
		}
		break;
	case 2:
	case 3:
		// volume
		break;
	default:
		throw_error("Unimplemented CDReg3 modification!");
		break;
	}
}

u8 CDPlayer::getCDReg3()
{
	switch (index())
	{
	case 0:
		return interruptEnableRegister;
	case 1:
		return interruptFlagRegister.val;
	default:
		throw_error("Unimplemented CDReg3 access!");
		return 0;
	}
}

void CDPlayer::init(Interrupt* interrupt, u64* cpuClock)
{
	this->cpuClock = cpuClock;
	this->interrupt = interrupt;
	indexRegister.content.isParameterFIFOEmpty = true;
	indexRegister.content.isParameterFIFONotFull = true;
	cdStat.content.isShellOpened = true;
	interruptEnableRegister = 0x1F;
	cdFile.init("game/game.bin");
}

void CDPlayer::destroy()
{
	cdFile.destroy();
}

void CDPlayer::checkIRQ()
{
	if (currResponse.type == 0 
		&& !responseQueue.empty()
		&& *cpuClock >= responseClock) {
		// switch to the next response
		currResponse = responseQueue.front();
		responseQueue.pop();

	}

	if (currResponse.type != 0) {

		indexRegister.content.isTransmissionBusy = false;
		indexRegister.content.isResponseFIFONotEmpty = true;
		interruptFlagRegister.content.responseType = (CDInterrupt)currResponse.type;
		// check if the bit corresponding to the current IRQ is set
		if ((currResponse.type & interruptEnableRegister) == currResponse.type) {
			interrupt->requestInterrupt(InterruptType::iCDROM);
		}
	}

	if (cdStat.content.activity == CDStatusActivity::Reading 
		&& currResponse.type == 0
		&& *cpuClock >= nextDataClock) {
		sendINT1Stat();
		indexRegister.content.isDataFIFONotEmpty = true;
		// should be about 1/75th of a second
		// I put half less just in case
		nextDataClock = *cpuClock + 200000;
	}
}