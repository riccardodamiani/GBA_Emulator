#include "interrupt.h"
#include "gba.h"
#include "memory_mapper.h"

Interrupt::Interrupt() {
	IE = GBA::memory.get_io_reg(0x200);
	IF = GBA::memory.get_io_reg(0x202);
	IME = GBA::memory.get_io_reg(0x208);
}

//set bit 0 og interrupt flag register
void Interrupt::setVBlankFlag() {
	*IF |= 1;	
}

void Interrupt::checkInterrupts() {
	if ((*IME & 1) && (*IE & *IF)) {	//there is a irq request

	}
}