#pragma once

#include "Disassembler.h"

class CPU;
class Memory;
struct CPUState;
enum ExceptionCause;

void setDebugging(bool state);
bool isDebugging();

class Interpreter {
public:
	~Interpreter();
	void init(CPU* cpu);
	void interpret();
	void exception(ExceptionCause cause);
	// called on ps1 putchar
	void ps1_putchar(char val);

private:
	Disassembler disassembler;
	CPU* cpu;
	Memory* memory;
	CPUState* state;
	u32* reg;
	u32 currPC;

	// used for ps1_putchar
	char buffer[1025];
	char bufferSize = 0;

	struct DelayReg {
		u32 regIndex;
		u32 oldVal;
		u32 newVal;
	};

	DelayReg* currDelayReg;
	DelayReg* oldDelayReg;

	void setDelayReg(u32 regIndex, u32 val) {
		currDelayReg->regIndex = regIndex;
		currDelayReg->oldVal = reg[regIndex];
		currDelayReg->newVal = val;
	}

	void updateDelayReg();

	inline u32 opcode(u32 instr) {
		return instr >> 26;
	}
	inline u32 regs(u32 instr) {
		return (instr >> 21) & 0b11111;
	}
	inline u32 regt(u32 instr) {
		return (instr >> 16) & 0b11111;
	}
	inline u32 regd(u32 instr) {
		return (instr >> 11) & 0b11111;
	}
	inline u32 imm(u32 instr) {
		return instr & 0xFFFF;
	}
	inline u32 immsign(u32 instr) {
		return static_cast<u32>(static_cast<i16>(instr & 0xFFFF));
	}
	inline u32 imm26(u32 instr) {
		return instr & ((1 << 26) - 1);
	}
	inline u32 imm11(u32 instr) {
		return instr & ((1 << 11) - 1);
	}
	inline u32 func(u32 instr) {
		return instr & 0b111111;
	}
	inline u32 shamt(u32 instr) {
		return (instr >> 6) & 0b11111;
	}
	inline u32 newPC(u32 instr);

	inline u32 newPCRelative(u32 instr);

	void check_overflow(u32 a, u32 b);
};