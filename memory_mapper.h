#ifndef MEMORY_MAPPER_H
#define MEMORY_MAPPER_H

#include "cartridge.h"
#include "io_registers.h"

#include <string>
#include <cstdint>
#include <memory>

struct realAddress {
	uint8_t* memory;
	uint32_t addr;
	const int* accessTimings;
};

const int accessTimings[][3] = {
	{1, 1, 1},	//BIOS ROM
	{1, 1, 1},	//INTERNAL WRAM
	{1, 1, 1},	//I/O
	{1, 1, 1},	//OAM
	{3, 3, 6},	//E-WRAM
	{1, 1, 2},	//PALETTE RAM
	{1, 1, 2},	//VRAM
	{5, 5, 8},	//GAMEPAK ROM
	{5, 5, 8},	//GAMEPAK FLASH
	{5, 5, 5}	//GAMEPAK SRAM
};


class MemoryMapper {
public:
	MemoryMapper();
	void loadRom(std::string rom_filename);
	uint8_t read_8(uint32_t address);
	uint16_t read_16(uint32_t address);
	uint32_t read_32(uint32_t address);
	void write_8(uint32_t address, uint8_t data);
	void write_16(uint32_t address, uint16_t data);
	void write_32(uint32_t address, uint32_t data);
private:
	//memory
	std::unique_ptr <uint8_t[]> _bios_mem;
	std::unique_ptr <uint8_t[]> _e_wram;
	std::unique_ptr <uint8_t[]> _i_wram;
	std::unique_ptr <uint8_t[]> _palette_ram;
	std::unique_ptr <uint8_t[]> _vram;
	std::unique_ptr <uint8_t[]> _oam;
	Cartridge _cartridge;
	Io_registers _ioReg;

	void loadBios();
	realAddress find_memory_addr(uint32_t gba_address);
	bool inCartridge(uint32_t addr);
};

#endif
