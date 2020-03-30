#pragma once

#include <string>

#include "../../definitions.h"

class Disassembler {
public:

	// returns the disassembled instruction
	std::string disassemble(u32 instruction);

	// returns the disassembled BIOS call
	std::string biosCall(u32 addr, u32* reg);

private:
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
	inline u32 imm26(u32 instr) {
		return instr & ((1 << 26) - 1);
	}
	inline u32 func(u32 instr) {
		return instr & 0b111111;
	}
	inline u32 shamt(u32 instr) {
		return (instr >> 6) & 0b11111;
	}

};