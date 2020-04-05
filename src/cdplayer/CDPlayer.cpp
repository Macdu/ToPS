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
		if (response.pos == response.size) {
			return 0;
		}
		else {
			u8 res = response.content[response.pos++];
			if (response.pos == response.size) {
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
		if ((val & 0b111) != 0 && isResponseOccuring) {
			// clear the response fifo
			response.type = 0;
			isResponseOccuring = false;
			interruptFlagRegister.content.responseType = CDInterrupt::NoInterrupt;
			if (nextResponse.type != 0) {
				// switch to the next response
				response = nextResponse;
				nextResponse.type = 0;
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
	case 1:
		return interruptFlagRegister.val;
		break;
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
}

void CDPlayer::checkIRQ()
{
	if (response.type == 0 || *cpuClock < responseClock )return;
	isResponseOccuring = true;
	indexRegister.content.isTransmissionBusy = false;
	indexRegister.content.isResponseFIFONotEmpty = true;
	interruptFlagRegister.content.responseType = (CDInterrupt)response.type;
	// check if the bit corresponding to the current IRQ is set
	if ((response.type & interruptEnableRegister) == response.type) {
		interrupt->requestInterrupt(InterruptType::iCDROM);
	}
}