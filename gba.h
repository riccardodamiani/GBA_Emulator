#ifndef GBA_H
#define GBA_H

#include "memory_mapper.h"
#include "clock.h"
#include "lcd_controller.h"
#include "cpu.h"
#include "graphics.h"
#include "interrupt.h"

#include <string>

class GBA {
public:
	static void Load(std::string rom_path);
	static void Run();
	static MemoryMapper memory;
	static Clock clock;
	static Cpu cpu;
	static LcdController lcd_ctl;
	static Interrupt irq;
private:
	static double limit_fps(double elapsedTime, double maxFPS);
	static Graphics graphics;
};

#endif
