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
	if (inCartridge(address)) {
		return _cartridge.read_8(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[0]);

	return addr.memory[addr.addr];
}

uint16_t MemoryMapper::read_16(uint32_t address) {
	if (inCartridge(address)) {
		return _cartridge.read_16(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[1]);

	return *(uint16_t*)&addr.memory[addr.addr];
}

uint32_t MemoryMapper::read_32(uint32_t address) {
	if (inCartridge(address)) {
		return _cartridge.read_32(address);
	}
	realAddress addr = find_memory_addr(address);

	if (addr.memory == nullptr)
		return 0;

	GBA::clock.addTicks(addr.accessTimings[2]);

	return *(uint32_t*)&addr.memory[addr.addr];
}

void MemoryMapper::write_8(uint32_t address, uint8_t data) {
	if (inCartridge(address)) {
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
	if (inCartridge(address)) {
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
	if (inCartridge(address)) {
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
	uint8_t mem_chunk = (gba_address >> 24) & 0xff;

	switch (mem_chunk) {
	case 0:		//bios
		return { _bios_mem.get(), gba_address & 0x3fff, accessTimings[0]};
		break;
	case 1:	//invalid memory
		return { nullptr, 0, nullptr};
		break;

	case 2:	//external wram
		return { _e_wram.get(), gba_address & 0x3ffff, accessTimings[4] };
		break;

	case 3:		//internal wram
		return { _i_wram.get(), gba_address & 0x7fff, accessTimings[1] };
		break;

	case 4:	//io registers
		if ((gba_address & 0xfff) > 0x803) 
			return { nullptr, 0, nullptr };	//avoid overflow in io registers
		return { (uint8_t*) &_ioReg, gba_address & 0xfff, accessTimings[2] };	//0x3fe??
		break;

	case 5:	//palette ram
		return { _palette_ram.get(), gba_address & 0x3ff, accessTimings[5] };
		break;

	case 6:	//vram
		return { _vram.get(), gba_address & 0x17fff, accessTimings[6] };
		break;

	case 7:	//oam
		return { _oam.get(), gba_address & 0x3ff, accessTimings[3] };
		break;

	default:	//invalid memory
		return { nullptr, 0, nullptr };
		break;
	}
}

bool MemoryMapper::inCartridge(uint32_t gba_address) {

	uint8_t mem_chunk = (gba_address >> 24) & 0xff;

	if (mem_chunk >= 0x8 && mem_chunk <= 0xe) {	//cartridge
		return true;
	}
	return false;
}