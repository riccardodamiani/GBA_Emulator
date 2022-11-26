#include "clock.h"

Clock::Clock() {
	_ticks = 0;
}

void Clock::addTicks(unsigned long long ticks) {
	_ticks += ticks;
}

unsigned long long Clock::getTicks() {
	return _ticks;
}