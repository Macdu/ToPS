#pragma once

#include "definitions.h"

class CPU;

enum InterruptType {
    iVBlank = 0,
    iGPU = 1,
    iCDROM = 2,
    iDMA = 3,
    iTMR0 = 4,
    iTMR1 = 5,
    iTMR2 = 6,
    iController = 7,
    iSIO = 8,
    iSPU = 9
};

/* Handles PSX Interrupts, Interrupts can be either: (from the NoCash spec)
    IRQ0 VBLANK (PAL=50Hz, NTSC=60Hz)
    IRQ1 GPU Can be requested via GP0(1Fh) command (rarely used)
    IRQ2 CDROM
    IRQ3 DMA
    IRQ4 TMR0 Timer 0 aka Root Counter 0 (Sysclk or Dotclk) 
    IRQ5 TMR1 Timer 1 aka Root Counter 1 (Sysclk or H-blank) 
    IRQ6 TMR2 Timer 2 aka Root Counter 2 (Sysclk or Sysclk/8) 
    IRQ7 Controller and Memory Card - Byte Received Interrupt 
    IRQ8 SIO 
    IRQ9 SPU
    IRQ10 Controller - Lightpen Interrupt (reportedly also PIO...?)
    there are also two software interrupt
 */
class Interrupt {
public:
    // 1F801070h I_STAT - Interrupt status register (R=Status, W=Acknowledge)
    u16 interruptStatus;
    // 1F801074h I_MASK - Interrupt mask register (R / W)
    u16 interruptMask;

    void setInterruptStatus(u16 val);
    void setInterruptMask(u16 val);
    // when a software or hardware interrupt is requested
    void requestInterrupt(InterruptType interrupt);
    // after all hardware IRQ have been checked, call this function
    // to notify the CPU if needed
    void checkIRQ();
    void init(CPU* cpu);

private:
    CPU* cpu;
};