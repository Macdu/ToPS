#include "Memory.h"

#include "Emulator.h"

void Memory::init(Emulator * emu)
{
	interrupt = emu->getInterrupt();
	bios = emu->getBios();
	ram = emu->getRam();
	timers = emu->getTimers();
	state = emu->getCPU()->getState();
	dma = emu->getDMA();
	gpu = emu->getGPU();
	controller = emu->getController();
	controller->init(emu);
	cdPlayer = emu->getCDPlayer();
}


u32 Memory::read32(u32 addr) const
{
	addr = removeRegion(addr);
	assert((addr & 0b11) == 0);
	// locate the addr
	if (addr < RAM::RAM_SIZE) {
		// RAM
		return ram->load32Ram(addr);
	}
	else if (addr >= 0x1FC00000 && addr - 0x1FC00000 < Bios::BIOS_SIZE) {
		// Bios
		return bios->read32(addr - 0x1FC00000);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		return ram->load32Scratchpad(addr - 0x1F800000);
	}
	else if (addr == 0x1F801070) {
		// I_STAT
		return (u32)interrupt->interruptStatus;
	}
	else if (addr == 0x1F801074) {
		// I_MASK
		return (u32)interrupt->interruptMask;
	}
	else if (addr >= 0x1F801080 && addr < 0x1F801100) {
		// DMA
		//printf("DMA read : 0x%08x\n", addr);
		fflush(stdout);
		if (addr == 0x1F8010F0) {
			return dma->primaryControlRegister;
		}
		else if (addr == 0x1F8010F4) {
			return dma->interruptControlRegister.toReg();
		}
		else {
			int channel = (addr >> 4) & 0b111;
			assert(channel != 7);
			switch ((channel >> 2) & 0b11) {
			case 0:
				// DMA base address
				return dma->memoryAddressRegisters[channel];
			case 1:
				// DMA Block Control
				return dma->blockControlRegisters[channel].reg;
			case 2:
				// DMA Channel Control
				return dma->getControlRegister(channel);
			default:
				throw_error("Wrong read to DMA register!");
			}
		}
	}
	else if (addr >= 0x1F801100 && addr <= 0x1F801138) {
		// Timer
		return (u32)timers->getReg(addr - 0x1F801100);
	}
	else if (addr >= 0x1F801810 && addr < 0x1F801820) {
		// GPU
		//printf("GPU read 0x%08x\n", addr);
		if (addr == 0x1F801810) {
			// GPUREAD port
			return gpu->getGPURead();
		}
		else if (addr == 0x1F801814) {
			// GPUStat register
			return gpu->getGPUStat();
		}
		return ~0;
	}
	else {
		throw_error("Failed to read address");
	}
}

u16 Memory::read16(u32 addr) const
{
	addr = removeRegion(addr);
	assert((addr & 1) == 0);
	if (addr < RAM::RAM_SIZE) {
		return ram->load16Ram(addr);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		return ram->load16Scratchpad(addr - 0x1F800000);
	}
	else if (addr >= 0x1F801C00 && addr < 0x1F802000) {
		// SPU
		return 0;
	}
	else if (addr == 0x1F801044) {
		// JOY_STAT
		return (u16)controller->getJoyStat();
	}
	else if (addr == 0x1F80104A) {
		// JOY_CTRL
		return controller->getJoyControl();
	}
	else if (addr == 0x1F801070) {
		// I_STAT
		return interrupt->interruptStatus;
	}
	else if (addr == 0x1F801074) {
		// I_MASK
		return interrupt->interruptMask;
	}
	else if (addr >= 0x1F801100 && addr <= 0x1F801138) {
		// Timer
		return timers->getReg(addr - 0x1F801100);
	}
	throw_error("Failed to read address");
}

u8 Memory::read8(u32 addr) const
{
	addr = removeRegion(addr);
	if (addr < RAM::RAM_SIZE) {
		// RAM
		return ram->load8Ram(addr);
	}
	else if (addr >= 0x1FC00000 && addr - 0x1FC00000 < Bios::BIOS_SIZE) {
		// Bios
		return bios->read8(addr - 0x1FC00000);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		return ram->load8Scratchpad(addr - 0x1F800000);
	}
	else if (addr >= 0x1f000000 && addr - 0x1f000000 < 0x100) {
		// Expansion 1
		return 0xFF;
	}
	else if (addr == 0x1F801040) {
		// JOY_RX_DATA
		return controller->readData();
	}
	else if (addr == 0x1F801800) {
		return cdPlayer->getCDReg0();
	}
	else if (addr == 0x1F801801) {
		return cdPlayer->getCDReg1();
	}
	else if (addr == 0x1F801802) {
		return cdPlayer->getCDReg2();
	}
	else if (addr == 0x1F801803) {
		return cdPlayer->getCDReg3();
	}
	printf("Attempt to read8 0x%08x\n", addr);
	throw_error("Fail to read address");
}

void Memory::write32(u32 addr, u32 value)
{
	addr = removeRegion(addr);
	assert((addr & 0b11) == 0);

	if ((state->cop0registers[12] & 0x00010000) != 0) {
		// if cache is isolated
		// printf("Write while cache is isolated addr : 0x%08x val: 0x%08x\n", addr, value);
		return;
	}

	// locate the addr
	if (addr <= RAM::RAM_SIZE) {
		// RAM
		ram->write32Ram(addr, value);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		ram->write32Scratchpad(addr - 0x1F800000, value);
	}
	else if (addr >= 0x1F801000 && addr <= 0x1F801060) {
		// various memory control registers
	}
	else if (addr == 0xFFFE0130) {
		// Cache control
	}
	else if (addr == 0x1F801070) {
		// I_STAT
		interrupt->setInterruptStatus((u16)value);
	} 
	else if (addr == 0x1F801074) {
		// I_MASK
		interrupt->setInterruptMask((u16)value);
	}
	else if (addr >= 0x1F801080 && addr < 0x1F801100) {
		// DMA
		//printf("DMA write : 0x%08x <- 0x%08x\n", addr, value);
		if (addr == 0x1F8010F0) {
			dma->primaryControlRegister = value;
		}
		else if (addr == 0x1F8010F4) {
			dma->interruptControlRegister.loadReg(value);
		}
		else {
			int channel = (addr >> 4) & 0b111;
			assert(channel != 7);
			switch ((addr >> 2) & 0b11) {
			case 0:
				// DMA base address
				dma->memoryAddressRegisters[channel] = value;
				break;
			case 1:
				// DMA Block Control
				dma->blockControlRegisters[channel].reg = value;
				break;
			case 2:
				// DMA Channel Control
				dma->setControlRegister(channel, value);
				break;
			default:
				throw_error("Wrong read to DMA register!");
			}
		}
	}
	else if (addr >= 0x1F801100 && addr <= 0x1F801138) {
		// Timer
		timers->setReg(addr - 0x1F801100, (u32)value);
	}
	else if (addr == 0x1F801810) {
		// GP0 command
		gpu->pushCmdGP0(value);
	}
	else if (addr == 0x1F801814) {
		// GP1 command
		gpu->gp1(value);
	}
	else {
		throw_error("Failed to write address");
	}
}

void Memory::write16(u32 addr, u16 value)
{
	addr = removeRegion(addr);
	assert((addr & 1) == 0);

	if ((state->cop0registers[12] & 0x00010000) != 0) {
		// if cache is isolated
		// printf("Write while cache is isolated addr : 0x%08x val: 0x%08x\n", addr, value);
		return;
	}

	// locate the addr
	if (addr <= RAM::RAM_SIZE) {
		// RAM
		ram->write16Ram(addr, value);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		ram->write16Scratchpad(addr - 0x1F800000, value);
	}
	else if (addr == 0x1F801048) {
		// JOY_MODE 
		// ignore it for now
	}
	else if (addr == 0x1F80104A) {
		// JOY_CTRL 
		controller->setJoyControl(value);
	}
	else if (addr == 0x1F80104E) {
		// JOY_BAUD
		// ignore it for now
	}
	else if (addr == 0x1F801070) {
		// I_STAT
		interrupt->setInterruptStatus(value);
	}
	else if (addr == 0x1F801074) {
		// I_MASK
		interrupt->setInterruptMask(value);
	}
	else if (addr >= 0x1F801100 && addr <= 0x1F801138) {
		// Timer
		timers->setReg(addr - 0x1F801100, value);
	}
	else if (addr >= 0x1F801D80 && addr < 0x1F802000) {
		// SPU
	}
	else if (addr >= 0x1f801c00 && addr < 0x1F801D80) {
		// SPU
	}
	else {
		printf("Adress 0x%08x not writeable\n", addr);
		throw_error("Failed to write address");
	}
}

void Memory::write8(u32 addr, u8 value)
{
	addr = removeRegion(addr);

	if ((state->cop0registers[12] & 0x00010000) != 0) {
		// if cache is isolated
		// printf("Write while cache is isolated addr : 0x%08x val: 0x%08x\n", addr, value);
		return;
	}

	// locate the addr
	if (addr <= RAM::RAM_SIZE) {
		// RAM
		ram->write8Ram(addr, value);
	}
	else if (addr >= 0x1F800000 && (addr - 0x1F800000) < RAM::SCRATCHPAD_SIZE) {
		// Scratchpad
		ram->write8Scratchpad(addr - 0x1F800000, value);
	}
	else if (addr >= 0x1F802000 && addr < 0x1F802060) {
		// Expansion Region 2 - Int/Dip/Post
	}
	else if (addr == 0x1F801040) {
		// JOY_TX_DATA
		controller->sendData(value);
	}
	else if (addr == 0x1F801800) {
		cdPlayer->setCDReg0(value);
	}
	else if (addr == 0x1F801801) {
		cdPlayer->setCDReg1(value);
	}
	else if (addr == 0x1F801802) {
		cdPlayer->setCDReg2(value);
	}
	else if (addr == 0x1F801803) {
		cdPlayer->setCDReg3(value);
	}
	else {
		printf("Attempt to write8 0x%08x : 0x%02x\n", addr, value);
		throw_error("Failed to write address");
	}
}

u32 Memory::removeRegion(u32 addr) const
{
	// keep everything (allow access to the 1024MB) of the addr for KUSEG2
	// for other memory regions, should only keep access to 512MB
	if ((addr >> 30) >= 3)
		return addr;
	return addr & ((1 << 29) - 1);
}
