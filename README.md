# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
The emulator is not able to execute the game yet and will probably freeze somewhere in the bios because of some instructions not yet implemented.  
Working on intro screen..

Done:
* Bios loading
* Game rom loading
* Gba memory mapping
* GamePak:
	* read data from GamePak
* Video:
	* object affine transformation
	* Objects priority
	* rendering of background 3 in mode 2
	* special effect: alpha blending
	* special effect: brightness adjust
	* graphic layers priority
  
Not even started:  
* Audio 
* Dma
* Keypad inputs
* Serial communication
* Timers
  
In progress:
* Arm/Thumb instruction sets (~80% done)
* Video:
	* Object 2-dimentional tile mapping
	* Object 256 color palette
* Cpu interrupts:
	* V-blank
* Cpu timings