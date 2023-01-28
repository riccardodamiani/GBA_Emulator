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

void Interrupt::setHBlankFlag() {
	*IF |= 0b10;
}

void Interrupt::setVCounterFlag() {
	*IF |= 0b100;
}

void Interrupt::checkInterrupts() {
	if (*IME & 1) {	//there is a irq request
		uint16_t irqs = *IE & *IF;
		if (!irqs) return;	//no irq 

		for (int i = 0; i < 14; i++) {	//find the irq
			if ((irqs >> i) & 1) {
				GBA::cpu.RaiseIRQ(IRQ_BIT_TYPE[i]);	//raise the corrisponding irq
				return;
			}
		}
	}
}