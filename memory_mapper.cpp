#include "memory_mapper.h"
#include "gba.h"
#include "error.h"
#include "dma.h"

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>


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

	WAITCNT = (WaitCnt*)&_ioReg.WAITCNT;

	//create DMAs objects
	for (int i = 0; i < 4; i++) {
		_dma[i].reset(new Dma(i));
	}

	fifoIndex[0] = fifoIndex[1] = 0;
	memset(fifo, 0, sizeof(fifo));

	loadBios();
}

MemoryMapper::~MemoryMapper() {

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

	if (addr.memory == (uint8_t*)&_ioReg) {	//register
		write_register(addr.addr, addr.memory[addr.addr], data);
		return;
	}

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

	if (addr.memory == (uint8_t*)&_ioReg) {	//register
		write_register(addr.addr, *(uint16_t*)(&addr.memory[addr.addr]), data);
		return;
	}

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

	if (addr.memory == (uint8_t*)&_ioReg) {	//register
		write_register(addr.addr, *(uint32_t*)(&addr.memory[addr.addr]), data);
		return;
	}

	*(uint32_t*)&addr.memory[addr.addr] = data;
}

//return a pointer to a io register
uint16_t* MemoryMapper::get_io_reg(uint32_t offset) {
	return (uint16_t *) ( & ((uint8_t*)&_ioReg)[offset]);
}

void MemoryMapper::setKeyInput(keyinput_struct input) {
	_ioReg.KEYINPUT.a = 1 - input.a;
	_ioReg.KEYINPUT.b = 1 - input.b;
	_ioReg.KEYINPUT.select = 1 - input.select;
	_ioReg.KEYINPUT.start = 1 - input.start;
	_ioReg.KEYINPUT.right = 1 - input.right;
	_ioReg.KEYINPUT.left = 1 - input.left;
	_ioReg.KEYINPUT.up = 1 - input.up;
	_ioReg.KEYINPUT.down = 1 - input.down;
	_ioReg.KEYINPUT.r = 1 - input.r;
	_ioReg.KEYINPUT.l = 1 - input.l;
}

uint32_t MemoryMapper::getFifo(uint8_t ch) {

	uint8_t dma = 0;
	if (_ioReg.DMA1DAD == (0x40000A0 + ch * 4)) {
		dma = 1;
	}
	else {
		dma = 2;
	}

	if (fifoIndex[ch & 1] <= 4) {
		_dma[dma]->trigger(FIFO);
	}

	if(fifoIndex[ch & 1] == 0)
		return 0;

	uint32_t firstOut = fifo[ch & 1][0];
	
	for (int i = 1; i < fifoIndex[ch & 1]; i++) {
		fifo[ch & 1][i - 1] = fifo[ch & 1][i];
	}

	fifoIndex[ch & 1]--;

	return firstOut;
}

void MemoryMapper::writeFifo(uint32_t val, uint8_t ch) {
	if (fifoIndex[ch & 1] == 8)
		return;

	fifo[ch][fifoIndex[ch & 1]] = val;
	fifoIndex[ch & 1]++;
}

uint8_t* MemoryMapper::getMemoryAddr(int chunk) {

	switch (chunk) {
	case 0:
		return _bios_mem.get();
		break;
	case 2:
		return _e_wram.get();
		break; 
	case 3:
		return _i_wram.get();
		break;
	case 4:
		return (uint8_t *)&_ioReg;
		break;
	case 5:
		return _palette_ram.get();
		break;
	case 6:
		return _vram.get();
		break;
	case 7:
		return _oam.get();
		break;
	default:
		return nullptr;
	}
}

realAddress MemoryMapper::find_memory_addr(uint32_t gba_address) {
	uint8_t mem_chunk = (gba_address >> 24) & 0xff;	//8 msb
	uint32_t localAddr = gba_address & 0xffffff;	//24 lsb

	switch (mem_chunk) {
	case 0:		//bios
	{
#ifdef _DEBUG
		if (localAddr > 0x3fff)
			printError(CRITICAL_ERROR, "trying to access out of bound memory");
#endif
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
#ifdef _DEBUG
		if (localAddr > 0x3ffff)
			printError(CRITICAL_ERROR, "trying to access out of bound memory");
#endif
		return { _e_wram.get(), localAddr & 0x3ffff, accessTimings[4] };
		break;
	}		
	case 3:		//internal wram
	{
		return { _i_wram.get(), localAddr & 0x7fff, accessTimings[1] };
		break;
	}
	case 4:	//io registers
	{
#ifdef _DEBUG
		if (localAddr > 0x803)
			printError(CRITICAL_ERROR, "trying to access out of bound memory");
#endif
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
#ifdef _DEBUG
		if (localAddr > 0x3ff)
			printError(CRITICAL_ERROR, "trying to access out of bound memory");
#endif
		return { _palette_ram.get(), localAddr & 0x3ff, accessTimings[5] };
		break;
	}
	case 6:	//vram
	{
		localAddr &= 0x1ffff;	//mirrors every 128k
		if (localAddr > 0x17fff) {	//last 32k mirrors the previous 32k
			localAddr = 0xffff /* 64k */  + localAddr & 0x7fff /* mirror of used 32k */;
		}
		return { _vram.get(), localAddr, accessTimings[6] };
		break;
	}
	case 7:	//oam
	{
#ifdef _DEBUG
		if (localAddr > 0x3ff)
			printError(CRITICAL_ERROR, "trying to access out of bound memory");
#endif
		return { _oam.get(), localAddr & 0x3ff, accessTimings[3] };
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
	case 0x9:
	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
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

void MemoryMapper::write_register(uint32_t gba_addr,
	uint8_t& real_mem, uint8_t data) {
	if (gba_addr > 0x3fe)
		return;

	switch (gba_addr) {
	case 0x202:	//clear interrupt flag
	case 0x203:
		real_mem &= ~data;
		break;
	default:
		real_mem = data;
		break;
	}
}

void MemoryMapper::write_register(uint32_t gba_addr, 
	uint16_t &real_mem, uint16_t data) {
	if (gba_addr > 0x3fe)
		return;

	switch (gba_addr) {
	case 0xba:	//DMA0 control
		real_mem = data;
		if (data & 0x8000) {	//DMA enable
			_dma[0]->enable_dma();
			_dma[0]->trigger(Dma_Trigger::EMPTY_TRIGGER);
		}
		break;
	case 0xc6:	//DMA1 control
		real_mem = data;
		if (data & 0x8000) {	//DMA enable
			_dma[1]->enable_dma();
			_dma[1]->trigger(Dma_Trigger::EMPTY_TRIGGER);
		}
		break;
	case 0xd2:	//DMA2 control
		real_mem = data;
		if (data & 0x8000) {	//DMA enable
			_dma[2]->enable_dma();
			_dma[2]->trigger(Dma_Trigger::EMPTY_TRIGGER);
		}
		break;
	case 0xd6:	//DMA3 control
		real_mem = data;
		if (data & 0x8000) {	//DMA enable
			_dma[3]->enable_dma();
			_dma[3]->trigger(Dma_Trigger::EMPTY_TRIGGER);
		}
		break;
	case 0x202:	//clearing interrupt flag
		_ioReg.IF &= ~data;
		break;
	default:
		real_mem = data;
		break;
	}
}

void MemoryMapper::write_register(uint32_t gba_addr, 
	uint32_t& real_mem, uint32_t data) {
	if (gba_addr > 0x3fe)
		return;

	switch (gba_addr) {
	case 0xa0:
		writeFifo(data, 0);
		break;
	case 0xa4:
		writeFifo(data, 1);
		break;
	default:
		real_mem = data;
		break;
	}
}