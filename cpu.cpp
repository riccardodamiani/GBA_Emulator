#include "cpu.h"
#include "gba.h"
#include "clock.h"

#include <cstdint>

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

void Cpu::execute_arm(ARM_opcode instruction, uint32_t opcode) {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	switch (instruction) {
	case ARM_OP_B:

		break;

	case ARM_OP_BL:
		break;

	default:
		break;
	}

	reg.R15 += 4;
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
		/*uint32_t offset_24Bit = opcode & 0xffffff;
		int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
		if (opcode & 0x1000000) {	//branch with link
			reg.R15 += 8 + offset * 4;
			reg.R14 = reg.R15 + 4;
			return true;
		}
		//branch
		reg.R15 += 8 + offset * 4;
		return true;
	}
	return false;*/
}


bool Cpu::ARM_IsBranchWithLink(uint32_t opcode) {
	uint32_t format = 0b0000'1011'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1111'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	return format == opcode_format;
}
