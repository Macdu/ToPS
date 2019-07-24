#pragma once

#include "definitions.h"

class RAM;
class GPU;

enum DMAChannel {
	MDECin = 0,
	MDECout = 1,
	GPU_DMA = 2,
	CDROM = 3,
	SPU = 4,
	PIO = 5,
	GPU_OTC = 6
};

enum DMATransferDirection {
	TO_RAM = 0, FROM_RAM = 1
};

enum DMASyncMode {
	// Start immediately and transfer all at once (used for CDROM, OTC)
	MANUAL = 0,
	// Sync blocks to DMA requests (used for MDEC, SPU, and GPU-data)
	REQUEST = 1,
	// Linked-List mode (used for GPU-command-lists)
	LINKED_LIST = 2
};

// represents the 0x1f8010n8 register
struct ChannelControlRegister {
	// bit 0
	DMATransferDirection direction;
	// bit 1
	bool isGoingBackward;
	// bit 9-10
	DMASyncMode syncMode;
	// bit 24
	bool enabled;
	// bit 28
	bool trigger;

	void loadReg(u32 reg);

	u32 toReg() const;
};

struct DMAInterruptRegister {
	// bit 15, sets bit 31
	bool forceIRQ;

	// bits 16-22, enable IRQ
	u8 irqEnable;

	// bit 23
	bool irqMasterEnable;

	// bit 24-30, set when DMA is finished
	// reset by writing 1 to it
	u8 irqFlag;

	void loadReg(u32 reg);

	u32 toReg() const;
};

/*
	Structure for the DMA: 
	allows for direct transfers of memory
 */
class DMA {
public:
	RAM* ram;
	GPU* gpu;

	// specify is each channel is enabled or disabled
	// not really used right now
	// default value is 0x07654321
	u32 primaryControlRegister;

	// Pointer to the virtual address the DMA will start reading from/writing to
	u32 memoryAddressRegisters[7];
	
	union
	{
		u32 reg;
		struct {
			// For request mode,size of a block (SPU and GPU have a 0x10 word size buffer)
			// For manual mode, number of words to transfer
			u16 lowBlock;
			// For request, amount of blocks to be transferred
			u16 highBlock;
		} content;
	} blockControlRegisters[7];
	
	// interrupt control register 0x1F8010F4
	DMAInterruptRegister interruptControlRegister;
	ChannelControlRegister channelControlRegisters[7];
	
	void reset();
	void setControlRegister(int index, u32 val);
	u32 getControlRegister(int index) const;

private:
	inline u32 getDMASize(int channel);
	// happens during a DMA
	void makeDMA(int channel);
	void dmaCopyBlock(int channel);
	void dmaLinkedList(int channel);
};