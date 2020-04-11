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

		case 0b001101:
			// break code
			sprintf(res, "break code");
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

		case 0b100010:
			// sub $rd, $rs, $rt
			sprintf(res, "sub $r%d, $r%d, $r%d", regd(instr), regs(instr), regt(instr));
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
			sprintf(res, "mtc0 $r%d $cop0_r%d", regt(instr), regd(instr));
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

	case 0b010010: {
		// COP2 instruction
		if ((instr & (1 << 25)) == 0) {
			switch (regs(instr))
			{
			case 0b00000: {
				// mfc2 $rt, $cop2_rd
				sprintf(res, "mfc2 $r%d $cop2_d%d", regt(instr), regd(instr));
				break;
			}

			case 0b00010: {
				// cfc2 $rt, $cop2_rd
				sprintf(res, "cfc2 $r%d $cop2_c%d", regt(instr), regd(instr));
				break;
			}

			case 0b00100: {
				// mtc2 $rt, $cop2_rd
				sprintf(res, "mtc2 $r%d $cop2_d%d", regt(instr), regd(instr));
				break;
			}

			case 0b00110: {
				// ctc2 $rt, $cop2_rd
				sprintf(res, "ctc2 $r%d $cop2_c%d", regt(instr), regd(instr));
				break;
			}
			default:
				break;
			}
		}
		else {
			// gte cmd
			sprintf(res, "gte cmd");
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

	case 0b110010:
		// lwc2 $cop2_datat, imm($rs)
		sprintf(res, "lwc2 $cop2_data%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;

	case 0b111010:
		// swc2 $cop2_datat, imm($rs)
		sprintf(res, "swc2 $cop2_data%d, 0x%04x($r%d)", regt(instr), imm(instr), regs(instr));
		break;
	}

	return std::string(res);
}

std::string Disassembler::biosCall(u32 addr, u32* reg)
{
	char res[200];
	res[0] = 0;
	if (addr == 0xA0) {
		switch (reg[9]) {
		case 0x13:
			sprintf(res, "SaveState(0x%08x)", reg[4]);
			break;
		case 0x15:
			sprintf(res, "strcat(0x%08x,0x%08x)", reg[4], reg[5]);
			break;
		case 0x17:
			sprintf(res, "strcmp(0x%08x,0x%08x)", reg[4], reg[5] );
			break;
		case 0x19:
			sprintf(res, "strcpy(0x%08x,0x%08x)", reg[4], reg[5]);
			break;
		case 0x1B:
			sprintf(res, "strlen(0x%08x)", reg[4]);
			break;
		case 0x2B:
			sprintf(res, "memset(0x%08x,0x%02x,0x%x)", reg[4], reg[5], reg[6]);
			break;
		case 0x25:
			sprintf(res, "toupper(%c)", (char)reg[4]);
			break;
		case 0x28:
			sprintf(res, "bzero(0x%08x,0x%x)", reg[4], reg[5]);
			break;
		case 0x2A:
			sprintf(res, "memcpy(0x%08x,0x%08x,0x%x)", reg[4], reg[5], reg[6]);
			break;
		case 0x2F:
			//sprintf(res, "rand()");
			break;
		case 0x33:
			sprintf(res, "malloc(0x%x)", reg[4]);
			break;
		case 0x34:
			sprintf(res, "free(0x%08x)", reg[4]);
			break;
		case 0x39:
			sprintf(res, "InitHeap(0x%08x,0x%x)", reg[4], reg[5]);
			break;
		case 0x40:
			sprintf(res, "SystemErrorUnresolvedException()");
			throw_error("SystemErrorUnresolvedException");
			break;
		case 0x3F:
			//sprintf(res, "Printf()");
			break;
		case 0x44:
			sprintf(res, "FlushCache()");
			break;
		case 0x49:
			sprintf(res, "GPU_cw(0x%08x)", reg[4]);
			break;
		case 0x70:
			sprintf(res, "_bu_init()");
			break;
		case 0x72:
			sprintf(res, "CdRemove()");
			break;
		case 0x78:
			sprintf(res, "CdAsyncSeekL(0x%08x)", reg[4]);
			break;
		case 0x7C:
			sprintf(res, "CdAsyncGetStatus(dst=0x%08x)", reg[4]);
			break;
		case 0x7E:
			sprintf(res, "CdAsyncReadSector(count=0x%x,dst=0x%08x,mode=0x%x)", reg[4], reg[5], reg[6]);
			break;
		case 0x95:
			sprintf(res, "CdInitSubFunc()");
			break;
		case 0x96:
			sprintf(res, "AddCDROMDevice()");
			break;
		case 0x97:
			sprintf(res, "AddMemCardDevice()");
			break;
		case 0x99:
			sprintf(res, "AddDummyTtyDevice()");
			break;
		case 0xA1:
			sprintf(res, "SystemErrorBootOrDiskFailure(type=0x%x,errorcode=0x%x)", reg[4], reg[5]);
			throw_error("SystemErrorBootOrDiskFailure");
			break;
		case 0xA2:
			sprintf(res, "EnqueueCdIntr()");
			break;
		case 0xA3:
			sprintf(res, "DequeueCdIntr()");
			break;
		case 0xA9:
			sprintf(res, "bu_callback_err_busy()");
			break;
		case 0xAB:
			sprintf(res, "_card_info(0x%x)", reg[4]);
			break;
		case 0xAD:
			sprintf(res, "set_card_auto_format(%d)", reg[4]);
			break;
		default:
			printf("BIOS call : 0x%02x\n", reg[9]);
			throw_error("Undefined A0 bios call!");
		}
	}
	else if (addr == 0xB0) {
		switch (reg[9]) {
		case 0x00:
			sprintf(res, "alloc_kernel_memory(0x%x)", reg[4]);
			break;
		case 0x07:
			//sprintf(res, "DeliverEvent(0x%08x, 0x%x)", reg[4], reg[5]);
			break;
		case 0x08:
			sprintf(res, "OpenEvent(0x%08x,0x%x,0x%x,0x%x)", reg[4], reg[5], reg[6], reg[7]);
			break;
		case 0x09:
			sprintf(res, "CloseEvent(0x%08x)", reg[4]);
			break;
		case 0x0B:
			//sprintf(res, "TestEvent(0x%08x)", reg[4]);
			break;
		case 0x0C:
			sprintf(res, "EnableEvent(0x%08x)", reg[4]);
			break;
		case 0x12:
			sprintf(res, "InitPad(0x%08x,0x%x,0x%08x,0x%x)", reg[4], reg[5], reg[6], reg[7]);
			break;
		case 0x13:
			sprintf(res, "StartPad()");
			break;
		case 0x15:
			sprintf(res, "OutdatedPadInitAndStart(0x%08x,0x%08x,unused,unused)", reg[4], reg[5]);
			break;
		case 0x16:
			sprintf(res, "OutdatedPadGetButtons()");
			break;
		case 0x17:
			//sprintf(res, "ReturnFromException()");
			break;
		case 0x18:
			sprintf(res, "SetDefaultExitFromException()");
			break;
		case 0x19:
			sprintf(res, "SetCustomExitFromException(0x%08x)", reg[4]);
			break;
		case 0x20:
			sprintf(res, "UnDeliverEvent(0x%08x,0x%x)", reg[4], reg[5]);
			break;
		case 0x32:
			sprintf(res, "FileOpen(filename=0x%08x,accessmode=0x%x)", reg[4], reg[5]);
			break;
		case 0x3D:
			//sprintf(res, "std_out_putchar(%c)", (u8)reg[4]);
			break;
		case 0x3F:
			//sprintf(res, "std_out_puts(0x%08x)", reg[4]);
			break;
		case 0x42:
			sprintf(res, "firstfile(0x%08x,0x%08x)", reg[4], reg[5]);
			break;
		case 0x45:
			sprintf(res, "FileDelete(0x%08x)", reg[4]);
			break;
		case 0x47:
			sprintf(res, "AddDevice(0x%08x)", reg[4]);
			break;
		case 0x4A:
			sprintf(res, "InitCard(%d)", reg[4]);
			break;
		case 0x4B:
			sprintf(res, "StartCard()");
			break;
		case 0x4D:
			sprintf(res, "_card_info_subfunc(0x%x)", reg[4]);
			break;
		case 0x4F:
			sprintf(res, "read_card_sector(0x%x,0x%04x,0x%08x)");
			break;
		case 0x50:
			sprintf(res, "allow_new_card()");
			break;
		case 0x56:
			sprintf(res, "GetC0Table()");
			break;
		case 0x57:
			sprintf(res, "GetB0Table()");
			break;
		case 0x58:
			sprintf(res, "get_bu_callback_port()");
			break;
		case 0x5B:
			sprintf(res, "ChangeClearPad(%d)", reg[4]);
			break;
		default:
			printf("BIOS call : 0x%02x\n", reg[9]);
			throw_error("Undefined B0 bios call!");
		}
	}
	else if (addr == 0xC0) {
		switch (reg[9]) {
		case 0x00:
			sprintf(res, "EnqueueTimerAndVblankIrqs(%d)", reg[4]);
			break;
		case 0x01:
			sprintf(res, "EnqueueSyscallHandler(%d)", reg[4]);
			break;
		case 0x02:
			sprintf(res, "SysEnqIntRP(%d,0x%08x)", reg[4], reg[5]);
			break;
		case 0x03:
			sprintf(res, "SysDeqIntRP(%d,0x%08x)", reg[4], reg[5]);
			break;
		case 0x07:
			sprintf(res, "InstallExceptionHandlers()");
			break;
		case 0x08:
			sprintf(res, "SysInitMemory(0x%08x,0x%08x)", reg[4], reg[5]);
			break;
		case 0x0A:
			sprintf(res, "ChangeClearRCnt(%d,%d)", reg[4], reg[5]);
			break;
		case 0x0C:
			sprintf(res, "InitDefInt(%d)", reg[4]);
			break;
		case 0x12:
			sprintf(res, "InstallDevices(0x%08x)", reg[4]);
			break;
		case 0x13:
			sprintf(res, "FlushStdInOutPut()");
			break;
		case 0x1C:
			sprintf(res, "AdjustA0Table()");
			break;
		default:
			printf("BIOS call : 0x%02x\n", reg[9]);
			throw_error("Undefined C0 bios call!");
		}
	}
	else {
		printf("BIOS call : 0x%08x -> 0x%02x\n",addr, reg[9]);
		throw_error("Undefined bios call!");
	}
	return res;
}
