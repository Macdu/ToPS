#pragma once

#include "../definitions.h"

enum ExceptionCause {
	INTERRUPT = 0,
	LOAD_ERROR = 4,
	STORE_ERROR = 5,
	SYSCALL = 8,
	BREAKPOINT = 9,
	ARITHMETIC_OVERFLOW = 12
};

struct CPUState {
	u32 registers[32];

	u32 hi;
	u32 lo;

	u32 pc;
	u32 nextpc;

	u32 cop0registers[16];
	u32* sr = &cop0registers[12];
	u32* cause = &cop0registers[13];
	u32* epc = &cop0registers[14];

	void setCop0Reg(u32 index, u32 val);

	u32 readCop0Reg(u32 index);

	void print();
};