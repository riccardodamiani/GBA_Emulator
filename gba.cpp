#include "gba.h"
#include "memory_mapper.h"

#include <string>

MemoryMapper GBA::memory;

void GBA::Load(std::string rom_filename) {
	memory.loadRom(rom_filename);
}

void GBA::Run() {
	while (1) {
		GBA::cpu.runFor(16780);
	}
}