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
		execute();
	}
}

void Cpu::next_instruction() {

}

//execute the next instruction
void Cpu::execute() {
	CPSR_registers* status = (CPSR_registers *)&reg.CPSR;

	if (status->T) {	//thumb
		execute_thumb();
		return;
	}

	execute_arm();
}

void  Cpu::execute_arm() {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	//align R15
	reg.R15 -= reg.R15 % 4;

	uint32_t instruction = GBA::memory.read_32(reg.R15);


	reg.R15 += 4;
}

void Cpu::execute_thumb() {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	//align R15
	reg.R15 -= reg.R15 % 2;

	uint16_t instruction = GBA::memory.read_16(reg.R15);

	reg.R15 += 2;
}

bool Cpu::isBranchOrBranchWithLink(uint32_t opcode) {
	uint32_t format = 0b0000'1010'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1110'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	return format == opcode_format;
}