#include "DMA.h"

#include "RAM.h"
#include "gpu/GPU.h"

// for debugging purposes
#include "cpu/interpreter/Interpreter.h"

void ChannelControlRegister::loadReg(u32 reg)
{
	direction = static_cast<DMATransferDirection>(reg & 1);
	isGoingBackward = (reg & 2) != 0;
	syncMode = static_cast<DMASyncMode>((reg >> 9) & 0b11);
	enabled = (reg & (1 << 24)) != 0;
	trigger = (reg & (1 << 28)) != 0;
}

u32 ChannelControlRegister::toReg() const
{
	u32 res = 0;
	res |= static_cast<u32>(direction);
	res |= static_cast<u32>(isGoingBackward) << 1;
	res |= static_cast<u32>(syncMode) << 9;
	res |= static_cast<u32>(enabled) << 24;
	res |= static_cast<u32>(trigger) << 28;
	return res;
}


void DMAInterruptRegister::loadReg(u32 reg)
{
	forceIRQ = (reg & (1 << 15)) != 0;
	irqEnable = static_cast<u8>(reg >> 16) & 0x7f;

	irqMasterEnable = (reg & (1 << 23)) != 0;
	irqFlag &= ~static_cast<u8>(reg >> 24) & 0x7f;
}

u32 DMAInterruptRegister::toReg() const
{
	u32 res = 0;
	res |= static_cast<u32>(forceIRQ) << 15;
	res |= static_cast<u32>(irqMasterEnable) << 23;

	res |= static_cast<u32>(irqEnable) << 16;
	res |= static_cast<u32>(irqFlag) << 24;

	// IRQ master flag
	res |= static_cast<u32>(forceIRQ || (irqMasterEnable && (irqEnable & irqFlag) != 0)) << 31;
	return res;
}


void DMA::reset()
{
	// Default value
	primaryControlRegister = 0x07654321;
}

void DMA::setControlRegister(int index, u32 val)
{
	channelControlRegisters[index].loadReg(val);
	// checks if a dma can start
	// happens when the enabled bit is on
	// and if the sync mod is manual (=0), trigger bit must also be set on
	if (channelControlRegisters[index].enabled 
		&& (channelControlRegisters[index].syncMode != DMASyncMode::MANUAL 
			|| channelControlRegisters[index].trigger)) {
		makeDMA(index);
	}
}

u32 DMA::getControlRegister(int index) const
{
	return channelControlRegisters[index].toReg();
}

inline u32 DMA::getDMASize(int channel)
{
	switch (channelControlRegisters[channel].syncMode) {
	case MANUAL:
		return blockControlRegisters[channel].content.lowBlock;
	case REQUEST:
		return blockControlRegisters[channel].content.lowBlock*blockControlRegisters[channel].content.highBlock;
	}
}

void DMA::makeDMA(int channel)
{
	//printf("DMA happening, channel %d\n", channel);
	switch (channelControlRegisters[channel].syncMode) {
		case LINKED_LIST:
			dmaLinkedList(channel);
			break;
		default:
			dmaCopyBlock(channel);
			break;
	}
	// set the DMA as finished
	channelControlRegisters[channel].trigger = false;
	channelControlRegisters[channel].enabled = false;
	// check if we need to change interrupt register
	if ((interruptControlRegister.irqEnable & (1 << channel)) != 0) {
		interruptControlRegister.irqFlag |= 1 << channel;
	}
}

void DMA::dmaCopyBlock(int channel)
{
	u32 addr = memoryAddressRegisters[channel];
	u32 toAdd = channelControlRegisters[channel].isGoingBackward ? -4 : 4;
	u32 size = getDMASize(channel);
	bool isToRam = channelControlRegisters[channel].direction == TO_RAM;

	for (u32 index = 0; index < size; index++) {
		addr &= 0x1ffffc;
		if (isToRam) {
			u32 val = 0;
			switch (static_cast<DMAChannel>(channel)) {
			case GPU_OTC:
				if (index == size - 1) {
					// end of table
					val = 0xffffff;
				}
				else {
					val = (addr - 4) & 0x1fffff;
				}
				break;
			case GPU_DMA:
				val = gpu->getGPURead();
				break;
			default:
				throw_error("DMA copy to this device not implemented yet!");
				break;
			}
			ram->write32Ram(addr, val);
		}
		else {
			u32 val = ram->load32Ram(addr);
			switch (static_cast<DMAChannel>(channel)) {
			case GPU_DMA:
				gpu->pushCmdGP0(val);
				break;
			default:
				throw_error("Not implemented from ram copy block"); 
				break;
			}
		}
		addr += toAdd;
	}
	// In syncMode REQUEST the base address is updated
	if (channelControlRegisters[channel].syncMode == REQUEST) {
		memoryAddressRegisters[channel] = addr - toAdd;
	}
}

void DMA::dmaLinkedList(int channel) {
	if (static_cast<DMAChannel>(channel) != GPU_DMA || channelControlRegisters[channel].direction == TO_RAM) {
		throw_error("Wrong linked list dma!");
	}

	u32 addr = memoryAddressRegisters[channel];
	u32 header = addr;
	do {
		addr = header & 0x1ffffc;
		header = ram->load32Ram(addr);
		u32 size = header >> 24;
		u32 currAddr = addr;
		for (u32 index = 0; index < size; index++) {
			currAddr += 4;
			currAddr &= 0x1ffffc;
			u32 gpu_opcode = ram->load32Ram(currAddr);
			//printf("GPU opcode : 0x%08x\n", gpu_opcode);
			gpu->pushCmdGP0(gpu_opcode);
		}
	} while ((header & (1 << 23)) == 0);
}