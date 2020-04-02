#include "CPUState.h"

void CPUState::setCop0Reg(u32 index, u32 val) {
	if (index == 12 || val == 0 || index == 8) {
		cop0registers[index] = val;
	}
	else if (index == 13) {
		// cause register
		// mtc0 can only modify the 8th and 9th bits (SW)
		*cause &= ~(0b11 << 8);
		if ((*cause & val & (0b11 << 8)) != 0) {
			throw_error("Unimplemented software interrupt!");
		}
		*cause |= val & (0b11 << 8);
	}
	else {
		throw_error("Cop0 register value unhandled");
	}
}

u32 CPUState::readCop0Reg(u32 index) {
	if (index == 12 || index == 13 || index == 14 || index == 8) {
		return cop0registers[index];
	}
	else if (index == 15) {
		// PRID, no$cash says it should be 2
		return 2;
	}
	else if (index == 6 || index == 7) {
		// index 6:
		// no$cash says it is useless
		// sending back 0 should be enough
		// index 7:
		// breakpoint-related
		// hopefully sending back 0 should be enough
		return 0;
	}
	printf("Cop0 unhandled read at index %d!\n", index);
	throw_error("Unhandled Cop0 Register read");
}

void CPUState::print() {
	printf("PC: 0x%08x \nlo,hi : 0x%08x 0x%08x\n", pc, lo, hi);
	for (int i = 0; i < 32; i++) {
		printf("$r%d : 0x%08x\n", i, registers[i]);
	}
}