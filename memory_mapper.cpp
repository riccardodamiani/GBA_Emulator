#include "memory_mapper.h"
#include "gba.h"

#include <string>
#include <fstream>
#include <iostream>

MemoryMapper::MemoryMapper() :
	_bios_mem(new uint8_t[0x4000]),
	_e_wram(new uint8_t[0x40000]),
	_i_wram(new uint8_t[0x8000]),
	_palette_ram(new uint8_t[0x400]),
	_vram(new uint8_t[0x18000]),
	_oam(new uint8_t[0x400])
{
	//init mem
	memset(_bios_mem.get(), 0, 0x4000);
	memset(_e_wram.get(), 0, 0x40000);
	memset(_i_wram.get(), 0, 0x8000);
	memset(_palette_ram.get(), 0, 0x400);
	memset(_vram.get(), 0, 0x18000);
	memset(_oam.get(), 0, 0x400);

	WAITCNT = (WaitCnt*)_ioReg.WAITCNT;

	loadBios();
}

void MemoryMapper::loadRom(std::string rom_filename) {
	_cartridge.open(rom_filename);
}

void MemoryMapper::loadBios() {
	std::ifstream biosFile;

	biosFile.open("gba_bios.bin", std::ios::binary);

	if (!biosFile.is_open()) {
		std::cout << " Error: unable to open gba_bios.bin file." << std::endl;
		exit(0);
	}

	biosFile.seekg(0, biosFile.end);
	uint32_t size = biosFile.tellg();

	if (size != 0x4000) {
		std::cout << " Error: gba_bios.bin has an invalid size." << std::endl;
		exit(0);
	}

	biosFile.seekg(0, biosFile.beg);

	biosFile.read((char *)_bios_mem.get(), 0x4000);
	biosFile.close();
}

uint8_t MemoryMapper::read_8(uint32_t address) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming);
		return _cartridge.read_8(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[0]);

	return addr.memory[addr.addr];
}

uint16_t MemoryMapper::read_16(uint32_t address) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming);
		return _cartridge.read_16(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[1]);

	return *(uint16_t*)&addr.memory[addr.addr];
}

uint32_t MemoryMapper::read_32(uint32_t address) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming * 2);	//game pak bus is only 16 bit wide
		return _cartridge.read_32(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[2]);

	return *(uint32_t*)&addr.memory[addr.addr];
}

void MemoryMapper::write_8(uint32_t address, uint8_t data) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming);
		_cartridge.write_8(address, data);
		return;
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return;

	GBA::clock.addTicks(addr.accessTimings[0]);

	addr.memory[addr.addr] = data;
}

void MemoryMapper::write_16(uint32_t address, uint16_t data) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming);
		_cartridge.write_16(address, data);
		return;
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return;

	GBA::clock.addTicks(addr.accessTimings[1]);

	*(uint16_t*)&addr.memory[addr.addr] = data;
}

void MemoryMapper::write_32(uint32_t address, uint32_t data) {
	gamePakAddr s;

	if ((s = inCartridge(address)).inGamePak) {
		GBA::clock.addTicks(1 + s.accessTiming * 2);	//game pak bus is only 16 bit wide
		_cartridge.write_32(address, data);
		return;
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return;

	GBA::clock.addTicks(addr.accessTimings[2]);

	*(uint32_t*)&addr.memory[addr.addr] = data;
}

realAddress MemoryMapper::find_memory_addr(uint32_t gba_address) {
	uint8_t mem_chunk = (gba_address >> 24) & 0xff;	//8 msb
	uint32_t localAddr = gba_address & 0xffffff;	//24 lsb

	switch (mem_chunk) {
	case 0:		//bios
	{
		if (localAddr > 0x3fff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _bios_mem.get(), localAddr, accessTimings[0] };
		break;
	}
	case 1:	//invalid memory
		return { nullptr, 0, nullptr};
		break;

	case 2:	//external wram
	{
		if (localAddr > 0x3ffff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _e_wram.get(), localAddr, accessTimings[4] };
		break;
	}		
	case 3:		//internal wram
	{
		if (localAddr > 0x7fff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _i_wram.get(), localAddr, accessTimings[1] };
		break;
	}
	case 4:	//io registers
	{
		if (localAddr > 0x803)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		if (localAddr >= 0x90 && localAddr < 0xa0) {	//wave ram bank
			uint8_t bankNr = (_ioReg.SOUND3CNT_L >> 6) & 1;
			return { &wave_ram_banks[bankNr][0], localAddr - 0x90, accessTimings[2] };
		}
		return { (uint8_t*)&_ioReg, localAddr, accessTimings[2] };	//0x3fe??
		break;
	}
	case 5:	//palette ram
	{
		if (localAddr > 0x3ff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _palette_ram.get(), localAddr, accessTimings[5] };
		break;
	}
	case 6:	//vram
	{
		if (localAddr > 0x17fff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _vram.get(), localAddr, accessTimings[6] };
		break;
	}
	case 7:	//oam
	{
		if (localAddr > 0x3ff)
			return { nullptr, 0, nullptr };	//avoid out of bound memory access

		return { _oam.get(), localAddr, accessTimings[3] };
		break;
	}
	default:	//invalid memory
		return { nullptr, 0, nullptr };
		break;
	}
}

//return true and update the system clock on valid game pak memory
gamePakAddr MemoryMapper::inCartridge(uint32_t gba_address) {

	uint8_t mem_chunk = (gba_address >> 24) & 0xff;	//8 msb
	switch (mem_chunk) {
	case 0x8:	//game pak rom
	case 0xa:
	case 0xc:
	{
		int waitSate = (mem_chunk - 8) / 2;
		return { true, waitcntAccessTimings[waitSate * 2][WAITCNT->WS0_fa] };
		break;
	}
	case 0xe:	//game pak sram
		return { true,  waitcntAccessTimings[6][WAITCNT->sram] };
		break;

	default:	//invalid memory
		return { false, 0 };
		break;
	}

	return { false, 0 };
}