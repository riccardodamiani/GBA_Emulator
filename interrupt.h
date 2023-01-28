#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <cstdint>

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