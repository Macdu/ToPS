#include "CDPlayer.h"

void CDPlayer::sendCommand(u8 cmd)
{
	switch (cmd) {
	case 0x01:
		cmdGetStat();
		break;
	case 0x19: 
		cmdTest();
		break;
	case 0x1A:
		cmdGetID();
		break;
	default:
		printf("Unknown CD cmd : %02x\n", cmd);
		throw_error("Unknown CD cmd");
	}
	// empty the parameter queue
	while (!parameterQueue.empty())parameterQueue.pop();
	indexRegister.content.isParameterFIFOEmpty = true;
	// set a delay before saying the response is available
	responseClock = *cpuClock + 500;
}

void CDPlayer::cmdTest()
{
	switch (parameterQueue.front()) {
	case 0x20:
		// BCD of the CDROM controller BIOS
		// value from Nocash doc
		if (cdDebugging)printf("CD: BIOS Time Test\n");
		sendNormalResponse();
		response.size = 4;
		response.content[0] = 0x94;
		response.content[1] = 0x09;
		response.content[2] = 0x19;
		response.content[3] = 0xC0;
		break;
	default:
		throw_error("Unknown test command");
	}
}

void CDPlayer::cmdGetStat()
{
	if (Debugging::cd)printf("CD: GetStat\n");
	sendNormalResponse();
	response.size = 1;
	response.content[0] = cdStat.val;
	// reset the shellOpened bit
	cdStat.content.isShellOpened = false;
}

void CDPlayer::cmdGetID()
{
	if (Debugging::cd)printf("CD: GetID\n");
	cdStat.content.isMotorOn = true;
	cdStat.val = 0x08;
	sendNormalResponse();
	response.size = 1;
	response.content[0] = cdStat.val;
	sendAdditionalResponse();
	nextResponse.size = 8;
	// stat,flags,type,atip,"SCEx"
	// values from NoCash doc
	/*
	nextResponse.content[0] = 0x02;
	nextResponse.content[1] = 0x00;
	nextResponse.content[2] = 0x20;
	nextResponse.content[3] = 0x00;
	nextResponse.content[4] = 'S';
	nextResponse.content[5] = 'C';
	nextResponse.content[6] = 'E';
	nextResponse.content[7] = 'A';
	*/
	nextResponse.type = 5;
	nextResponse.content[0] = 0x08;
	nextResponse.content[1] = 0x40;
	for (int i = 2; i < 8; i++)nextResponse.content[i] = 0x00;
}