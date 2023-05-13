#include "interrupt.h"
#include "gba.h"
#include "memory_mapper.h"

Interrupt::Interrupt() {
	IE = GBA::memory.get_io_reg(0x200);
	IF = GBA::memory.get_io_reg(0x202);
	IME = GBA::memory.get_io_reg(0x208);
	irq_cnt = 3;
}

//set bit 0 of interrupt flag register
void Interrupt::setVBlankFlag() {
	if(*IE & 0b1)
		*IF |= 1;	
}

//set bit 1 of interrupt flag register
void Interrupt::setHBlankFlag() {
	if (*IE & 0b10)
		*IF |= 0b10;
}

//set bit 2 of interrupt flag register
void Interrupt::setVCounterFlag() {
	if (*IE & 0b100)
		*IF |= 0b100;
}

//set bit 8-11 of interrupt flag depending on the DMA that triggered the interrupt
void Interrupt::setDMAFlag(uint8_t dmaNr) {
	if (*IE & (0b1 << (8 + dmaNr)))
		*IF |= (0b1 << (8 + dmaNr));
}

void Interrupt::checkInterrupts() {
	if (*IME & 1) {	//there is a irq request
		uint16_t irqs = *IE & *IF;
		if (!irqs) return;	//no irq 

		//need a bit of time before the interrupt happens
		irq_cnt--;
		if (irq_cnt > 0)	
			return;
		irq_cnt = 3;

		//find the irq
		for (int i = 0; i < 14; i++) {
			if ((irqs >> i) & 1) {
				GBA::cpu.RaiseIRQ((Interrupt_Type)i);	//raise the corrisponding irq
				return;
			}
		}
	}
}