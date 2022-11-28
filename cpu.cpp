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

	if (ARM_IsBranch(opcode)) return ARM_opcode::ARM_OP_B;
	if (ARM_IsBranchWithLink(opcode)) return ARM_opcode::ARM_OP_BL;

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

bool Cpu::ARM_IsBranch(uint32_t opcode) {
	uint32_t format = 0b0000'1010'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1110'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	return format == opcode_format;
}


bool Cpu::ARM_IsBranchWithLink(uint32_t opcode) {
	uint32_t format = 0b0000'1011'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1111'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	return format == opcode_format;
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
