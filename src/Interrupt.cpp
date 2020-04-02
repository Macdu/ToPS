#include "Interrupt.h"

#include "cpu/CPU.h"

void Interrupt::setInterruptStatus(u16 val)
{
	// 0=Clear Bit, 1=No change
	interruptStatus &= val;
	// update cop0 cause
	if ((interruptMask & interruptStatus) == 0) {
		*cpu->getState()->cause &= ~(1 << 10);
	}
}

void Interrupt::setInterruptMask(u16 val)
{
	// we are only interested in the first 11 bits
	interruptMask = val & ((1 << 11) - 1);
	// update cop0 cause
	if ((interruptMask & interruptStatus) != 0) {
		*cpu->getState()->cause |= 1 << 10;
	}
}

void Interrupt::requestInterrupt(InterruptType interrupt)
{
	u32 interruptBit = 1 << (u32)interrupt;
	interruptStatus |= interruptBit;
	// update cause register if needed
	if ((interruptMask & interruptStatus) != 0) {
		*cpu->getState()->cause |= 1 << 10;

		if (Debugging::interrupt && (u8)interrupt != 0) {
			constexpr u32 canInterruptBit = (1 << 10) + 1;
			bool willInterrupt = (*cpu->getState()->sr & canInterruptBit) == canInterruptBit;
			if(willInterrupt)printf("Interrupt : %d\n", (u8)interrupt);
		}
	}
}

void Interrupt::checkIRQ()
{
	// if a hardware interrupt has not been acknowledged
	if ((interruptMask & interruptStatus) != 0) {
		constexpr u32 canInterruptBit = (1 << 10) + 1;
		// check if hardware interrupt are enabled and interrupts are enabled
		if ((*cpu->getState()->sr & canInterruptBit) == canInterruptBit) {
			// now we can request an interrupt
			cpu->requestInterrupt();
		}
	}
}

void Interrupt::init(CPU* cpu)
{
	interruptMask = 0;
	interruptStatus = 0;
	this->cpu = cpu;
}