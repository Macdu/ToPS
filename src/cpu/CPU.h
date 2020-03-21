#pragma once

#include "../Memory.h"
#include "interpreter/Interpreter.h"
#include "CPUState.h"

class Emulator;

// Emulates a R3000A CPU
class CPU {
public:

	u64 clockCycle;
	void init(Emulator* emu);
	void reset();
	Memory* getMemory();
	// run one cpu instruction
	void step();
	CPUState* getState() { return &state; };
	void requestInterrupt();

private:
	Emulator* emu;
	Memory memory;
	Interpreter interpreter;
	CPUState state;
};