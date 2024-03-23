#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <cstdint>
#include <string>
#include <memory>

struct CartHeader {
	uint32_t entryPoint;
	uint8_t nintendoLogo[156];
	uint8_t gameTitle[12];
	uint8_t gameCode[4];
	uint8_t makerCode[2];
	uint8_t fixed;
	uint8_t mainUnitCode;
	uint8_t deviceType;
	uint8_t reserved[7];
	uint8_t swVersion;
	uint8_t checksum;
	uint8_t reserved_2[2];
};

class Cartridge {
public:
	void open(std::string rom_filename);
	bool saveSram();
	uint8_t read_8(uint32_t address);
	uint16_t read_16(uint32_t address);
	uint32_t read_32(uint32_t address);
	void write_8(uint32_t addr, uint8_t data);
	void write_16(uint32_t addr, uint16_t data);
	void write_32(uint32_t addr, uint32_t data);
private:
	std::unique_ptr <uint8_t[]> _rom;
	uint32_t _romSize;
	std::unique_ptr <uint8_t[]> _eeprom;
	uint32_t _eepromSize;
	std::unique_ptr <uint8_t[]> _sram;
	uint32_t _sramSize;
	std::unique_ptr <uint8_t[]> _flash;
	uint32_t _flashSize;

	CartHeader* _header;
	std::string _rom_name;
	std::string _rom_path;

	void load(std::string rom_filename);
	bool load_state();
	void find_rom_name(std::string romPath);
	void findBackupId();
	void findEeprom();
	void findFlash();
	void findSram();
};

#endif
