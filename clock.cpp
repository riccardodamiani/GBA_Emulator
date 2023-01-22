#include "clock.h"
#include "lcd_controller.h"
#include "gba.h"

LcdController GBA::GBA::lcd_ctl;

Clock::Clock() {
	_ticks = 0;
}

void Clock::addTicks(unsigned long long ticks) {
	_ticks += ticks;

	GBA::lcd_ctl.update_V_count(ticks);
}

unsigned long long Clock::getTicks() {
	return _ticks;
}

void Clock::clear() {
	_ticks = 0;
}