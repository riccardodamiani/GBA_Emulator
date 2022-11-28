#include "cpu.h"
#include "gba.h"
#include "clock.h"

#include <cstdint>
#include <iostream>

Clock GBA::clock;

Cpu::Cpu() {
	reg = {};
}

void Cpu::runFor(uint32_t ticks) {
	unsigned long long startingTicks = GBA::clock.getTicks();

	unsigned long long endingTicks = startingTicks + ticks;

	while (GBA::clock.getTicks() < endingTicks) {
		next_instruction();
	}
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
	CPSR_registers* status = (CPSR_registers *)&reg.CPSR;

	if (status->T) {	//thumb
		next_instruction_thumb();
		return;
	}

	next_instruction_arm();
}

ARM_opcode Cpu::decode_arm(uint32_t opcode) {

	ARM_opcode instr;

	if (instr = ARM_IsBranch(opcode)) return instr;
	if (instr = ARM_IsAluInst(opcode)) return instr;

	return ARM_opcode::ARM_OP_INVALID;
}

bool Cpu::arm_checkInstructionCondition(uint32_t opcode) {
	uint8_t condition = (opcode >> 28) & 0x0f;
	CPSR_registers* flag = (CPSR_registers*)&reg.CPSR;

	bool condition_met = false;

	switch (condition) {
	case 0:	//EQ (z = 1)
		condition_met = flag->Z == 1;
		break;
	case 1:		//NE (z = 0)
		condition_met = flag->Z == 0;
		break;
	case 2:		//CS/HS (c = 1)
		condition_met = flag->C == 1;
		break;
	case 3:		//CC/LO	(c = 0)
		condition_met = flag->C == 0;
		break;
	case 4:		//MI (N = 1) negative
		condition_met = flag->N == 1;
		break;
	case 5:		//PL (N = 0) >= 0
		condition_met = flag->N == 0;
		break;
	case 6:		//VS (V = 1) overflow
		condition_met = flag->V == 1;
		break;
	case 7:		//VC (V = 0) no overflow
		condition_met = flag->V == 0;
		break;
	case 8:		//HI (C = 1 or Z = 0)	unsigned higher
		condition_met = (flag->C == 1) || (flag->Z == 0);
		break;
	case 9:		//LS (C = 0 or Z=1)	unsigned lower or same
		condition_met = (flag->C == 0) || (flag->Z == 1);
		break;
	case 0xa:	//GE (N = V) signed greater or equal
		condition_met = flag->N == flag->V;
		break;
	case 0xb:	//LT (N!=V)	signed less than
		condition_met = flag->N != flag->V;
		break;
	case 0xc:	//GT (Z=0 and N=V)	signed greater than
		condition_met = (flag->Z == 0) && (flag->N == flag->V);
		break;
	case 0xd:	//LE (Z=1 or N!=V)	signed less or equal
		condition_met = (flag->Z == 1) || (flag->N != flag->V);
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
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

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

	case ARM_OP_INVALID:
		std::cout << "Error: Instruction not implemented: " << std::hex << opcode << std::endl;
		system("pause");
		exit(1);
		break;
	default:
		std::cout << "Error: Instruction not implemented: " << std::hex << opcode << std::endl;
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
		return ARM_OP_TST;
		break;
	case 9:	//test exclusive
		return ARM_OP_TEQ;
		break;
	case 0xa:	//compare
		return ARM_OP_CMP;
		break;
	case 0xb:	//compare negative
		return ARM_OP_CMN;
		break;
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

//branch
void Cpu::Arm_B(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R15 += 8 + offset * 4;

}

//branch with link
void Cpu::Arm_BL(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R15 += 8 + offset * 4;
	reg.R14 = reg.R15 + 4;

}
