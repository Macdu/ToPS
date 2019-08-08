#include "Interpreter.h"

#include <string>

#include "../../Emulator.h"

bool is_debugging = false;

void setDebugging(bool state) {
	is_debugging = state;
}

bool isDebugging() {
	return is_debugging;
}

void Interpreter::ps1_putchar(char val)
{
	buffer[bufferSize++] = val;
	if (bufferSize == 1024 || val == '\n' || val == 0) {
		buffer[bufferSize - 1] = 0;
		printf("PS out : %s\n", buffer + 1);
		bufferSize = 0;
	}
}

Interpreter::~Interpreter()
{
	delete currDelayReg;
	delete oldDelayReg;
}

void Interpreter::init(CPU * cpu)
{
	this->cpu = cpu;
	this->memory = cpu->getMemory();
	this->state = cpu->getState();
	this->reg = this->state->registers;
	this->oldDelayReg = new DelayReg();
	this->currDelayReg = new DelayReg();
}


void Interpreter::interpret()
{
	//if (state->pc == 0x800513c4)is_debugging = true;
	assert(reg[0] == 0);
	currPC = state->pc;
	//if (currPC == 0x8004fadc) {
	//	printf("Oktt!\n");
	//}
	u32 instr = memory->read32(currPC);
	if(is_debugging)printf("PC : 0x%08x : %s\n",state->pc, disassembler.disassemble(instr).c_str());

	// hack used in order to show printf
	// intercepts std_out_putchar bios calls
	if (currPC == 0x00004070) {
		ps1_putchar(static_cast<char>(reg[4] & 0xFF));
	}

	state->pc = state->nextpc;
	state->nextpc += 4;

	u32 opcode = instr >> 26;

	switch (opcode)
	{
	case 0b000000: {
		// opcode secial
		switch (func(instr)) {
		case 0b000000: {
			// ignore nop case ?
			if (instr == 0) {
				// nop, nothing to do
			}
			else {
				// sll $rd, $rt, shamt
				reg[regd(instr)] = reg[regt(instr)] << shamt(instr);
			}
			break;
		}

		case 0b000010:
			// srl $rd, $rt, shamt
			reg[regd(instr)] = reg[regt(instr)] >> shamt(instr);
			break;

		case 0b000011:
			// sra $rd, $rt, shamt
			reinterpret_cast<i32*>(reg)[regd(instr)] = reinterpret_cast<i32*>(reg)[regt(instr)] >> shamt(instr);
			break;

		case 0b000100:
			// sllv $rd, $rt, $rs
			reg[regd(instr)] = reg[regt(instr)] << (reg[regs(instr)] & 0b11111);
			break;

		case 0b000110:
			// srlv $rd, $rt, $rs
			reg[regd(instr)] = reg[regt(instr)] >> (reg[regs(instr)] & 0b11111);
			break;

		case 0b000111:
			// srav $rd, $rt, $rs
			reinterpret_cast<i32*>(reg)[regd(instr)] = reinterpret_cast<i32*>(reg)[regt(instr)] >> (reg[regs(instr)] & 0b11111);
			break;

		case 0b001000: {
			// jr $rs
			u32 addr = reg[regs(instr)];
			if ((addr & 0b11) != 0) {
				printf("PC alignment error!");
				exception(ExceptionCause::LOAD_ERROR);
				break;
			}
			state->nextpc = addr;
			break;
		}

		case 0b001001:
			// jalr $rd, $rs
			reg[regd(instr)] = state->nextpc;
			if ((reg[regs(instr)] & 0b11) != 0) {
				printf("PC alignment error!");
				exception(ExceptionCause::LOAD_ERROR);
				break;
			}
			state->nextpc = reg[regs(instr)];
			break;

		case 0b001100:
			// syscall
			exception(ExceptionCause::SYSCALL);
			break;

		case 0b010000:
			// mfhi $rd
			reg[regd(instr)] = state->hi;
			break;

		case 0b010001:
			// mthi $rs
			state->hi = reg[regs(instr)];
			break;

		case 0b010010:
			// mflo $rd
			reg[regd(instr)] = state->lo;
			break;

		case 0b010011:
			// mtlo $rs
			state->lo = reg[regs(instr)];
			break;

		case 0b011000: {
			// mult $rs, $rt
			i64 v1 = static_cast<i32>(reg[regs(instr)]);
			i64 v2 = static_cast<i32>(reg[regt(instr)]);
			i64 res = v1 * v2;
			state->hi = static_cast<u32>(res >> 32);
			state->lo = static_cast<u32>(res);
			break;
		}

		case 0b011001: {
			// multu $rs, $rt
			u64 v1 = static_cast<u64>(reg[regs(instr)]);
			u64 v2 = static_cast<u64>(reg[regt(instr)]);
			u64 res = v1 * v2;
			state->hi = static_cast<u32>(res >> 32);
			state->lo = static_cast<u32>(res);
			break;
		}

		case 0b011010: {
			// div $rs, $rt
			i32 denominator = static_cast<i32>(reg[regt(instr)]);
			i32 numerator = static_cast<i32>(reg[regs(instr)]);
			if (denominator == 0) {
				state->hi = static_cast<u32>(numerator);
				if (numerator >= 0) {
					state->lo = -1;
				}
				else {
					state->lo = 1;
				}
			}
			else if (numerator == 1 << 31 && denominator == -1) {
				state->lo = 1 << 31;
				state->hi = 0;
			}
			else {
				state->hi = static_cast<u32>(numerator % denominator);
				state->lo = static_cast<u32>(numerator / denominator);
			}
			break;
		}

		case 0b011011: {
			// divu $rs, $rt
			u32 denominator = reg[regt(instr)];
			u32 numerator = reg[regs(instr)];
			if (denominator == 0) {
				state->hi = numerator;
				state->lo = -1;
			}
			else {
				state->hi = numerator % denominator;
				state->lo = numerator / denominator;
			}
			break;
		}

		case 0b100000:
			// add $rd, $rs, $rt
			check_overflow(reg[regs(instr)], reg[regt(instr)]);
			reg[regd(instr)] = reg[regs(instr)] + reg[regt(instr)];
			break;

		case 0b100001:
			// addu $rd, $rs, $rt
			reg[regd(instr)] = reg[regs(instr)] + reg[regt(instr)];
			break;

		case 0b100011:
			// subu $rd, $rs, $rt
			reg[regd(instr)] = reg[regs(instr)] - reg[regt(instr)];
			break;

		case 0b100100:
			// and $rd, $rs, $rt
			reg[regd(instr)] = reg[regs(instr)] & reg[regt(instr)];
			break;

		case 0b100110:
			// xor $rd, $rs, $rt
			reg[regd(instr)] = reg[regs(instr)] ^ reg[regt(instr)];
			break;

		case 0b100111:
			// nor $rd, $rs, $rt
			reg[regd(instr)] = ~(reg[regs(instr)] | reg[regt(instr)]);
			break;

		case 0b101010:
			// slt $rd, $rs, $rt
			reg[regd(instr)] = static_cast<u32>(reinterpret_cast<i32*>(reg)[regs(instr)] < reinterpret_cast<i32*>(reg)[regt(instr)]);
			break;

		case 0b101011:
			// sltu $rd, $rs, $rt
			reg[regd(instr)] = static_cast<u32>(reg[regs(instr)] < reg[regt(instr)]);
			break;

		case 0b100101:
			// or $rd, $rs, $rt
			reg[regd(instr)] = reg[regs(instr)] | reg[regt(instr)];
			break;

		default:
			fprintf(stderr, "0x%08x is not defined (opcode 0x%02x, func 0x%02x)!\n", instr, opcode, func(instr));
			state->print();
			throw_error("Undefined instruction");
		}
		break;
	}


	case 0b000001: {
		switch (regt(instr)) {
		case 0b00000:
			// bltz $rs, imm
			if (reinterpret_cast<i32*>(reg)[regs(instr)] < 0) {
				state->nextpc = newPCRelative(instr);
			}
			break;
		case 0b00001:
			// bgez $rs, imm
			if (reinterpret_cast<i32*>(reg)[regs(instr)] >= 0) {
				state->nextpc = newPCRelative(instr);
			}
			break;
		case 0b10000:
			// bltzal $rs, imm
			reg[31] = state->nextpc;
			if (reinterpret_cast<i32*>(reg)[regs(instr)] < 0) {
				state->nextpc = newPCRelative(instr);
			}
			break;
		case 0b10001:
			// bgezal $rs, imm
			reg[31] = state->nextpc;
			if (reinterpret_cast<i32*>(reg)[regs(instr)] >= 0) {
				state->nextpc = newPCRelative(instr);
			}
			break;
		default:
			state->print();
			assert(false);
		}
		break;
	}

	case 0b000010:
		// j imm
		state->nextpc = newPC(instr);
		break;

	case 0b000011:
		// jal imm
		reg[31] = state->nextpc;
		state->nextpc = newPC(instr);
		break;

	case 0b000100:
		// beq $rs, $rt, imm
		if (reg[regs(instr)] == reg[regt(instr)]) {
			state->nextpc = newPCRelative(instr);
		}
		break;

	case 0b000101:
		// bne $rs, $rt, imm
		if (reg[regs(instr)] != reg[regt(instr)]) {
			state->nextpc = newPCRelative(instr);
		}
		break;

	case 0b000110:
		// blez $rs, imm
		if (static_cast<i32>(reg[regs(instr)]) <= 0) {
			state->nextpc = newPCRelative(instr);
		}
		break;

	case 0b000111:
		// bgtz $rs, imm
		if (static_cast<i32>(reg[regs(instr)]) > 0) {
			state->nextpc = newPCRelative(instr);
		}
		break;

	case 0b001000:
		// addi $rt, $rs, imm
		check_overflow(reg[regs(instr)], immsign(instr));
		reg[regt(instr)] = reg[regs(instr)] + immsign(instr);
		break;

	case 0b001001:
		// addiu $rt, $rs, imm
		reg[regt(instr)] = reg[regs(instr)] + immsign(instr);
		break;

	case 0b001010:
		// slti $rt, $rs, imm
		reg[regt(instr)] = static_cast<u32>(reinterpret_cast<i32*>(reg)[regs(instr)] < static_cast<i32>(immsign(instr)));
		break;

	case 0b001011:
		// sltiu $rt, $rs, imm
		reg[regt(instr)] = static_cast<u32>(reg[regs(instr)] < immsign(instr));
		break;

	case 0b001100:
		// andi $rt, $rs, imm
		reg[regt(instr)] = reg[regs(instr)] & imm(instr);
		break;

	case 0b001101:
		// ori $rt, $rs, imm
		reg[regt(instr)] = imm(instr) | reg[regs(instr)];
		break;

	case 0b001110:
		// xori $rt, $rs, imm
		reg[regt(instr)] = imm(instr) ^ reg[regs(instr)];
		break;

	case 0b001111:
		// lui $rt, imm
		reg[regt(instr)] = imm(instr) << 16;
		break;

	case 0b010000: {
		// coprocessor instruction
		switch (regs(instr))
		{
		case 0b00000: 
			// mfc0 $rt, $cpo0_rd
			setDelayReg(regt(instr), state->readCop0Reg(regd(instr)));
			break;

		case 0b00100: {
			// mtc0 $rt, $rd
			assert(imm11(instr) == 0);
			state->setCop0Reg(regd(instr), reg[regt(instr)]);
			break;

		case 0b10000: {
			// rfe
			assert((instr & 0x3f) == 0b010000);
			u32 modes_interrupts = *state->sr & 0x3f;
			*state->sr &= ~0x3f;
			*state->sr |= modes_interrupts >> 2;
			break;
		}

		default:
			throw_error("Undefined coprocessor opcode!");
		}
		}
		break;
	}

	case 0b100000:
		// lb $rt, imm($rs)
		setDelayReg(regt(instr), static_cast<u32>(static_cast<i8>(memory->read8(reg[regs(instr)] + immsign(instr)))));
		break;

	case 0b100001: {
		// lh $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		if ((addr & 1) != 0) {
			printf("Load alignment16 error!");
			exception(ExceptionCause::LOAD_ERROR);
			break;
		}
		setDelayReg(regt(instr), static_cast<u32>(static_cast<i16>(memory->read16(addr))));
		break;
	}

	case 0b100010: {
		// lwl $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		u32 val = (currDelayReg->regIndex == regt(instr)) ? currDelayReg->newVal : 0;
		u32 aligned_val = memory->read32(addr & ~3);

		switch (addr & 3)
		{
		case 0:
			val = (val & 0x00ffffff) | (aligned_val << 24);
			break;
		case 1:
			val = (val & 0x0000ffff) | (aligned_val << 16);
			break;
		case 2:
			val = (val & 0x000000ff) | (aligned_val << 8);
			break;
		case 3:
			val = aligned_val;
			break;
		}
		setDelayReg(regt(instr), val);
		break;
	}

	case 0b100011: {
		// lw $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		if ((addr & 0b11) != 0) {
			printf("Load alignment error!");
			exception(ExceptionCause::LOAD_ERROR);
			break;
		}
		setDelayReg(regt(instr), memory->read32(addr));
		break;
	}

	case 0b100100:
		// lb $rt, imm($rs)
		setDelayReg(regt(instr), static_cast<u32>(memory->read8(reg[regs(instr)] + immsign(instr))));
		break;

	case 0b100101: {
		// lhu $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		if ((addr & 1) != 0) {
			printf("Load alignment error!");
			exception(ExceptionCause::LOAD_ERROR);
		}
		setDelayReg(regt(instr), static_cast<u32>(memory->read16(addr)));
		break;
	}

	case 0b100110: {
		// lwr $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		u32 val = (currDelayReg->regIndex == regt(instr)) ? currDelayReg->newVal : 0;
		u32 aligned_val = memory->read32(addr & ~3);

		switch (addr & 3)
		{
		case 0:
			val = aligned_val;
			break;
		case 1:
			val = (val & 0xff000000) | (aligned_val >> 8);
			break;
		case 2:
			val = (val & 0xffff0000) | (aligned_val >> 16);
			break;
		case 3:
			val = (val & 0xffffff00) | (aligned_val >> 24);
			break;
		}
		setDelayReg(regt(instr), val);
		break;
	}

	case 0b101000:
		// sb $rt, imm($rs)
		memory->write8(reg[regs(instr)] + immsign(instr), static_cast<u8>(reg[regt(instr)]));
		break;

	case 0b101001: {
		// sh $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		if ((addr & 1) != 0) {
			printf("Store alignment error!");
			exception(ExceptionCause::STORE_ERROR);
			break;
		}
		memory->write16(addr, static_cast<u16>(reg[regt(instr)]));
		break;
	}

	case 0b101010: {
		// swl $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		u32 val = memory->read32(addr & ~3);
		switch (addr & 3) {
		case 0:
			val = (val & 0xffffff00) | (reg[regt(instr)] >> 24);
			break;
		case 1:
			val = (val & 0xffff0000) | (reg[regt(instr)] >> 16);
			break;
		case 2:
			val = (val & 0xff000000) | (reg[regt(instr)] >> 8);
			break;
		case 3:
			val = reg[regt(instr)];
			break;
		}
		memory->write32(addr & ~3, val);
		break;
	}

	case 0b101011: {
		// sw $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		if ((addr & 0b11) != 0) {
			printf("Store alignment error!");
			exception(ExceptionCause::STORE_ERROR);
			break;
		}
		memory->write32(addr, reg[regt(instr)]);
		break;
	}

	case 0b101110: {
		// swl $rt, imm($rs)
		u32 addr = reg[regs(instr)] + immsign(instr);
		u32 val = memory->read32(addr & ~3);
		switch (addr & 3) {
		case 0:
			val = reg[regt(instr)];
			break;
		case 1:
			val = (val & 0x000000ff) | (reg[regt(instr)] << 8);
			break;
		case 2:
			val = (val & 0x0000ffff) | (reg[regt(instr)] << 16);
			break;
		case 3:
			val = (val & 0x00ffffff) | (reg[regt(instr)] << 24);
			break;
		}
		memory->write32(addr & ~3, val);
		break;
	}

	default:
		fprintf(stderr, "0x%08x is not defined (opcode 0x%02x)!\n", instr, opcode);
		state->print();
		throw_error("Undefined instruction");
		break;
	}

	updateDelayReg();
}

void Interpreter::exception(ExceptionCause cause)
{
	*state->epc = currPC;

	// If in branch delay
	if (state->pc + 4 != state->nextpc) {
		*state->epc -= 4;
		*state->epc |= 1 << 31;
	}

	// BEV bit
	if ((*state->sr & (1 << 22)) != 0) {
		state->pc = 0xbfc00180;
	}
	else {
		state->pc = 0x80000080;
	}
	state->nextpc = state->pc + 4;

	// shifting of kernel user and interrupt enable bits
	u32 modes_interrupts = *state->sr & 0xf;
	modes_interrupts &= ~0x3f;
	modes_interrupts |= modes_interrupts << 2;

	*state->cause = static_cast<u32>(cause) << 2;
}

void Interpreter::updateDelayReg()
{
	if (oldDelayReg->oldVal == reg[oldDelayReg->regIndex]) {
		reg[oldDelayReg->regIndex] = oldDelayReg->newVal;
	}
	// swap the delayRegs
	DelayReg* temp;
	temp = currDelayReg;
	currDelayReg = oldDelayReg;
	oldDelayReg = temp;
	currDelayReg->regIndex = 0;
	reg[0] = 0;
}

inline u32 Interpreter::newPC(u32 instr)
{
	return (state->nextpc & (0b1111 << 28)) | (imm26(instr) << 2);
}

inline u32 Interpreter::newPCRelative(u32 instr)
{
	return state->pc + (immsign(instr) << 2);
}

void Interpreter::check_overflow(u32 a, u32 b) {
	u32 res = a + b;
	if (!((a ^ b) & 0x80000000) && ((a ^ res) & 0x80000000)) {
		printf("Arithmetic overflow!\n");
		exception(ExceptionCause::ARITHMETIC_OVERFLOW);
	}
}
