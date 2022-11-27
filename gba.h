#ifndef GBA_H
#define GBA_H

#include "memory_mapper.h"
#include "clock.h"
#include "cpu.h"

#include <string>

class GBA {
public:
	static void Load(std::string rom_path);
	static void Run();
	static MemoryMapper memory;
	static Clock clock;
	static Cpu cpu;
private:
};

#endif
