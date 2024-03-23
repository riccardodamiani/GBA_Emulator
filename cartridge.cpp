#include "cartridge.h"

#include <string>
#include <fstream>
#include <iostream>

void Cartridge::open(std::string rom_filename) {

	load(rom_filename);

	_header = (CartHeader*)_rom.get();

	if (_header->fixed != 0x96) {
		std::cout << " Error: invalid fixed value in rom header" << std::endl;
		exit(0);
	}

	//check checksum
	uint16_t chk = 0;
	for (int i = 0xa0; i < 0xbc; i++) {
		chk = chk - _rom[i];
	}
	chk -= 0x19;
	chk &= 0xff;

	if (chk != _header->checksum) {
		std::cout << " Error: the rom header checksum failed" << std::endl;
		exit(0);
	}
	find_rom_name(rom_filename);
	std::cout << "Rom path: " << _rom_path << std::endl;
	std::cout << "Rom name: " << _rom_name << std::endl;

	_eepromSize = 0;
	_sramSize = 0;
	_flashSize = 0;

	load_state();
	findBackupId();

}

void Cartridge::find_rom_name(std::string romPath) {
	size_t endOfPath = romPath.find_last_of("/\\");
	if (endOfPath == std::string::npos) {
		endOfPath = 0;
	}

	size_t endOfName = romPath.find_last_of(".");
	if (endOfName == std::string::npos) {
		endOfName = romPath.size();
	}
	_rom_name = romPath.substr(endOfPath, romPath.size() - (romPath.size() - endOfName));
	_rom_path = romPath.substr(0, endOfPath);
}

bool Cartridge::saveSram() {
	std::string sram_filename = _rom_path + _rom_name + ".sram";
	std::ofstream sramFile;

	sramFile.open(sram_filename, std::ios::binary);
	if (!sramFile.is_open()) {
		std::cout << "Error: unable to open the sram file " << sram_filename << std::endl;
		exit(0);
	}
	if (_sramSize != 0) {
		sramFile.exceptions(std::ofstream::badbit | std::ofstream::failbit);
		try {
			sramFile.write((const char*)_sram.get(), _sramSize);
		}
		catch(std::ofstream::failure& e){
			std::cout << "Error: unable to write to file " << sram_filename << std::endl;
		}
	}
	sramFile.close();
	std::cout << "Sram state saved correctly" << std::endl;
	return true;
}

void Cartridge::load(std::string rom_filename) {
	std::ifstream romFile;

	romFile.open(rom_filename, std::ios::binary);

	if (!romFile.is_open()) {
		std::cout << " Error: unable to open the rom file " << rom_filename << std::endl;
		exit(0);
	}

	romFile.seekg(0, romFile.end);
	uint32_t size = romFile.tellg();	//rom size
	_rom.reset(new uint8_t[size]);	//allocate rom memory to a unique_ptr

	romFile.seekg(0, romFile.beg);

	romFile.read((char*)_rom.get(), size);	//load the rom file
	romFile.close();

	_romSize = size;
	return;
}

void Cartridge::load_state() {
	std::ifstream sramFile;

	std::string sramFilename = _rom_path + _rom_name + ".sram";
	sramFile.open(sramFilename, std::ios::binary);

	//load the sram files if it finds it
	if (sramFile.is_open()) {
		std::cout << "Found sram file" << std::endl;
		sramFile.seekg(0, sramFile.end);
		uint32_t size = sramFile.tellg();	//sram size

		_sram.reset(new uint8_t[size]);	//allocate sram memory to a unique_ptr

		sramFile.seekg(0, sramFile.beg);

		sramFile.read((char*)_sram.get(), size);	//load the sram file
		sramFile.close();

		_sramSize = size;
		std::cout << "Loaded " << _sramSize  << " bytes of sram from file" << std::endl;
	}
}

void Cartridge::findBackupId() {
	
	if(_sramSize == 0)
		findSram();

	if(_eepromSize == 0)
		findEeprom();

	if(_flashSize == 0)
		findFlash();
	
}

void Cartridge::findEeprom() {
	const char str[] = "EEPROM_V";

	for (int i = 0; i < _romSize-8; i++) {
		if (memcmp(&_rom[i], str, 8) == 0) {
			std::cout << " Warning: eeprom memory detected but not handled" << std::endl;
			return;
		}
	}
}


void Cartridge::findFlash() {
	const char str[] = "FLASH";

	for (int i = 0; i < _romSize-10; i++) {
		if (memcmp(&_rom[i], str, 5) == 0) {
			if (_rom[i + 5] == '5' || _rom[i + 5] == '_') {	//512 kbit (64 kBytes)
				_flashSize = 0x10000;
				_flash.reset(new uint8_t[0x10000]);
			}
			else {		//1Mbit (128 kBytes)
				_flashSize = 0x20000;
				_flash.reset(new uint8_t[0x20000]);
			}
			std::cout << "Allocated " << _flashSize << " bytes of flash" << std::endl;
			return;
		}
	}
}


void Cartridge::findSram() {
	
	const char str[] = "SRAM_V";

	for (int i = 0; i < _romSize-10; i++) {
		if (memcmp(&_rom[i], str, 6) == 0) {
			_sramSize = 0x8000;
			_sram.reset(new uint8_t[0x8000]);
			memset(_sram.get(), 0xff, 0x8000);
			std::cout << "Allocated " << _sramSize << " bytes of sram" << std::endl;
			return;
		}
	}
}

uint8_t Cartridge::read_8(uint32_t address) {
	uint32_t memoryAddr = address & 0x1ffffff;
	uint32_t memChunk = (address >> 24) & 0xff;

	uint32_t memSize = 0;
	uint8_t* mem = nullptr;

	if (memChunk == 0xe) {	//sram
		memSize = _sramSize;
		mem = _sram.get();
	}
	else {	//rom
		memSize = _romSize;
		mem = _rom.get();
	}

	if (memoryAddr >= memSize) {
		return (address / 2) & 0xff;
	}

	return mem[memoryAddr];
}

uint16_t Cartridge::read_16(uint32_t address) {
	uint32_t memoryAddr = address & 0x1ffffff;
	uint32_t memChunk = (address >> 24) & 0xff;

	uint32_t memSize = 0;
	uint8_t* mem = nullptr;

	if (memChunk == 0xe) {	//sram
		memSize = _sramSize;
		mem = _sram.get();
	}
	else {	//rom
		memSize = _romSize;
		mem = _rom.get();
	}

	if (memoryAddr >= memSize) {
		return (address / 2) & 0xffff;
	}

	return *(uint16_t *)(&mem[memoryAddr]);
}

uint32_t Cartridge::read_32(uint32_t address) {
	uint32_t memoryAddr = address & 0x1ffffff;
	uint32_t memChunk = (address >> 24) & 0xff;

	uint32_t memSize = 0;
	uint8_t* mem = nullptr;

	if (memChunk == 0xe) {	//sram
		memSize = _sramSize;
		mem = _sram.get();
	}
	else {	//rom
		memSize = _romSize;
		mem = _rom.get();
	}

	if (memoryAddr >= memSize) {
		return (address / 2) & 0xffff | ((((address + 4) / 2) & 0xffff) << 16);
	}

	return *(uint32_t*)(&mem[memoryAddr]);
}

void Cartridge::write_8(uint32_t addr, uint8_t data) {
	uint32_t memoryAddr = addr & 0x1ffffff;
	uint32_t memChunk = (addr >> 24) & 0xff;

	if (memChunk != 0xe)	//not sram
		return;
	if (_sramSize == 0)
		return;

	memoryAddr %= 0x8000;
	_sram.get()[memoryAddr] = data;
}

void Cartridge::write_16(uint32_t addr, uint16_t data) {


}

void Cartridge::write_32(uint32_t addr, uint32_t data) {


}