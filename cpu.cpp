#include "cpu.h"
#include "gba.h"
#include "clock.h"
#include "thumb_decoder.h"
#include "arm_decoder.h"

#include <cstdint>
#include <iostream>

Clock GBA::clock;

Cpu::Cpu()
{
	Reset();
}

void Cpu::runFor(uint32_t ticks) {
	unsigned long long startingTicks = GBA::clock.getTicks();

	unsigned long long endingTicks = startingTicks + ticks;

	while (GBA::clock.getTicks() < endingTicks) {
		next_instruction();
	}
}

uint32_t Cpu::getPC() {
	return reg.R15;
}

void Cpu::saveBankReg(PrivilegeMode currentMode) {

	switch (currentMode) {
	case SUPERVISOR:
		reg.R8_std = reg.R8;
		reg.R9_std = reg.R9;
		reg.R10_std = reg.R10;
		reg.R11_std = reg.R11;
		reg.R12_std = reg.R12;
		reg.R13_svc = reg.R13;
		reg.R14_svc = reg.R14;
		break;

	case IRQ:
		reg.R8_std = reg.R8;
		reg.R9_std = reg.R9;
		reg.R10_std = reg.R10;
		reg.R11_std = reg.R11;
		reg.R12_std = reg.R12;
		reg.R13_irq = reg.R13;
		reg.R14_irq = reg.R14;
		break;

	case FIQ:
		reg.R8_fiq = reg.R8;
		reg.R9_fiq = reg.R9;
		reg.R10_fiq = reg.R10;
		reg.R11_fiq = reg.R11;
		reg.R12_fiq = reg.R12;
		reg.R13_fiq = reg.R13;
		reg.R14_fiq = reg.R14;
		break;

	case UNDEFINED:
		reg.R8_std = reg.R8;
		reg.R9_std = reg.R9;
		reg.R10_std = reg.R10;
		reg.R11_std = reg.R11;
		reg.R12_std = reg.R12;
		reg.R13_und = reg.R13;
		reg.R14_und = reg.R14;
		break;

	case ABORT:
		reg.R8_std = reg.R8;
		reg.R9_std = reg.R9;
		reg.R10_std = reg.R10;
		reg.R11_std = reg.R11;
		reg.R12_std = reg.R12;
		reg.R13_abt = reg.R13;
		reg.R14_abt = reg.R14;
		break;

	case USER: case SYSTEM:
		reg.R8_std = reg.R8;
		reg.R9_std = reg.R9;
		reg.R10_std = reg.R10;
		reg.R11_std = reg.R11;
		reg.R12_std = reg.R12;
		reg.R13_std = reg.R13;
		reg.R14_std = reg.R14;
		break;
	}
}

void Cpu::getBankReg(PrivilegeMode newMode) {

	switch (newMode) {
	case SUPERVISOR:
		reg.R8 = reg.R8_std;
		reg.R9 = reg.R9_std;
		reg.R10 = reg.R10_std;
		reg.R11 = reg.R11_std;
		reg.R12 = reg.R12_std;
		reg.R13 = reg.R13_svc;
		reg.R14 = reg.R14_svc;
		reg.CPSR_f->mode = newMode;
		break;

	case IRQ:
		reg.R8 = reg.R8_std;
		reg.R9 = reg.R9_std;
		reg.R10 = reg.R10_std;
		reg.R11 = reg.R11_std;
		reg.R12 = reg.R12_std;
		reg.R13 = reg.R13_irq;
		reg.R14 = reg.R14_irq;
		reg.CPSR_f->mode = newMode;
		break;

	case FIQ:
		reg.R8 = reg.R8_fiq;
		reg.R9 = reg.R9_fiq;
		reg.R10 = reg.R10_fiq;
		reg.R11 = reg.R11_fiq;
		reg.R12 = reg.R12_fiq;
		reg.R13 = reg.R13_fiq;
		reg.R14 = reg.R14_fiq;
		reg.CPSR_f->mode = newMode;
		break;

	case UNDEFINED:
		reg.R8 = reg.R8_std;
		reg.R9 = reg.R9_std;
		reg.R10 = reg.R10_std;
		reg.R11 = reg.R11_std;
		reg.R12 = reg.R12_std;
		reg.R13 = reg.R13_und;
		reg.R14 = reg.R14_und;
		reg.CPSR_f->mode = newMode;
		break;

	case ABORT:
		reg.R8 = reg.R8_std;
		reg.R9 = reg.R9_std;
		reg.R10 = reg.R10_std;
		reg.R11 = reg.R11_std;
		reg.R12 = reg.R12_std;
		reg.R13 = reg.R13_abt;
		reg.R14 = reg.R14_abt;
		reg.CPSR_f->mode = newMode;
		break;

	case USER: case SYSTEM:
		reg.R8 = reg.R8_std;
		reg.R9 = reg.R9_std;
		reg.R10 = reg.R10_std;
		reg.R11 = reg.R11_std;
		reg.R12 = reg.R12_std;
		reg.R13 = reg.R13_std;
		reg.R14 = reg.R14_std;
		reg.CPSR_f->mode = newMode;
		break;
	}
}


void Cpu::setPrivilegeMode(PrivilegeMode currentMode, PrivilegeMode mode) {
	saveBankReg(currentMode);
	getBankReg(mode);
}


void Cpu::setPrivilegeMode(PrivilegeMode mode) {
	PrivilegeMode currentPM = (PrivilegeMode)reg.CPSR_f->mode;

	if (currentPM == mode)	//same mode
		return;

	saveBankReg(currentPM);
	getBankReg(mode);

}

void Cpu::Reset() {

	reg = {};
	reg.CPSR_f = (CPSR_registers*)&reg.CPSR;
	getBankReg(SUPERVISOR);

	GBA::clock.clear();
	
}

void Cpu::RaiseIRQ() {

}

void Cpu::RaiseFIQ() {

}

void Cpu::RaiseUndefined() {

}

void Cpu::RaiseSWI() {

}


void Cpu::next_instruction_arm() {

	reg.R15 -= reg.R15 % 4;	//align R15
	uint32_t opcode = GBA::memory.read_32(reg.R15);

	ARM_opcode instruction = ArmDecoder::decode(opcode);
	execute_arm(instruction, opcode);
}

void Cpu::next_instruction_thumb() {

	reg.R15 -= reg.R15 % 2;	//align R15

	uint16_t opcode = GBA::memory.read_16(reg.R15);

	THUMB_opcode instruction = ThumbDecoder::decode(opcode);
	execute_thumb(instruction, opcode);
}

//execute the next instruction
void Cpu::next_instruction() {

	if (reg.R15 == 0x1774) {
		reg.R15 = reg.R15;
	}

	if (reg.CPSR_f->T) {	//thumb
		next_instruction_thumb();
		return;
	}

	next_instruction_arm();
}

bool Cpu::thumbCheckCondition(uint16_t opcode) {
	uint8_t condition = (opcode >> 8) & 0x0f;

	bool condition_met = false;

	switch (condition) {
	case 0:	//EQ (z = 1)
		condition_met = reg.CPSR_f->Z == 1;
		break;
	case 1:		//NE (z = 0)
		condition_met = reg.CPSR_f->Z == 0;
		break;
	case 2:		//CS/HS (c = 1)
		condition_met = reg.CPSR_f->C == 1;
		break;
	case 3:		//CC/LO	(c = 0)
		condition_met = reg.CPSR_f->C == 0;
		break;
	case 4:		//MI (N = 1) negative
		condition_met = reg.CPSR_f->N == 1;
		break;
	case 5:		//PL (N = 0) >= 0
		condition_met = reg.CPSR_f->N == 0;
		break;
	case 6:		//VS (V = 1) overflow
		condition_met = reg.CPSR_f->V == 1;
		break;
	case 7:		//VC (V = 0) no overflow
		condition_met = reg.CPSR_f->V == 0;
		break;
	case 8:		//HI (C = 1 or Z = 0)	unsigned higher
		condition_met = (reg.CPSR_f->C == 1) || (reg.CPSR_f->Z == 0);
		break;
	case 9:		//LS (C = 0 or Z=1)	unsigned lower or same
		condition_met = (reg.CPSR_f->C == 0) || (reg.CPSR_f->Z == 1);
		break;
	case 0xa:	//GE (N = V) signed greater or equal
		condition_met = reg.CPSR_f->N == reg.CPSR_f->V;
		break;
	case 0xb:	//LT (N!=V)	signed less than
		condition_met = reg.CPSR_f->N != reg.CPSR_f->V;
		break;
	case 0xc:	//GT (Z=0 and N=V)	signed greater than
		condition_met = (reg.CPSR_f->Z == 0) && (reg.CPSR_f->N == reg.CPSR_f->V);
		break;
	case 0xd:	//LE (Z=1 or N!=V)	signed less or equal
		condition_met = (reg.CPSR_f->Z == 1) || (reg.CPSR_f->N != reg.CPSR_f->V);
		break;
	}

	return condition_met;
}

void Cpu::execute_thumb(THUMB_opcode instruction, uint16_t opcode) {
	
	switch (instruction) {
	case THUMB_OP_LSL_IMM:	//logic/arithm shift left
		Thumb_LSL_IMM(opcode);
		reg.R15 += 2;
		break;
	
	case THUMB_OP_LSR_IMM:	//logical right shift
		Thumb_LSR_IMM(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ASR_IMM:	//arithmetic shift right
		Thumb_ASR_IMM(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_RR:	//add register + register
		Thumb_ADD_RR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_SUB_RR:	//sub register - register
		Thumb_SUB_RR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_RI:	//add register + immidiate
		Thumb_ADD_RI(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_SUB_RI:	//sub register - immidiate
		Thumb_SUB_RI(opcode);
		reg.R15 += 2;
		break;

		//alu operations
	case THUMB_OP_AND:	//and
		Thumb_AND(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_TST:
		Thumb_TST(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_NEG:	//negate
		Thumb_NEG(opcode);
		reg.R15 += 2;
		break;
		
	case THUMB_OP_MVN:	//mvn
		Thumb_MVN(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ORR:	//or
		Thumb_ORR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_CMP:	//cmp
		Thumb_CMP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_MUL:
		Thumb_MUL(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ROR:
		Thumb_ROR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_EOR:
		Thumb_EOR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_CMP_I:	//compare immidiate
		Thumb_CMP_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_MOV_I:	//move immidiate
		Thumb_MOV_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_I:	//add immidiate
		Thumb_ADD_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_SUB_I:	//sub immidiate
		Thumb_SUB_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDR_PC:	//load pc-relative
		Thumb_LDR_PC(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STR_O:	//store register offset
		Thumb_STR_O(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDR_SP:
		Thumb_LDR_SP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STR_SP:
		Thumb_STR_SP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_MOV_HRR:	//move high registers
		Thumb_MOV_HRR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_CMP_HRR:	//compare high registers
		Thumb_CMP_HRR(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_B:
		Thumb_B(opcode);
		break;

	case THUMB_OP_BX:
		Thumb_BX(opcode);
		break;

	case THUMB_OP_BEQ:	//conditional branches
	case THUMB_OP_BNE:
	case THUMB_OP_BCS:
	case THUMB_OP_BCC:
	case THUMB_OP_BMI:
	case THUMB_OP_BPL:
	case THUMB_OP_BVS:
	case THUMB_OP_BVC:
	case THUMB_OP_BHI:
	case THUMB_OP_BLS:
	case THUMB_OP_BGE:
	case THUMB_OP_BLT:
	case THUMB_OP_BGT:
	case THUMB_OP_BLE:
		if (thumbCheckCondition(opcode)) {
			int8_t offset_8 = (int8_t)(opcode & 0xff);
			int16_t offset = ((int16_t)offset_8 * 2) + 4;
			reg.R15 += offset;
			break;
		}
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_R_SP:
		Thumb_ADD_R_SP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_R_PC:
		Thumb_ADD_R_PC(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_SUB_SP:
		Thumb_SUB_SP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_ADD_SP:
		Thumb_ADD_SP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_PUSH:
		Thumb_PUSH(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_POP:
		Thumb_POP(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDMIA:	//multiple load
		Thumb_LDMIA(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STMIA:	//multiple store
		Thumb_STMIA(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_BL_F:
		Thumb_BL_1(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_BL_LR_IMM:
		Thumb_BL_2(opcode);
		break;

	case THUMB_OP_STR_I:	//store immidate offset
		Thumb_STR_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDR_I:	//load immidiate offset
		Thumb_LDR_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STRB_I:	//store byte immidate offset
		Thumb_STRB_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDRB_I:	//load byte immidiate offset
		Thumb_LDRB_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STRH:	//store halfword
		Thumb_STRH_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDRH:		//load halfword
		Thumb_LDRH_I(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_LDRH_R:	//load halfword register offset
		Thumb_LDRH_R(opcode);
		reg.R15 += 2;
		break;

	case THUMB_OP_STRH_R:	//store halfword register offset
		Thumb_STRH_R(opcode);
		reg.R15 += 2;
		break;

	default:
		std::cout << "!! Thumb instruction not implemented: " << std::hex
			<< "opcode: 0x" << opcode << ", instruction 0x" << instruction << std::endl;
		system("pause");
		exit(1);
		break;
	}

}

//logic/arithm shift left
inline void Cpu::Thumb_LSL_IMM(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t offset = (opcode >> 6) & 0b11111;
	if (offset == 0) {
		*Rd = Rs;
		return;
	}

	*Rd = Rs << offset;
	reg.CPSR_f->C = (Rs >> (32 - offset)) & 1;
	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = *Rd & 0x80000000;
}

//logical right shift
inline void Cpu::Thumb_LSR_IMM(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t offset = (opcode >> 6) & 0b11111;
	if (offset == 0) {
		offset = 32;
	}

	*Rd = Rs >> offset;
	reg.CPSR_f->C = (Rs >> (offset - 1)) & 1;
	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = *Rd & 0x80000000;
}

//arithmetic shift right
inline void Cpu::Thumb_ASR_IMM(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t offset = (opcode >> 6) & 0b11111;
	if (offset == 0) {
		offset = 32;
	}

	if (!(Rs & 0x80000000)) {	//is positive
		*Rd = Rs >> offset;	//just a normal right shift
	}
	else {	//is negative
		uint64_t ones = ((uint64_t)1 << offset) - 1;
		ones <<= (32 - offset);
		*Rd = (Rs >> offset) | (uint32_t)ones;
	}

	reg.CPSR_f->C = (Rs >> (offset - 1)) & 1;
	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = *Rd & 0x80000000;
}

//add register-register
inline void Cpu::Thumb_ADD_RR(uint16_t opcode) {
	uint8_t Rn_reg_code = (opcode >> 6) & 0b111;
	uint32_t Rn = ((uint32_t*)&reg)[Rn_reg_code];	//operand register

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint64_t result = (uint64_t)Rs + (uint64_t)Rn;
	*Rd = (uint32_t)result;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = (result >> 32) & 1;	//carry
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rn ^ Rs)) & (Rs ^ *Rd)) >> 31) & 1;

}

//sub register register
inline void Cpu::Thumb_SUB_RR(uint16_t opcode) {
	uint8_t Rn_reg_code = (opcode >> 6) & 0b111;
	uint32_t Rn = ((uint32_t*)&reg)[Rn_reg_code];	//operand register

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd = Rs - Rn;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(Rs < Rn);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rs ^ (uint32_t)(-(int32_t)Rn))) & (Rs ^ *Rd)) >> 31) & 1;
}

//add register-immidiate
inline void Cpu::Thumb_ADD_RI(uint16_t opcode) {
	uint32_t nn = (opcode >> 6) & 0b111;	//operand immidiate

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint64_t result = (uint64_t)Rs + (uint64_t)nn;
	*Rd = (uint32_t)result;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = (result >> 32) & 1;	//carry
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(nn ^ Rs)) & (Rs ^ *Rd)) >> 31) & 1;
}

//sub register-immidiate
//TODO: for overflow flag maybe we should consider 2nd operand as negative
inline void Cpu::Thumb_SUB_RI(uint16_t opcode) {
	uint32_t nn = (opcode >> 6) & 0b111;	//operand immidiate

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint32_t result = Rs - nn;
	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(Rs < nn);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rs ^ (uint32_t)(-(int32_t)nn))) & (Rs ^ result)) >> 31) & 1;

	*Rd = result;
}

//thumb alu operations 
//MVN
inline void Cpu::Thumb_MVN(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd = ~Rs;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
}

//OR logical
inline void  Cpu::Thumb_ORR(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd |= Rs;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
}

//CMP
inline void Cpu::Thumb_CMP(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];

	uint32_t result = Rd - Rs;
	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(Rd < Rs);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rd ^ (uint32_t)(-(int32_t)Rs))) & (Rd ^ result)) >> 31) & 1;
}

//ROR
inline void Cpu::Thumb_ROR(uint16_t opcode){
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd = shifterRightRotate(*Rd, Rs & 0xff);

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
	reg.CPSR_f->C = shifter_carry_out;
}

//EOR
inline void Cpu::Thumb_EOR(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd ^= Rs;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
}

//multiply
inline void Cpu::Thumb_MUL(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd *= Rs;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
	reg.CPSR_f->C = 0;
}

//and
inline void Cpu::Thumb_AND(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd &= Rs;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;
}

//TST
inline void Cpu::Thumb_TST(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint32_t result = Rd & Rs;

	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;
}

//negate
inline void Cpu::Thumb_NEG(uint16_t opcode) {
	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register
	
	uint32_t result = 0 - Rs;

	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(0 < Rs);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(0 ^ (uint32_t)(-(int32_t)Rs))) & (0 ^ result)) >> 31) & 1;

	*Rd = result;
}

//move immidiate
inline void Cpu::Thumb_MOV_I(uint16_t opcode) {
	uint8_t Rd_reg_code = (opcode >> 8) & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//operand register
	
	uint8_t nn = opcode & 0xff;
	*Rd = nn;

	reg.CPSR_f->Z = nn == 0;
	reg.CPSR_f->N = (nn & 0x80000000) != 0;
}

//sub immidiate.
//TODO: for overflow flag maybe we should consider 2nd operand as negative
inline void Cpu::Thumb_SUB_I(uint16_t opcode) {
	uint8_t Rd_reg_code = (opcode >> 8) & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//operand register

	uint32_t nn = opcode & 0xff;
	uint32_t result = *Rd - nn;

	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(*Rd < nn);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(*Rd ^ (uint32_t)(-(int32_t)nn))) & (*Rd ^ result)) >> 31) & 1;

	*Rd = result;
}

//add immidiate
inline void Cpu::Thumb_ADD_I(uint16_t opcode) {
	uint8_t Rd_reg_code = (opcode >> 8) & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//operand register

	uint32_t nn = opcode & 0xff;
	uint32_t oper1 = *Rd;
	uint64_t result = (uint64_t)oper1 + (uint64_t)nn;
	*Rd = result;

	reg.CPSR_f->Z = *Rd == 0;
	reg.CPSR_f->N = (*Rd & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = (result >> 32) & 1;	//carry
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(oper1 ^ nn)) & (nn ^ *Rd)) >> 31) & 1;
}

//compare immidiate
//TODO: for overflow flag maybe we should consider 2nd operand as negative
inline void Cpu::Thumb_CMP_I(uint16_t opcode) {
	uint8_t Rd_reg_code = (opcode >> 8) & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//operand register

	uint32_t nn = opcode & 0xff;
	uint32_t result = Rd - nn;

	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(Rd < nn);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rd ^ (uint32_t)(-(int32_t)nn))) & (Rd ^ result)) >> 31) & 1;
}

//load pc-relative
inline void Cpu::Thumb_LDR_PC(uint16_t opcode) {
	uint8_t Rd_reg_code = (opcode >> 8) & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint16_t offset = (opcode & 0xff) * 4;
	uint32_t address = ((reg.R15 + 4) & ~2) + offset;

	*Rd = GBA::memory.read_32(address);

}

//store halfword
inline void Cpu::Thumb_STRH_I(uint16_t opcode) {
	uint8_t nn = ((opcode >> 6) & 0b11111) * 2;

	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//source register

	GBA::memory.write_16(Rb + nn, Rd & 0xffff);
}

//load halfword
inline void Cpu::Thumb_LDRH_I(uint16_t opcode) {
	uint8_t nn = ((opcode >> 6) & 0b11111) * 2;

	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	*Rd = GBA::memory.read_16(Rb + nn);
}

//store register offset 
inline void Cpu::Thumb_STR_O(uint16_t opcode) {
	uint8_t Ro_reg_code = (opcode >> 6) & 0b111;
	uint32_t Ro = ((uint32_t*)&reg)[Ro_reg_code];	//offset register

	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//destination register

	GBA::memory.write_32(Rb + Ro, Rd);

}

//load halfword
inline void Cpu::Thumb_LDRH_R(uint16_t opcode) {
	uint8_t Ro_code = (opcode >> 6) & 0b111;
	uint32_t Ro = ((uint32_t*)&reg)[Ro_code];	//offset register

	uint8_t Rb_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_code];	//base address register

	uint8_t Rd_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_code];	//destination register

	*Rd = GBA::memory.read_16(Rb + Ro);
}

//store halfword
inline void Cpu::Thumb_STRH_R(uint16_t opcode) {
	uint8_t Ro_code = (opcode >> 6) & 0b111;
	uint32_t Ro = ((uint32_t*)&reg)[Ro_code];	//offset register

	uint8_t Rb_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_code];	//base address register

	uint8_t Rd_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_code];	//destination register

	GBA::memory.write_16(Rb + Ro, Rd);
}

//store immidate offset
inline void Cpu::Thumb_STR_I(uint16_t opcode) {
	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//source register

	uint8_t offset = ((opcode >> 6) & 0b11111) * 4;

	GBA::memory.write_32(Rb + offset, Rd);
}

//load immidate offset
inline void Cpu::Thumb_LDR_I(uint16_t opcode) {
	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t offset = ((opcode >> 6) & 0b11111) * 4;

	*Rd = GBA::memory.read_32(Rb + offset);
}

//store byte immidate offset
inline void Cpu::Thumb_STRB_I(uint16_t opcode) {
	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//source register

	uint8_t offset = ((opcode >> 6) & 0b11111);

	GBA::memory.write_8(Rb + offset, Rd & 0xff);
}

inline void Cpu::Thumb_LDRB_I(uint16_t opcode) {
	uint8_t Rb_reg_code = (opcode >> 3) & 0b111;
	uint32_t Rb = ((uint32_t*)&reg)[Rb_reg_code];	//base address register

	uint8_t Rd_reg_code = opcode & 0b111;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t offset = ((opcode >> 6) & 0b11111);

	*Rd = GBA::memory.read_8(Rb + offset);
}

//move hi register
inline void Cpu::Thumb_MOV_HRR(uint16_t opcode) {
	//get operand register
	uint8_t Rd_reg_code = opcode & 0b111;
	Rd_reg_code |= (opcode & 0b10000000) >> 4;
	uint32_t *Rd = &((uint32_t*)&reg)[Rd_reg_code];	//destination register

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	Rs_reg_code |= (opcode & 0b1000000) >> 3;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register
	if (Rs_reg_code == 0xf) Rs += 4;

	*Rd = Rs;
}

//compare hi registers
inline void Cpu::Thumb_CMP_HRR(uint16_t opcode) {
	//get operand register
	uint8_t Rd_reg_code = opcode & 0b111;
	Rd_reg_code |= (opcode & 0b10000000) >> 4;
	uint32_t Rd = ((uint32_t*)&reg)[Rd_reg_code];	//destination register
	if (Rd_reg_code == 0xf) Rd += 4;

	uint8_t Rs_reg_code = (opcode >> 3) & 0b111;
	Rs_reg_code |= (opcode & 0b1000000) >> 3;
	uint32_t Rs = ((uint32_t*)&reg)[Rs_reg_code];	//source register
	if (Rs_reg_code == 0xf) Rs += 4;

	uint32_t result = Rd - Rs;

	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(Rd < Rs);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(Rd ^ (uint32_t)(-(int32_t)Rs))) & (Rd ^ result)) >> 31) & 1;
}

//branch exchange
inline void Cpu::Thumb_BX(uint16_t opcode) {
	//get operand register
	uint8_t op_reg_code = (opcode >> 3) & 0b111;
	op_reg_code |= (opcode & 0b1000000) >> 3;
	uint32_t Rs = ((uint32_t*)&reg)[op_reg_code];	//operand register
	if (op_reg_code == 0xf) Rs = (Rs + 4) & ~2;

	uint32_t thumb = Rs & 1;

	reg.R15 = Rs - thumb;
	reg.CPSR = (reg.CPSR & ~(1 << 5)) | (thumb << 5);
}

//push
inline void Cpu::Thumb_PUSH(uint16_t opcode) {

	uint8_t registers_to_push = opcode & 0xff;

	if (opcode & 0x100) {
		reg.R13 -= 4;
		GBA::memory.write_32(reg.R13, reg.R14);
	}

	for (int i = 7; i >= 0; i--) {
		if ((registers_to_push >> i) & 1) {
			reg.R13 -= 4;
			uint32_t* r = &((uint32_t*)&reg)[i];
			GBA::memory.write_32(reg.R13, *r);
		}
	}
}

//pop
inline void Cpu::Thumb_POP(uint16_t opcode) {
	uint8_t registers_to_pop = opcode & 0xff;

	for (int i = 0; i < 8; i++) {
		if ((registers_to_pop >> i) & 1) {
			uint32_t* r = &((uint32_t*)&reg)[i];
			*r = GBA::memory.read_32(reg.R13);
			reg.R13 += 4;
		}
	}

	if (opcode & 0x100) {
		reg.R15 = GBA::memory.read_32(reg.R13) & 0xfffffffe;	//ignore least significant bit
		reg.R15 -= 2;	//to compensate for the R15 increase after the execution
		reg.R13 += 4;
	}
}

//load multiple
inline void Cpu::Thumb_LDMIA(uint16_t opcode) {
	uint8_t rList = opcode & 0xff;

	uint8_t Rb_reg_code = (opcode >> 8) & 0b111;
	uint32_t *Rb = &((uint32_t*)&reg)[Rb_reg_code];	//base address register
	uint32_t addr = *Rb;

	if (rList == 0) {	//empty register list (weird behavior)
		reg.R15 = GBA::memory.read_32(*Rb);
		*Rb += 0x40;
		return;
	}

	for (int i = 0; i < 8; i++) {	//load registers
		if ((rList >> i) & 1) {
			uint32_t* r = &((uint32_t*)&reg)[i];
			*r = GBA::memory.read_32(addr);
			addr += 4;
		}
	}

	//weird behaviors
	if (!((0x1 << Rb_reg_code) & rList)) {	//if base register not inside register list
		*Rb = addr;	//writeback
	}
}

//store multiple
inline void Cpu::Thumb_STMIA(uint16_t opcode) {
	uint8_t rList = opcode & 0xff;

	uint8_t Rb_reg_code = (opcode >> 8) & 0b111;
	uint32_t* Rb = &((uint32_t*)&reg)[Rb_reg_code];	//base address register
	uint32_t addr = *Rb;
	
	//weird behavior
	if (rList == 0) {	//empty register list (weird behavior)
		GBA::memory.write_32(*Rb, reg.R15);
		*Rb += 0x40;
		return;
	}

	for (int i = 0; i < 8; i++) {	//store registers
		if ((rList >> i) & 1) {
			uint32_t r = ((uint32_t*)&reg)[i];
			GBA::memory.write_32(addr, r);
			addr += 4;
		}
	}

	//weird behavior
	//writeback only if base register is not the first entry of the register list
	uint8_t rb_bit = 1 << Rb_reg_code;
	if (!(	//not
		(rb_bit & rList) &&		//base register in list
		!((rb_bit - 1) & rList)		//and first entry
		)) {
		*Rb = addr;	//writeback
	}
}

//unconditional branch
inline void Cpu::Thumb_B(uint16_t opcode) {
	uint32_t uoffset = (opcode & 0x7ff) * 2;

	int16_t offset = uoffset;
	if (uoffset & 0x800) {
		offset |= 0xf000;
	}
	offset += 4;

	reg.R15 += offset;
}

//first instruction of long branch with link: LR = PC + 4 + Imm
inline void Cpu::Thumb_BL_1(uint16_t opcode) {

	uint32_t msb = (opcode & 0x7ff) << 12;
	
	reg.R14 = (reg.R15 + 4 + msb) & 0x7fffff;

	if (reg.R14 & 0x400000) {	//is negative (consider R14 as a 23 bit signed integer)
		reg.R14 |= 0xff800000;
	}
}

//second instruction of long branch with link: PC = LR + Imm
inline void Cpu::Thumb_BL_2(uint16_t opcode) {

	uint32_t lsb = opcode & 0x7ff;
	uint32_t tempR14 = reg.R15 + 2;
	reg.R15 = reg.R14 + (lsb << 1);
	reg.R14 = tempR14 | 0b1;
}

//get relative address from sp
inline void Cpu::Thumb_ADD_R_SP(uint16_t opcode) {
	uint32_t nn = (opcode & 0xff) * 4;

	uint8_t Rd_code = (opcode >> 8) & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_code];	//destination register

	*Rd = reg.R13 + nn;
}

//get relative address from pc
inline void Cpu::Thumb_ADD_R_PC(uint16_t opcode) {
	uint32_t nn = (opcode & 0xff) * 4;

	uint8_t Rd_code = (opcode >> 8) & 0b111;
	uint32_t* Rd = &((uint32_t*)&reg)[Rd_code];	//destination register

	*Rd = ((reg.R15 + 4) & ~2) + nn;
}

//sub offset sp
inline void Cpu::Thumb_SUB_SP(uint16_t opcode) {
	uint32_t nn = opcode & 0x7f;

	reg.R13 -= nn * 4;
}

//ADD SP
inline void Cpu::Thumb_ADD_SP(uint16_t opcode){
	uint32_t nn = opcode & 0x7f;

	reg.R13 += nn * 4;
}

//load sp-relative
inline void Cpu::Thumb_LDR_SP(uint16_t opcode) {
	uint8_t dst_reg_code = (opcode >> 8) & 0b111;
	uint32_t *dst_reg = &((uint32_t*)&reg)[dst_reg_code];	//destination register

	uint32_t offset = opcode & 0xff;
	uint32_t address = reg.R13 + offset * 4;

	*dst_reg = GBA::memory.read_32(address);
}

//store sp-relative
inline void Cpu::Thumb_STR_SP(uint16_t opcode) {
	uint8_t src_reg_code = (opcode >> 8) & 0b111;
	uint32_t src_reg = ((uint32_t*)&reg)[src_reg_code];	//source register

	uint32_t offset = opcode & 0xff;
	uint32_t address = reg.R13 + offset * 4;

	GBA::memory.write_32(address, src_reg);
}

bool Cpu::arm_checkInstructionCondition(uint32_t opcode) {
	uint8_t condition = (opcode >> 28) & 0x0f;

	bool condition_met = false;

	switch (condition) {
	case 0:	//EQ (z = 1)
		condition_met = reg.CPSR_f->Z == 1;
		break;
	case 1:		//NE (z = 0)
		condition_met = reg.CPSR_f->Z == 0;
		break;
	case 2:		//CS/HS (c = 1)
		condition_met = reg.CPSR_f->C == 1;
		break;
	case 3:		//CC/LO	(c = 0)
		condition_met = reg.CPSR_f->C == 0;
		break;
	case 4:		//MI (N = 1) negative
		condition_met = reg.CPSR_f->N == 1;
		break;
	case 5:		//PL (N = 0) >= 0
		condition_met = reg.CPSR_f->N == 0;
		break;
	case 6:		//VS (V = 1) overflow
		condition_met = reg.CPSR_f->V == 1;
		break;
	case 7:		//VC (V = 0) no overflow
		condition_met = reg.CPSR_f->V == 0;
		break;
	case 8:		//HI (C = 1 or Z = 0)	unsigned higher
		condition_met = (reg.CPSR_f->C == 1) || (reg.CPSR_f->Z == 0);
		break;
	case 9:		//LS (C = 0 or Z=1)	unsigned lower or same
		condition_met = (reg.CPSR_f->C == 0) || (reg.CPSR_f->Z == 1);
		break;
	case 0xa:	//GE (N = V) signed greater or equal
		condition_met = reg.CPSR_f->N == reg.CPSR_f->V;
		break;
	case 0xb:	//LT (N!=V)	signed less than
		condition_met = reg.CPSR_f->N != reg.CPSR_f->V;
		break;
	case 0xc:	//GT (Z=0 and N=V)	signed greater than
		condition_met = (reg.CPSR_f->Z == 0) && (reg.CPSR_f->N == reg.CPSR_f->V);
		break;
	case 0xd:	//LE (Z=1 or N!=V)	signed less or equal
		condition_met = (reg.CPSR_f->Z == 1) || (reg.CPSR_f->N != reg.CPSR_f->V);
		break;
	case 0xe:	//AL always
		condition_met = true;
		break;
	case 0xf:	//NV never
		condition_met = false;
		break;
	}

	return condition_met;
}

void Cpu::execute_arm(ARM_opcode instruction, uint32_t opcode) {

	if (!arm_checkInstructionCondition(opcode)) {	//doesn't meet the condition
		reg.R15 += 4;
		//GBA::clock.addTicks(1);
		return;
	}

	switch (instruction) {
	case ARM_OP_B:	//branch
		Arm_B(opcode);
		break;

	case ARM_OP_BL:		//branch with link
		Arm_BL(opcode);
		break;

	case ARM_OP_BX:	//branch and exchange
		Arm_BX(opcode);
		break;

	case ARM_OP_AND:	//and
		Arm_AND(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_EOR:	//xor
		Arm_EOR(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_CMP:		//cmp
		Arm_CMP(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_MOV:		//mov
		Arm_MOV(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_ADD:	//add
		Arm_ADD(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_ADC:	//adc
		Arm_ADC(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_SUB:	//subtract
		Arm_SUB(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_RSB:	//reversed subtract
		Arm_RSB(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_BIC:	//bic
		Arm_BIC(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_TEQ:	//test xor
		Arm_TEQ(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_TST:	//test and
		Arm_TST(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_ORR:	//logical or
		Arm_ORR(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_LDRH:	//load halfword
		Arm_LDRH(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_LDM:		//load data block (pop)
		Arm_LDM(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_STM:	//store data block (push)
		Arm_STM(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_LDR:		//load register
		Arm_LDR(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_STR:	//store register
		Arm_STR(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_MSR:
		Arm_MSR(opcode);
		reg.R15 += 4;
		break;

	default:
		std::cout << "!! Arm instruction not implemented: " << std::hex 
			<< "opcode: 0x" << opcode << ", instruction 0x" << instruction << std::endl;
		system("pause");
		exit(1);
		break;
	}
}


int32_t Cpu::convert_24Bit_to_32Bit_signed(uint32_t val) {

	if (val & 0x800000) {
		return static_cast<int32_t>(val | 0xff000000);
	}
	return static_cast<int32_t>(val & 0xffffff);
}

/*
inline uint32_t Cpu::shifterLeftRotate(uint32_t n, int bits) {
	if (bits == 0) {
		shifter_carry_out = reg.CPSR_f->C;
		return;
	}
	return (n << bits) | (n >> (32 - bits));
}*/

//rotate right with the cpu shifter updating also the shifter carry out
inline uint32_t Cpu::shifterRightRotate(uint32_t n, int bits) {
	if (bits == 0) {
		shifter_carry_out = reg.CPSR_f->C;
		return n;
	}
	shifter_carry_out = (n >> (bits - 1)) & 1;	//carry = last rotated bit
	return (n >> bits) | (n << (32 - bits));
}

//fast rotate right
inline uint32_t Cpu::rightRotate(uint32_t n, int bits) {
	return (n >> bits) | (n << (32 - bits));
}

inline uint32_t Cpu::arithmRight(uint32_t n, int bits) {
	if (!(n & 0x80000000)) {
		return n >> bits;
	}

	uint64_t ones = ((uint64_t)1 << bits) - 1;
	ones <<= (32 - bits);
	return (n >> bits) | (uint32_t)ones;
}


//branch
inline void Cpu::Arm_B(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R15 += 8 + offset * 4;
}

//branch with link
inline void Cpu::Arm_BL(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R14 = reg.R15 + 4;
	reg.R15 += 8 + offset * 4;
}

//branch and exchange
inline void Cpu::Arm_BX(uint32_t opcode) {
	//get operand register
	uint8_t op_reg_code = opcode & 0x0f;
	uint32_t Rn = ((uint32_t*)&reg)[op_reg_code];	//operand register
	if (op_reg_code == 0xf) Rn += 8;

	uint32_t thumb = Rn & 1;

	//note: only bx instruction is defined in ARMv4t
	reg.R15 = Rn - thumb;
	reg.CPSR = (reg.CPSR & ~(1 << 5)) | (thumb << 5);
}


inline void Cpu::ARM_ALU_unpacker(uint32_t opcode, uint32_t** destReg, uint32_t& oper1, uint32_t& oper2, uint8_t &s) {

	uint8_t I = (opcode >> 25) & 1;
	s = (opcode >> 20) & 1;	//set condition code

	//get destination register
	uint8_t dest_reg_code = (opcode >> 12) & 0x0f;
	*destReg = &((uint32_t*)&reg)[dest_reg_code];

	//get operator 1 register
	uint8_t reg_1_code = (opcode >> 16) & 0x0f;
	oper1 = ((uint32_t*)&reg)[reg_1_code];
	if (reg_1_code == 0xf) oper1 += 8;

	if (I) {	//immidiate 2nd operand 
		uint8_t Is = (opcode >> 8) & 0x0f;
		uint32_t nn = opcode & 0xff;
		if (Is != 0) {
			oper2 = shifterRightRotate(nn, Is * 2);
			//shifter_carry_out = (oper2 >> 31) & 1;
		}
		else {
			shifter_carry_out = reg.CPSR_f->C;
			oper2 = nn;
		}
		
		return;
	}
	
	//operand 2 is a register
	ARM_ALU_oper2_getter(opcode, oper2);

}

inline void Cpu::ARM_Shifter(uint8_t shiftType, uint8_t shift_amount, uint32_t val, uint32_t& result, bool enableSpecialShift) {

	shifter_carry_out = reg.CPSR_f->C;
	if (shift_amount) {	//there is a shift
		GBA::clock.addTicks(1);
		switch (shiftType) {
		case 0:		//logical left
			shifter_carry_out = (val >> (32 - shift_amount)) & 1;	//carry = last lost bit
			result = val << shift_amount;
			break;

		case 1:		//logical right
			shifter_carry_out = (val >> (shift_amount - 1)) & 1;	//carry = last lost bit
			result = val >> shift_amount;
			break;

		case 2:		//arithmetic right
			shifter_carry_out = (val >> (shift_amount - 1)) & 1;	//carry = last lost bit
			result = arithmRight(val, shift_amount);
			break;

		case 3:		//rotate right
			//shifter_carry_out = (val >> (shift_amount - 1)) & 1;	//carry = last rotated bit
			result = shifterRightRotate(val, shift_amount);
			break;
		}
		return;
	}

	//special shifts only for immidiate shift amount
	if (!enableSpecialShift) {
		result = val;
		return;
	}

	switch (shiftType) {
	case 0:		//logical left
		shifter_carry_out = reg.CPSR_f->C;
		result = val;
		break;

	case 1:		//logical right
		shifter_carry_out = (val >> 31) & 1;	//carry: 31th bit
		result = 0;
		break;

	case 2:		//arithmetic right
		shifter_carry_out = (val >> 31) & 1;	//carry: 31th bit
		result = (shifter_carry_out == 0 ? 0 : 0xffffffff);
		break;

	case 3:		//rotate right extended
		GBA::clock.addTicks(1);
		uint8_t prev_carry = (val >> 31) & 1;
		shifter_carry_out = val & 1;
		result = val >> 1;
		result |= (prev_carry << 31);
		break;
	}
}

inline void Cpu::ARM_ALU_oper2_getter(uint32_t opcode, uint32_t& oper2) {

	//get operand 2 register
	uint8_t operand_reg_code = opcode & 0x0f;
	uint32_t Rm = ((uint32_t*)&reg)[operand_reg_code];
	uint32_t real_Rm = 0;	//register value corrected in case of R15
	uint8_t Is;	//shift amount

	if (opcode & 0x10) {	//bit 4 set: shift amount taken from a register
		uint8_t shift_reg_code = (opcode >> 8) & 0x0f;
		uint32_t Rs = ((uint32_t*)&reg)[shift_reg_code];
		Is = Rs & 0xff;
		if (operand_reg_code == 0xf) real_Rm = 12;

		real_Rm += Rm;

		uint8_t ST = (opcode >> 5) & 0x3;	//shift type
		ARM_Shifter(ST, Is, real_Rm, oper2, false);
	}
	else { //bit 4 clear: immidiate shift amount 
		Is = (opcode >> 7) & 0x1f;
		if (operand_reg_code == 0xf) real_Rm = 8;

		real_Rm += Rm;

		uint8_t ST = (opcode >> 5) & 0x3;	//shift type
		ARM_Shifter(ST, Is, real_Rm, oper2, true);
	}
}

//arithmetic operation
//TODO: for overflow flag maybe we should consider 2nd operand as negative
inline void Cpu::Arm_CMP(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;
	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);

	uint32_t result = oper1 - oper2;
	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(oper1 < oper2);	//carry = !borrow
	//if oper1 and oper2 have same sign but result have different sign: overflow
	reg.CPSR_f->V = (((~(oper1 ^ (uint32_t)(-(int32_t)oper2))) & (oper1 ^ *dest_reg)) >> 31) & 1;

}

//logical op
//OR: operator1 | operator2
inline void Cpu::Arm_ORR(uint32_t opcode) {
	uint32_t oper1, oper2, * dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);

	uint32_t result = oper1 | oper2;
	*dest_reg = result;

	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = result == 0;
			reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//logical operation
inline void Cpu::Arm_MOV(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	*dest_reg = oper2;
	
	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = oper2 == 0;
			reg.CPSR_f->N = (oper2 & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//tests operand1 XOR operand2
//logical operation
inline void Cpu::Arm_TEQ(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);

	uint32_t result = oper1 ^ oper2;

	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = result == 0;
			reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//operand1 AND operand2
//logical operation
inline void Cpu::Arm_AND(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);

	*dest_reg = oper1 & oper2;

	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = *dest_reg == 0;
			reg.CPSR_f->N = (*dest_reg & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}

}

//operand1 xor operand2
inline void Cpu::Arm_EOR(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);

	*dest_reg = oper1 ^ oper2;

	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = *dest_reg == 0;
			reg.CPSR_f->N = (*dest_reg & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//test. Logical operation
inline void Cpu::Arm_TST(uint32_t opcode) {
	uint32_t oper1, oper2, * dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	uint32_t result = oper1 & oper2;

	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = result == 0;
			reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//arithmetic operation
//operand1 + operand2
inline void Cpu::Arm_ADD(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	uint64_t result = (uint64_t)oper1 + (uint64_t)oper2;
	*dest_reg = oper1 + oper2;

	if (s) {
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = *dest_reg == 0;
			reg.CPSR_f->N = (*dest_reg & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = (result >> 32) & 1;	//carry
			//if oper1 and oper2 have same sign but result have different sign: overflow
			reg.CPSR_f->V = (((~(oper1 ^ oper2)) & (oper1 ^ *dest_reg)) >> 31) & 1;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}

}

//add with carry
inline void Cpu::Arm_ADC(uint32_t opcode) {
	uint32_t oper1, oper2, * dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	uint64_t result = (uint64_t)oper1 + (uint64_t)oper2 + reg.CPSR_f->C;
	*dest_reg = result;

	if (s) {
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = *dest_reg == 0;
			reg.CPSR_f->N = (*dest_reg & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = (result >> 32) & 1;	//carry
			//if oper1 and oper2 have same sign but result have different sign: overflow
			reg.CPSR_f->V = (((~(oper1 ^ oper2)) & (oper1 ^ *dest_reg)) >> 31) & 1;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//arithmetic operation
//operand1 - operand2
inline void Cpu::Arm_SUB(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	uint32_t result = oper1 - oper2;
	*dest_reg = result;

	if (s) {
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = result == 0;
			reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = !(oper1 < oper2);	//carry = !borrow
			//if oper1 and oper2 have same sign but result have different sign: overflow
			reg.CPSR_f->V = (((~(oper1 ^ (uint32_t)(-(int32_t)oper2))) & (oper1 ^ result)) >> 31) & 1;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}	
}

//arithmetic operation
//reversed sub: operand2 - operand1
inline void Cpu::Arm_RSB(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	uint32_t result = oper2 - oper1;
	*dest_reg = result;

	if (s) {
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = result == 0;
			reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = !(oper1 < oper2);	//carry = !borrow
			//if oper1 and oper2 have same sign but result have different sign: overflow
			reg.CPSR_f->V = (((~(oper2 ^ (uint32_t)(-(int32_t)oper1))) & (oper2 ^ result)) >> 31) & 1;
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}

//bit clear. Logical operation
inline void Cpu::Arm_BIC(uint32_t opcode) {
	uint32_t oper1, oper2, * dest_reg;
	uint8_t s;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, s);
	*dest_reg = oper1 & ~oper2;

	if (s) {
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = *dest_reg == 0;
			reg.CPSR_f->N = (*dest_reg & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = shifter_carry_out;	//carry
		}
		else {
			setPrivilegeMode((PrivilegeMode)((CPSR_registers*)&reg.SPSR)->mode);
			reg.CPSR = reg.SPSR;
		}
	}
}


//
inline void Cpu::Arm_MSR(uint32_t opcode) {
	uint32_t Rm = 0;
	uint8_t Pd = (opcode >> 22) & 1;

	uint32_t out_mask = 0;
	if ((opcode >> 19) & 1) out_mask |= 0b1111'1111'0000'0000'0000'0000'0000'0000;	//flag field
	if ((opcode >> 18) & 1) out_mask |= 0b0000'0000'1111'1111'0000'0000'0000'0000;	//status field (reserved. Should not be changed)
	if ((opcode >> 17) & 1) out_mask |= 0b0000'0000'0000'0000'1111'1111'0000'0000;	//extention field (reserved. Should not be changed)
	if ((opcode >> 16) & 1) out_mask |= 0b0000'0000'0000'0000'0000'0000'1111'1111;	//control field


	if ((opcode >> 16) & 1) {	//MSR
		uint8_t reg_code = opcode & 0x0f;
		Rm = ((uint32_t*)&reg)[reg_code];
	}
	else {	//MSR immidiate
		uint8_t I = (opcode >> 25) & 1;
	
		if (I) {
			uint8_t reg_code = opcode & 0x0f;
			Rm = ((uint32_t*)&reg)[reg_code];
		}
		else {
			uint32_t imm = opcode & 0xff;
			uint8_t Is = (opcode >> 8) & 0x0f;
			Rm = rightRotate(imm, Is * 2);
		}
	}

	if (Pd) {	//dest = SPSR
		reg.SPSR = (reg.SPSR & (~out_mask));
		reg.SPSR |= Rm & out_mask;
	}
	else {
		//dest = CPSR
		PrivilegeMode prevMode = (PrivilegeMode)((CPSR_registers*)&reg.CPSR)->mode;
		reg.CPSR = (reg.CPSR & (~out_mask));
		reg.CPSR |= Rm & out_mask;
		setPrivilegeMode(prevMode, (PrivilegeMode)((CPSR_registers*)&reg.CPSR)->mode);
	}
}

inline void Cpu::Arm_MRS(uint32_t opcode) {

	uint8_t reg_code = (opcode >> 12) & 0x0f;
	uint32_t &Rd = ((uint32_t*)&reg)[reg_code];
	uint8_t Pd = (opcode >> 22) & 1;

	if (Pd) {	//dest = SPSR
		Rd = reg.SPSR;
	}
	else {
		//dest = CPSR
		Rd = reg.CPSR;
	}
}

inline void Cpu::ARM_SDTH_unpacker(uint32_t opcode, uint32_t& address, uint32_t** src_dest_reg, 
	uint32_t & R15_as_src_correction) {
	struct param {
		uint8_t L : 1,
			W : 1,	//write back address
			I : 1,	//immidiate
			U : 1,	//add/subtract offset
			P : 1,	//offset before/after transfer
			_ : 3;
	}param = {};

	*((uint8_t*)&param) = (opcode >> 20) & 0x1f;
	R15_as_src_correction = 0;

	//get the base address register
	uint8_t base_reg_code = (opcode >> 16) & 0x0f;
	uint32_t* Rn = &((uint32_t*)&reg)[base_reg_code];
	uint32_t Rn_value = *Rn;
	if (base_reg_code == 0x0f) Rn_value += 8;	//R15 + 8

	//get src/dst register
	uint8_t src_dst_reg_code = (opcode >> 12) & 0x0f;
	uint32_t* Rd = &((uint32_t*)&reg)[src_dst_reg_code];
	*src_dest_reg = Rd;
	if (base_reg_code == 0xf) R15_as_src_correction = 12;

	uint32_t offset = 0;

	if (param.I) {	//immidiate offset
		offset = ((opcode >> 8) & 0b1111) << 4;
		offset |= opcode & 0b1111;
	}
	else {	//offset in register
		uint8_t offset_reg_code = opcode & 0x0f;
		offset = ((uint32_t*)&reg)[offset_reg_code];
	}

	if (param.P) {	//add offset before transfer
		if (param.U) {
			Rn_value += offset;
		}
		else {
			Rn_value -= offset;
		}
		address = Rn_value;
		if (param.W)	//write back
			*Rn = address;
	}
	else {
		address = Rn_value;
		if (param.U) {
			Rn_value += offset;
		}
		else {
			Rn_value -= offset;
		}
		//write back (with post-indexing, write-back is always enabled)
		*Rn = Rn_value;
	}
}

inline void Cpu::Arm_LDRH(uint32_t opcode) {
	uint32_t address, *dest_reg;
	uint32_t notUsed;

	ARM_SDTH_unpacker(opcode, address, &dest_reg, notUsed);

	uint16_t val = GBA::memory.read_16(address);
	*dest_reg = val;
}

inline void Cpu::ARM_SDT_unpacker(uint32_t opcode, uint32_t& address, uint32_t** src_dest_reg, uint8_t& b,
	uint32_t &R15_as_src_correction) {
	struct param {
		uint8_t L : 1,
			W : 1,	//write back address
			B : 1,	//trensfer byte/word
			U : 1,	//add/subtract offset
			P : 1,	//offset before/after transfer
			I : 1,	//immidiate
			_ : 2;
	}param = {};

	*((uint8_t*)&param) = (opcode >> 20) & 0x3f;
	b = param.B;
	R15_as_src_correction = 0;

	//get the base address register
	uint8_t base_reg_code = (opcode >> 16) & 0x0f;
	uint32_t *Rn = &((uint32_t*)&reg)[base_reg_code];
	uint32_t Rn_value = *Rn;
	if (base_reg_code == 0x0f) Rn_value += 8;	//R15 + 8

	//get src/dst register
	uint8_t src_dst_reg_code = (opcode >> 12) & 0x0f;
	uint32_t* Rd = &((uint32_t*)&reg)[src_dst_reg_code];
	*src_dest_reg = Rd;
	if (src_dst_reg_code == 0xf)
		R15_as_src_correction = 12;

	uint32_t offset = 0;

	if (param.I) {	//offset in register
		uint8_t offset_reg_code = opcode & 0x0f;
		uint32_t Rm = ((uint32_t*)&reg)[offset_reg_code];

		uint8_t Is = (opcode >> 7) & 0x1f;
		uint8_t ST = (opcode >> 5) & 0x3;

		ARM_Shifter(ST, Is, Rm, offset, true);
	}
	else {	//offset in immidiate 12 bits
		offset = opcode & 0xfff;
	}
	
	if (param.P) {	//add offset before transfer
		if (param.U) {
			Rn_value += offset;
		}
		else {
			Rn_value -= offset;
		}
		address = Rn_value;
		if(param.W)	//write back
			*Rn = address;
	}
	else {
		address = Rn_value;
		if (param.U) {
			Rn_value += offset;
		}
		else {
			Rn_value -= offset;
		}
		//write back (with post-indexing, write-back is always enabled)
		*Rn = Rn_value;
	}
}

//load data from address to a register
inline void Cpu::Arm_LDR(uint32_t opcode) {
	uint32_t address, *dest_reg;
	uint8_t b;	//load byte
	uint32_t notUsed;

	ARM_SDT_unpacker(opcode, address, &dest_reg, b, notUsed);

	if (b) {	//load byte
		*dest_reg = GBA::memory.read_8(address);
	}
	else {	//load 32bit word
		uint8_t disalignment = address % 4;
		uint32_t val = rightRotate(GBA::memory.read_32(address - disalignment), disalignment * 8);
		*dest_reg = val;
	}

}

//store data from a register to a address
inline void Cpu::Arm_STR(uint32_t opcode) {
	uint32_t address, *src_reg;
	uint8_t b;	//store byte
	uint32_t R15_corr;

	ARM_SDT_unpacker(opcode, address, &src_reg, b, R15_corr);

	if (b) {	//store byte
		GBA::memory.write_8(address, (R15_corr + *src_reg) & 0xff);
	}
	else {	//store 32bit word
		/*uint8_t disalignment = address % 4;
		uint32_t val = rightRotate(GBA::memory.read_32(address - disalignment), disalignment * 8);
		*dest_reg = val;*/
		GBA::memory.write_32(address, R15_corr + *src_reg);
	}
}

uint32_t countSetBits(uint32_t word) {
	int count = 0;
	while (word){
		word = word & (word - 1);    // clear the least significant bit set
		count++;
	}
	return count;
}

uint8_t reverse8(uint8_t b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint16_t reverse16(uint16_t word) {
	return (reverse8(word & 0xff) << 8) |
		(reverse8((word >> 8) & 0xff));
}

inline void Cpu::Arm_STM_DEC(uint8_t paramP, uint16_t reg_list, uint32_t &address) {

	if (paramP) {	//pre: add offset before transfer
		for (int i = 15; i >= 0; i--) {
			if ((reg_list >> i) & 1) {
				address -= 4;
				GBA::memory.write_32(address, ((uint32_t*)&reg)[i]);
			}
		}
	}
	else {
		//post: add offset after transfer
		for (int i = 15; i >= 0; i--) {
			if ((reg_list >> i) & 1) {
				GBA::memory.write_32(address, ((uint32_t*)&reg)[i]);
				address -= 4;
			}
		}
	}
}

inline void Cpu::Arm_STM_INC(uint8_t paramP, uint16_t reg_list, uint32_t& address) {

	if (paramP) {	//pre: add offset before transfer
		for (int i = 0; i < 16; i++) {
			if ((reg_list >> i) & 1) {
				address += 4;
				GBA::memory.write_32(address, ((uint32_t*)&reg)[i]);
			}
		}
	}
	else {
		//post: add offset after transfer
		for (int i = 0; i < 16; i++) {
			if ((reg_list >> i) & 1) {
				GBA::memory.write_32(address, ((uint32_t*)&reg)[i]);
				address += 4;
			}
		}
	}
}

//store block data
inline void Cpu::Arm_STM(uint32_t opcode) {
	struct param {
		uint8_t L : 1,
			W : 1,	//write back address
			S : 1,	//PSR, force user mode
			U : 1,	//add/subtract offset
			P : 1,	//offset before/after transfer
			I : 1,	//immidiate
			_ : 2;
	}param = {};

	*((uint8_t*)&param) = (opcode >> 20) & 0x3f;
	uint16_t reg_list = opcode & 0xffff;

	//set correct register bank
	PrivilegeMode prevMod = (PrivilegeMode)reg.CPSR_f->mode;
	if (param.S) {
		setPrivilegeMode(USER);
	}

	//base register
	uint8_t Rn_code = (opcode >> 16) & 0x0f;
	uint32_t* Rn = &((uint32_t*)&reg)[Rn_code];		
	uint32_t address = *Rn;

	if (param.U) {
		Arm_STM_INC(param.P, reg_list, address);
	}
	else {
		Arm_STM_DEC(param.P, reg_list, address);
	}
	
	if (param.W) {	//write back
		*Rn = address;
	}

	//restore register bank
	setPrivilegeMode(prevMod);
}

inline void Cpu::Arm_LDM_DEC(uint8_t paramP, uint16_t reg_list, uint32_t& address) {

	if (paramP) {	//pre: add offset before transfer
		for (int i = 15; i >= 0; i--) {
			if ((reg_list >> i) & 1) {
				address -= 4;
				((uint32_t*)&reg)[i] = GBA::memory.read_32(address);
			}
		}
	}
	else {
		//post: add offset after transfer
		for (int i = 15; i >= 0; i--) {
			if ((reg_list >> i) & 1) {
				((uint32_t*)&reg)[i] = GBA::memory.read_32(address);
				address -= 4;
			}
		}
	}
}

inline void Cpu::Arm_LDM_INC(uint8_t paramP, uint16_t reg_list, uint32_t& address) {

	if (paramP) {	//pre: add offset before transfer
		for (int i = 0; i < 16; i++) {
			if ((reg_list >> i) & 1) {
				address += 4;
				((uint32_t*)&reg)[i] = GBA::memory.read_32(address);
			}
		}
	}
	else {
		//post: add offset after transfer
		for (int i = 0; i < 16; i++) {
			if ((reg_list >> i) & 1) {
				((uint32_t*)&reg)[i] = GBA::memory.read_32(address);
				address += 4;
			}
		}
	}
}

//load block data
inline void Cpu::Arm_LDM(uint32_t opcode) {
	struct param {
		uint8_t L : 1,
			W : 1,	//write back address
			S : 1,	//PSR, force user mode
			U : 1,	//add/subtract offset
			P : 1,	//offset before/after transfer
			I : 1,	//immidiate
			_ : 2;
	}param = {};

	*((uint8_t*)&param) = (opcode >> 20) & 0x3f;
	uint16_t reg_list = opcode & 0xffff;

	//set correct register bank
	PrivilegeMode prevMod = (PrivilegeMode)reg.CPSR_f->mode;
	if (param.S) {
		if (reg_list & 0x8000) {	//R15 in register list
			reg.CPSR = reg.SPSR;
			prevMod = (PrivilegeMode)reg.CPSR_f->mode;	//to avoid the mode to be wrongly changed at the end of this function
		}
		else {	//user register bank
			setPrivilegeMode(USER);
		}
	}

	//base register
	uint8_t Rn_code = (opcode >> 16) & 0x0f;
	uint32_t* Rn = &((uint32_t*)&reg)[Rn_code];
	uint32_t address = *Rn;

	if (param.U) {
		Arm_LDM_INC(param.P, reg_list, address);
	}
	else {
		Arm_LDM_DEC(param.P, reg_list, address);
	}

	if (param.W) {	//write back
		*Rn = address;
	}

	//restore register bank
	setPrivilegeMode(prevMod);
}