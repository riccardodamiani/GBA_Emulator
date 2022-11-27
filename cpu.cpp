#include "cpu.h"
#include "gba.h"

#include <cstdint>

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