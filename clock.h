#ifndef CLOCK_H
#define CLOCK_H

class Clock {
public:
	Clock();
	void addTicks(unsigned long long ticks);
	unsigned long long getTicks();
private:
	unsigned long long _ticks;
};

#endif
