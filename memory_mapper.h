#ifndef MEMORY_MAPPER_H
#define MEMORY_MAPPER_H

#include "cartridge.h"
#include "io_registers.h"

#include <string>
#include <cstdint>
#include <memory>

struct WaitCnt {
	uint16_t sram : 2,	//game pak ram wait control
		WS0_fa : 2,	//wait state 0 first access
		WS0_sa : 1,	//wait state 1 second access
		WS1_fa : 2,	//..
		WS1_sa : 1,
		WS2_fa : 2,
		WS2_sa : 1,
		PHI : 2,	//phi terminal output
		not_used : 1,
		GB_prefetch : 1,	//game pak prefetch buffer
		GP_type : 1;	//game pak type
	uint16_t not_used2;
};

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

const int waitcntAccessTimings[][4] = {
	{4, 3, 2, 8},		//WS0 first access
	{2, 1, 0, 0},		//WS0 second access
	{4, 3, 2, 8},		//WS1 first access
	{4, 1, 0, 0},		//WS1 second access
	{4, 3, 2, 8},		//WS2 first access
	{8, 1, 0, 0},		//WS2 second access
	{4, 3, 2, 8},		//sram
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
	uint8_t wave_ram_banks[2][0x10];
	WaitCnt *WAITCNT;

	void loadBios();
	realAddress find_memory_addr(uint32_t gba_address);
	bool inCartridge(uint32_t addr);
};

#endif
