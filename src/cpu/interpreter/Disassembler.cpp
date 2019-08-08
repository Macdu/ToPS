#include "Disassembler.h"

std::string Disassembler::disassemble(u32 instr)
{
	char res[100];
	res[0] = 0;
	u32 opcode = instr >> 26;
	switch (opcode) {
	case 0b000000: {
		// opcode secial
		switch (func(instr)) {
		case 0b000000: {
			if (instr == 0) {
				// nop
				sprintf(res, "nop");
			}
			else {
				// sll $rd, $rt, shamt
				sprintf(res, "sll $r%d, $r%d, %d", regd(instr), regt(instr), shamt(instr));
			}
			break;
		}

		case 0b000010:
			// srl $rd, $rt, shamt
			sprintf(res, "srl $r%d, $r%d, %d", regd(instr), regt(instr), shamt(instr));
			break;

		case 0b000011:
			// sra $rd, $rt, shamt
			sprintf(res, "sra $r%d, $r%d, %d", regd(instr), regt(instr), shamt(instr));
			break;

		case 0b000100:
			// sllv $rd, $rt, $rs
			sprintf(res, "sllv r%d, $r%d, r%d", regd(instr), regt(instr), regs(instr));
			break;

		case 0b000110:
			// srlv $rd, $rt, $rs
			sprintf(res, "srlv r%d, $r%d, r%d", regd(instr), regt(instr), regs(instr));
			break;

		case 0b000111:
			// srav $rd, $rt, $rs
			sprintf(res, "srav r%d, $r%d, r%d", regd(instr), regt(instr), regs(instr));
			break;

		case 0b001000:
			// jr $rs
			sprintf(res, "jr $r%d", regs(instr));
			break;

		case 0b001001:
			// jalr $rd, $rs
			sprintf(res, "jalr $r%d, $r%d", regd(instr), regs(instr));
			break;

		case 0b001100:
			// syscall
			sprintf(res, "syscall");
			break;

		case 0b010000:
			// mfhi $rd
			sprintf(res, "mfhi $r%d", regd(instr));
			break;

		case 0b010001:
			// mthi $rs
			sprintf(res, "mthi $r%d", regs(instr));
			break;

		case 0b010010:
			// mflo $rd
			sprintf(res, "mflo $r%d", regd(instr));
			break;

		case 0b010011:
			// mtlo $rs
			sprintf(res, "mtlo $r%d", regs(instr));
			break;

		case 0b011000:
			// mult $rs, $rt
			sprintf(res, "mult $r%d, $r%d", regs(instr), regt(instr));
			break;

		case 0b011001:
			// multu $rs, $rt
			sprintf(res, "multu $r%d, $r%d", regs(instr), regt(instr));
			break;

		case 0b011010:
			// div $rs, $rt
			sprintf(res, "div $r%d, $r%d", regs(instr), regt(instr));
			break;

		case 0b011011:
			// divu $rs, $rt
			sprintf(res, "divu $r%d, $r%d", regs(instr), regt(instr));
			break;

		case 0b100000:
			// add $rd, $rs, $rt
			sprintf(res, "add $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100001:
			// addu $rd, $rs, $rt
			sprintf(res, "addu $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100011:
			// subu $rd, $rs, $rt
			sprintf(res, "subu $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100100:
			// and $rd, $rs, $rt
			sprintf(res, "and $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100110:
			// xor $rd, $rs, $rt
			sprintf(res, "xor $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100111:
			// nor $rd, $rs, $rt
			sprintf(res, "nor $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b101010:
			// slt $rd, $rs, $rt
			sprintf(res, "slt $r%d $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b101011:
			// sltu $rd, $rs, $rt
			sprintf(res, "sltu $r%d $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;

		case 0b100101:
			// or $rd, $rs, $rt
			sprintf(res, "or $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
			break;
		}
		break;
	}

	case 0b000001: {
		switch (regt(instr)) {
		case 0b00000:
			// bltz $rs, imm
			sprintf(res, "bltz $r%d, 0x%04x", regs(instr), imm(instr));
			break;
		case 0b00001: 
			// bgez $rs, imm
			sprintf(res, "bgez $r%d, 0x%04x", regs(instr), imm(instr));
			break;
		case 0b10000:
			// bltzal $rs, imm
			sprintf(res, "bltzal $r%d, 0x%04x", regs(instr), imm(instr));
			break;
		case 0b10001:
			// bgezal $rs, imm
			sprintf(res, "bgezal $r%d, 0x%04x", regs(instr), imm(instr));
			break;
		}
		break;
	}

	case 0b000010:
		// j imm
		sprintf(res, "j 0x%07x", imm26(instr) << 2);
		break;

	case 0b000011:
		// jal imm
		sprintf(res, "jal 0x%07x", imm26(instr) << 2);
		break;

	case 0b000100:
		// beq $rs, $rt, imm
		sprintf(res, "beq $r%d, $r%d, 0x%04x", regs(instr), regt(instr), imm(instr));
		break;

	case 0b000101:
		// bne $rs, $rt, imm
		sprintf(res, "bne $r%d, $r%d, 0x%04x",regs(instr),regt(instr), imm(instr));
		break;

	case 0b000110:
		// blez $rs, imm
		sprintf(res, "blez $r%d, 0x%04x", regs(instr), imm(instr));
		break;

	case 0b000111:
		// bgtz $rs, imm
		sprintf(res, "bgtz $r%d, 0x%04x", regs(instr), imm(instr));
		break;

	case 0b001000:
		// addi $rt, $rs, imm
		sprintf(res, "addi $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001001:
		// addiu $rt, $rs, imm
		sprintf(res, "addiu $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001010:
		// slti $rt, $rs, imm
		sprintf(res, "slti $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001011:
		// sltiu $rt, $rs, imm
		sprintf(res, "slti $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001100:
		// andi $rt, $rs, imm
		sprintf(res, "andi $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001101:
		// ori $rt, $rs, imm
		sprintf(res, "ori $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001110:
		// xori $rt, $rs, imm
		sprintf(res, "xori $r%d, $r%d, 0x%04x", regt(instr), regs(instr), imm(instr));
		break;

	case 0b001111:
		// lui $rt, imm
		sprintf(res, "lui $r%d 0x%04x", regt(instr), imm(instr));
		break;

	case 0b010000: {
		// coprocessor instruction
		switch (regs(instr))
		{
		case 0b00000: {
			// mfc0 $rt, $cop0_rd
			sprintf(res, "mfc0 $r%d $cop0_r%d", regt(instr), regd(instr));
			break;
		}

		case 0b00100: {
			// mtc0 $rt, $cop0_rd
			sprintf(res, "mtco $r%d $cop0_r%d", regt(instr), regd(instr));
			break;

		case 0b10000:
			// rfe
			sprintf(res, "rfe");
			break;
		}
		default:
			break;
		}
		break;
	}

	case 0b100000:
		// lb $rt, imm($rs)
		sprintf(res, "lb $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100001:
		// lh $rt, imm($rs)
		sprintf(res, "lh $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100010:
		// lwl $rt, imm($rs)
		sprintf(res, "lwl $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100011:
		// lw $rt, imm($rs)
		sprintf(res, "lw $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100100:
		// lb $rt, imm($rs)
		sprintf(res, "lbu $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100101:
		// lhu $rt, imm($rs)
		sprintf(res, "lhu $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b100110:
		// lwr $rt, imm($rs)
		sprintf(res, "lwr $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b101000:
		// sb $rt, imm($rs)
		sprintf(res, "sb $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b101001:
		// sh $rt, imm($rs)
		sprintf(res, "sh $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b101010:
		// swl $rt, imm($rs)
		sprintf(res, "swl $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b101011:
		// sw $rt, imm($rs)
		sprintf(res, "sw $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b101110:
		// swr $rt, imm($rs)
		sprintf(res, "swr $r%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;
	}

	return std::string(res);
}
