#include "cpu.h"
#include "gba.h"
#include "clock.h"

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

	uint32_t opcode = GBA::memory.read_32(reg.R15);

	ARM_opcode instruction = decode_arm(opcode);
	execute_arm(instruction, opcode);
}

void Cpu::next_instruction_thumb() {

}

//execute the next instruction
void Cpu::next_instruction() {

	if (reg.CPSR_f->T) {	//thumb
		next_instruction_thumb();
		return;
	}

	next_instruction_arm();
}

ARM_opcode Cpu::decode_arm(uint32_t opcode) {

	ARM_opcode instr;

	if (instr = ARM_IsBranch(opcode)) return instr;		//branches
	if (instr = ARM_IsAluInst(opcode)) return instr;	//data processing
	if (instr = ARM_IsSDTInst(opcode)) return instr;	//single store/load

	return ARM_opcode::ARM_OP_INVALID;
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

	case ARM_OP_CMP:		//cmp
		Arm_CMP(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_MOV:		//mov
		Arm_MOV(opcode);
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

	case ARM_OP_TEQ:	//test xor
		Arm_TEQ(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_MSR:
		Arm_MSR(opcode);
		reg.R15 += 4;
		break;

	default:
		std::cout << "!! Instruction not implemented: " << std::hex 
			<< "op " << opcode << ", instruction " << instruction << std::endl;
		system("pause");
		exit(1);
		break;
	}
}

/*void Cpu::execute_thumb() {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	//align R15
	reg.R15 -= reg.R15 % 2;

	uint16_t instruction = GBA::memory.read_16(reg.R15);

	reg.R15 += 2;
}*/

int32_t convert_24Bit_to_32Bit_signed(uint32_t val) {

	if (val & 0x800000) {
		return static_cast<int32_t>(val | 0xff000000);
	}
	return static_cast<int32_t>(val & 0xffffff);
}

ARM_opcode Cpu::ARM_IsBranch(uint32_t opcode) {
	
	uint32_t branch_format = 0b0000'1010'0000'0000'0000'0000'0000'0000;
	uint32_t branch_with_link_format = 0b0000'1011'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1111'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	//branch
	if (branch_format == opcode_format)
		return ARM_OP_B;

	//branch with link
	if (branch_with_link_format == opcode_format) {
		return ARM_OP_BL;
	}

	return ARM_OP_INVALID;
}

ARM_opcode Cpu::ARM_IsAluInst(uint32_t opcode) {
	uint32_t mask =		0b0000'1100'0000'0000'0000'0000'0000'0000;
	uint32_t format =	0b0000'0000'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if(format != opcode_format)	//not alu instruction
		return ARM_OP_INVALID;

	switch ((opcode >> 21) & 0x0f) {
	case 0:	//and
		return ARM_OP_AND;
		break;
	case 1:	//xor
		return ARM_OP_EOR;
		break;
	case 2:	//subtract
		return ARM_OP_SUB;
		break;
	case 3:	//subtract reversed
		return ARM_OP_RSB;
		break;
	case 4:	//add
		return ARM_OP_ADD;
		break;
	case 5:	//add with carry
		return ARM_OP_ADC;
		break;
	case 6:	//subtract with carry
		return ARM_OP_SBC;
		break;
	case 7:	//sub cy. reversed
		return ARM_OP_RSC;
		break;
	case 8:	//test
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_TST;
		break;
	}
	case 9:	//test exclusive
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_TEQ;
		break;
	}
	case 0xa:	//compare
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_CMP;
		break;
	}
		
	case 0xb:	//compare negative
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_CMN;
		break;
	}
	case 0xc:	//or
		return ARM_OP_ORR;
		break;
	case 0xd:	//move
		return ARM_OP_MOV;
		break;
	case 0xe:	//bit clear
		return ARM_OP_BIC;
		break;
	case 0xf:	//not
		return ARM_OP_MVN;
		break;
	}

	return ARM_OP_INVALID;

}

ARM_opcode Cpu::ARM_IsMSR_MRS(uint32_t opcode) {
	uint32_t mrs_mask =		0b0000'1111'1011'1111'0000'1111'1111'1111;
	uint32_t msr_mask =		0b0000'1111'1011'1111'1111'1111'1111'0000;
	uint32_t msri_mask =	0b0000'1101'1011'1111'1111'0000'0000'0000;

	uint32_t mrs_format =	0b0000'0001'0000'1111'0000'0000'0000'0000;
	uint32_t msr_format =	0b0000'0001'0010'1001'1111'0000'0000'0000;
	uint32_t msri_format =	0b0000'0001'0010'1000'1111'0000'0000'0000;

	uint32_t opcode_format = opcode & mrs_mask;
	if (opcode_format == mrs_format) return ARM_OP_MRS;

	opcode_format = opcode & msr_mask;
	if (opcode_format == msr_format) return ARM_OP_MSR;

	opcode_format = opcode & msri_mask;
	if (opcode_format == msri_format) return ARM_OP_MSR;

	return ARM_OP_INVALID;

}

ARM_opcode Cpu::ARM_IsSDTInst(uint32_t opcode) {
	uint32_t mask = 0b0000'1100'0000'0000'0000'0000'0000'0000;
	uint32_t format = 0b0000'0100'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if (format != opcode_format)	//not signle data transfer instruction
		return ARM_OP_INVALID;

	switch ((opcode >> 20) & 1) {	//20th bit = load/store bit
	case 0:	//and
		return ARM_OP_STR;
		break;
	case 1:	//xor
		return ARM_OP_LDR;
		break;
	}

	return ARM_OP_INVALID;

}

inline uint8_t Cpu::leftRotate(uint8_t n, int bits) {
	return (n << bits) | (n >> (8 - bits));
}

inline uint16_t Cpu::leftRotate(uint16_t n, int bits) {
	return (n << bits) | (n >> (16 - bits));
}

inline uint32_t Cpu::leftRotate(uint32_t n, int bits) {
	return (n << bits) | (n >> (32 - bits));
}

inline uint8_t Cpu::rightRotate(uint8_t n, int bits) {
	return (n >> bits) | (n << (8 - bits));
}

inline uint16_t Cpu::rightRotate(uint16_t n, int bits) {
	return (n >> bits) | (n << (16 - bits));
}

inline uint32_t Cpu::rightRotate(uint32_t n, int bits) {
	return (n >> bits) | (n << (32 - bits));
}

inline uint32_t Cpu::arithmRight(uint32_t n, int bits) {
	if (n & 0x80000000) {
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
	reg.R15 += 8 + offset * 4;
	reg.R14 = reg.R15 + 4;
}


inline void Cpu::ARM_ALU_unpacker(uint32_t opcode, uint32_t** destReg, uint32_t& oper1, uint32_t& oper2, uint8_t& c, uint8_t &s) {

	uint8_t I = (opcode >> 25) & 1;
	s = (opcode >> 20) & 1;	//set condition code

	//get destination register
	uint8_t dest_reg_code = (opcode >> 12) & 0x0f;
	*destReg = &((uint32_t*)&reg)[dest_reg_code];

	//get operator 1 register
	uint8_t reg_1_code = (opcode >> 16) & 0x0f;
	oper1 = ((uint32_t*)&reg)[reg_1_code];

	if (I) {	//immidiate 2nd operand 
		uint8_t Is = (opcode >> 8) & 0x0f;
		uint32_t nn = opcode & 0xff;
		oper2 = rightRotate(nn, Is * 2);
		return;
	}
	
	//operand 2 is a register
	ARM_ALU_oper2_getter(opcode, oper2, c);

}

inline void Cpu::ARM_Shifter(uint8_t shiftType, uint8_t shift_amount, uint32_t val, uint32_t& result, uint8_t &c) {

	if (shift_amount) {	//there is a shift
		GBA::clock.addTicks(1);
		switch (shiftType) {
		case 0:		//logical left
			c = (val >> (32 - shift_amount)) & 1;	//carry = last lost bit
			result = val << shift_amount;
			break;

		case 1:		//logical right
			c = (val >> (shift_amount - 1)) & 1;	//carry = last lost bit
			result = val >> shift_amount;
			break;

		case 2:		//arithmetic right
			c = (val >> (shift_amount - 1)) & 1;	//carry = last lost bit
			result = arithmRight(val, shift_amount);
			break;

		case 3:		//rotate right
			c = (val >> (shift_amount - 1)) & 1;	//carry = last rotated bit
			result = rightRotate(val, shift_amount);
			break;
		}
	}
	else {	//special shifts
		switch (shiftType) {
		case 0:		//logical left
			c = reg.CPSR_f->C;
			result = val;
			break;

		case 1:		//logical right
			c = (val >> 31) & 1;	//carry: 31th bit
			result = 0;
			break;

		case 2:		//arithmetic right
			c = (val >> 31) & 1;	//carry: 31th bit
			result = (c == 0 ? 0 : 0xffffffff);
			break;

		case 3:		//rotate right extended
			GBA::clock.addTicks(1);
			uint8_t prev_carry = (val >> 31) & 1;
			c = val & 1;
			result = val >> 1;
			result |= (prev_carry << 31);
			break;
		}
	}
}

inline void Cpu::ARM_ALU_oper2_getter(uint32_t opcode, uint32_t& oper2, uint8_t &c) {

	//get operand 2 register
	uint8_t operand_reg_code = opcode & 0x0f;
	uint32_t Rm = ((uint32_t*)&reg)[operand_reg_code];
	uint32_t real_Rm = 0;
	uint8_t Is;

	if (opcode & 0x10) {	//bit 4 set: shift amount taken from a register
		uint8_t shift_reg_code = (opcode >> 8) & 0x0f;
		uint32_t Rs = ((uint32_t*)&reg)[shift_reg_code];
		Is = Rs & 0xff;
		if (operand_reg_code == 0xf) real_Rm = 12;
	}
	else { //bit 4 clear: immidiate shift amount 
		Is = (opcode >> 7) & 0x1f;
		if (operand_reg_code == 0xf) real_Rm = 8;
	}

	real_Rm += Rm;
	
	uint8_t ST = (opcode >> 5) & 0x3;
	ARM_Shifter(ST, Is, real_Rm, oper2, c);
}

//arithmetic operation
inline void Cpu::Arm_CMP(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t c, s;
	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, c, s);

	uint32_t result = oper1 - oper2;
	reg.CPSR_f->Z = result == 0;
	reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
	reg.CPSR_f->C = !(oper1 < oper2);	//carry = !borrow
	reg.CPSR_f->V = (((oper1 | oper2) ^ result) >> 31) & 1;	//overflow

}

//logical operation
inline void Cpu::Arm_MOV(uint32_t opcode) {
	uint32_t oper1, oper2, *dest_reg;
	uint8_t c, s;
	c = reg.CPSR_f->C;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, c, s);
	*dest_reg = oper2;
	
	if (s) {	//flags
		if (dest_reg != &reg.R15) {
			reg.CPSR_f->Z = oper2 == 0;
			reg.CPSR_f->N = (oper2 & 0x80000000) != 0;	//negative
			reg.CPSR_f->C = c;
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
	uint8_t c, s;
	c = reg.CPSR_f->C;

	ARM_ALU_unpacker(opcode, &dest_reg, oper1, oper2, c, s);

	uint32_t result = oper1 ^ oper2;

	if (s) {	//flags
		reg.CPSR_f->Z = result == 0;
		reg.CPSR_f->N = (result & 0x80000000) != 0;	//negative
		reg.CPSR_f->C = c;
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

inline void Cpu::ARM_SDT_unpacker(uint32_t opcode, uint32_t& address, uint32_t** src_dest_reg, uint8_t& b) {
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

	//get the base address register
	uint8_t base_reg_code = (opcode >> 16) & 0x0f;
	uint32_t *Rn = &((uint32_t*)&reg)[base_reg_code];
	uint32_t Rn_value = *Rn;
	if (base_reg_code == 0x0f) Rn_value += 8;	//R15 + 8

	//get src/dst register
	uint8_t src_dst_reg_code = (opcode >> 12) & 0x0f;
	uint32_t* Rd = &((uint32_t*)&reg)[src_dst_reg_code];
	*src_dest_reg = Rd;

	uint32_t offset = 0;

	if (param.I) {	//offset in register
		uint8_t offset_reg_code = opcode & 0x0f;
		uint32_t Rm = ((uint32_t*)&reg)[offset_reg_code];

		uint8_t Is = (opcode >> 7) & 0x1f;
		uint8_t ST = (opcode >> 5) & 0x3;
		uint8_t c = 0;		//not used

		ARM_Shifter(ST, Is, Rm, offset, c);
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
		if (param.W)	//write back
			*Rn = Rn_value;
	}
}

//load data from address to a register
inline void Cpu::Arm_LDR(uint32_t opcode) {
	uint32_t address, *dest_reg;
	uint8_t b;	//load byte

	ARM_SDT_unpacker(opcode, address, &dest_reg, b);

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

	ARM_SDT_unpacker(opcode, address, &src_reg, b);

	if (b) {	//store byte
		GBA::memory.write_8(address, *src_reg & 0xff);
	}
	else {	//store 32bit word
		/*uint8_t disalignment = address % 4;
		uint32_t val = rightRotate(GBA::memory.read_32(address - disalignment), disalignment * 8);
		*dest_reg = val;*/
		GBA::memory.write_32(address, *src_reg);
	}
}