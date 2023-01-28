#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <cstdint>

enum Interrupt_Type {
	IRQ_VBLANK,
	IRQ_HBLANK,
	IRQ_VCOUNTER,
	IRQ_TIMER,
	IRQ_COM,
	IRQ_DMA,
	IRQ_KEYPAD,
	IRQ_GAMEPAK
};

Interrupt_Type const IRQ_BIT_TYPE[] = { 
	IRQ_VBLANK,
	IRQ_HBLANK,
	IRQ_VCOUNTER,
	IRQ_TIMER,
	IRQ_TIMER,
	IRQ_TIMER,
	IRQ_TIMER,
	IRQ_COM,
	IRQ_DMA,
	IRQ_DMA,
	IRQ_DMA,
	IRQ_DMA,
	IRQ_KEYPAD,
	IRQ_GAMEPAK
};

class Interrupt {
public:
	Interrupt();
	void setVBlankFlag();
	void setHBlankFlag();
	void setVCounterFlag();
	void checkInterrupts();
private:
	uint16_t *IE, *IF, *IME;

};

#endif