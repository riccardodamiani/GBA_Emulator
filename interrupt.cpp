#include "interrupt.h"
#include "gba.h"
#include "memory_mapper.h"

Interrupt::Interrupt() {
	IE = GBA::memory.get_io_reg(0x200);
	IF = GBA::memory.get_io_reg(0x202);
	IME = GBA::memory.get_io_reg(0x208);
}

//set bit 0 of interrupt flag register
void Interrupt::setVBlankFlag() {
	*IF |= 1;	
}

//set bit 1 of interrupt flag register
void Interrupt::setHBlankFlag() {
	*IF |= 0b10;
}

//set bit 2 of interrupt flag register
void Interrupt::setVCounterFlag() {
	*IF |= 0b100;
}

void Interrupt::checkInterrupts() {
	if (*IME & 1) {	//there is a irq request
		uint16_t irqs = *IE & *IF;
		if (!irqs) return;	//no irq 

		for (int i = 0; i < 14; i++) {	//find the irq
			if ((irqs >> i) & 1) {
				GBA::cpu.RaiseIRQ((Interrupt_Type)i);	//raise the corrisponding irq
				return;
			}
		}
	}
}