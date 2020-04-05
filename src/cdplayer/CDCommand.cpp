#include "CDPlayer.h"

void CDPlayer::sendCommand(u8 cmd)
{
	switch (cmd) {
	case 0x01:
		cmdGetStat();
		break;
	case 0x08:
		cmdStop();
		break;
	case 0x0A:
		cmdInit();
		break;
	case 0x0c:
		cmdDemute();
		break;
	case 0x0E:
		cmdSetMode();
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
	responseClock = *cpuClock + 1000;
}

void CDPlayer::cmdTest()
{
	switch (parameterQueue.front()) {
	case 0x20:
		// BCD of the CDROM controller BIOS
		// value from Nocash doc
		if (Debugging::cd)printf("CD: BIOS Time Test\n");
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
	sendINT3Stat();
	// reset the shellOpened bit
	cdStat.content.isShellOpened = false;
}

void CDPlayer::cmdGetID()
{
	if (Debugging::cd)printf("CD: GetID\n");
	cdStat.content.isMotorOn = true;
	sendINT3Stat();

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
	nextResponse.content[7] = 'A';*/
	// this part is if you want no CD in the drive
	
	cdStat.val = 0x08;
	nextResponse.type = 5;
	nextResponse.content[0] = 0x08;
	nextResponse.content[1] = 0x40;
	for (int i = 2; i < 8; i++)nextResponse.content[i] = 0x00;
	
}

void CDPlayer::cmdSetMode()
{
	if (Debugging::cd)printf("CD: SetMode(0x%02x)\n", parameterQueue.front());
	cdMode.val = parameterQueue.front();
	sendINT3Stat();
}

void CDPlayer::cmdStop()
{
	if (Debugging::cd)printf("CD: Stop\n");
	cdStat.content.activity = CDStatusActivity::DoingNothing;
	sendINT3Stat();
	cdStat.content.isMotorOn = false;
	sendINT2Stat();
}

void CDPlayer::cmdInit()
{
	if (Debugging::cd)printf("CD: Init\n");
	cdMode.val = 0;
	sendINT3Stat();
	cdStat.content.isMotorOn = true;
	sendINT2Stat();
}

void CDPlayer::cmdDemute()
{
	if (Debugging::cd)printf("CD: Demute\n");
	sendINT3Stat();
	cdMode.content.allowCDA = true;
	cdMode.content.sendXA_ADPCM = true;
}