#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <cstdint>

enum Interrupt_Type {
	IRQ_VBLANK = 0,
	IRQ_HBLANK = 1,
	IRQ_VCOUNTER = 2,
	IRQ_TIMER_0 = 3,
	IRQ_TIMER_1 = 4,
	IRQ_TIMER_2 = 5,
	IRQ_TIMER_3 = 6,
	IRQ_COM = 7,
	IRQ_DMA_0 = 8,
	IRQ_DMA_1 = 9,
	IRQ_DMA_2 = 10,
	IRQ_DMA_3 = 11,
	IRQ_KEYPAD = 12,
	IRQ_GAMEPAK = 13
};

class Interrupt {
public:
	Interrupt();
	void setVBlankFlag();
	void setHBlankFlag();
	void setVCounterFlag();
	void setDMAFlag(uint8_t dmaNr);
	void checkInterrupts();
private:
	uint16_t *IE, *IF, *IME;
	uint8_t irq_cnt;

};

#endif