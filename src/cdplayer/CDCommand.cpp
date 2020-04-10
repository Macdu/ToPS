#include "CDPlayer.h"

void CDPlayer::sendCommand(u8 cmd)
{
	switch (cmd) {
	case 0x01:
		cmdGetStat();
		break;
	case 0x02:
		cmdSetLoc();
		break;
	case 0x06:
		cmdRead();
		break;
	case 0x08:
		cmdStop();
		break;
	case 0x09:
		cmdPause();
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
	case 0x15:
		cmdSeekL();
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
	// send the first response after a delay
	responseClock = *cpuClock + 1000;
}

void CDPlayer::cmdTest()
{
	switch (parameterQueue.front()) {
	case 0x20: {
		// BCD of the CDROM controller BIOS
		// value from Nocash doc
		if (Debugging::cd)printf("CD: BIOS Time Test\n");
		CDCmdResponse response = sendNormalResponse();
		response.size = 4;
		response.content[0] = 0x94;
		response.content[1] = 0x09;
		response.content[2] = 0x19;
		response.content[3] = 0xC0;
		responseQueue.push(response);
		break;
	}
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

	auto response = sendAdditionalResponse();
	response.size = 8;
	// stat,flags,type,atip,"SCEx"
	// values from NoCash doc
	
	response.content[0] = 0x02;
	response.content[1] = 0x00;
	response.content[2] = 0x20;
	response.content[3] = 0x00;
	response.content[4] = 'S';
	response.content[5] = 'C';
	response.content[6] = 'E';
	response.content[7] = 'A';
	// this part is if you want no CD in the drive
	/*
	cdStat.val = 0x08;
	response.type = 5;
	response.content[0] = 0x08;
	response.content[1] = 0x40;
	for (int i = 2; i < 8; i++)response.content[i] = 0x00;
	*/
	responseQueue.push(response);
	
}

void CDPlayer::cmdSetMode()
{
	if (Debugging::cd)printf("CD: SetMode(0x%02x)\n", parameterQueue.front());
	cdMode.val = parameterQueue.front();
	// update the interesting sector size
	cdFile.sector_end = (cdMode.content.sectorSize == CdModeSectorSize::DataOnly) ? 0x800 : 0x924;
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
	cdFile.sector_end = 0x800;
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

void CDPlayer::cmdSetLoc()
{
	nextCDPos.minutes = parameterQueue.front();
	parameterQueue.pop();
	nextCDPos.seconds = parameterQueue.front();
	parameterQueue.pop();
	nextCDPos.sectors = parameterQueue.front();
	parameterQueue.pop();
	if (Debugging::cd)printf("CD: SetLoc (m=%02x, sd=%02x, st=%02x)\n",
		nextCDPos.minutes, nextCDPos.seconds, nextCDPos.sectors);

	sendINT3Stat();

	// already start the seeking, shouldn't be an issue
	// this should normally happen during next seek or read
	cdFile.setSector(nextCDPos);
}

void CDPlayer::cmdSeekL()
{
	if (Debugging::cd)printf("CD: SeekL\n");
	sendINT3Stat();

	cdStat.content.activity = CDStatusActivity::Seeking;
	sendINT2Stat();
}

void CDPlayer::cmdRead()
{
	if (Debugging::cd)printf("CD: Read\n");
	sendINT3Stat();

	indexRegister.content.isDataFIFONotEmpty = false;
	cdStat.content.activity = CDStatusActivity::Reading;
	nextDataClock = *cpuClock + 200000;
}

void CDPlayer::cmdPause()
{
	if (Debugging::cd)printf("CD: Pause\n");
	sendINT3Stat();

	cdStat.content.activity = CDStatusActivity::DoingNothing;
	sendINT2Stat();
}
