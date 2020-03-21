#include "CPU.h"

#include "../Emulator.h"

void CPU::init(Emulator * emu)
{
	clockCycle = 0;
	this->emu = emu;
	interpreter.init(this);
	memory.init(emu);
}

void CPU::reset()
{
	state.pc = 0xBFC00000;
	state.nextpc = state.pc + 4;
}

Memory * CPU::getMemory()
{
	return &memory;
}

void CPU::step()
{
	clockCycle++;
	interpreter.interpret();
}

void CPU::requestInterrupt()
{
	interpreter.exception(ExceptionCause::INTERRUPT);
}
