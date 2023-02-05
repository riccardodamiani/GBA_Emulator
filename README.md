# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
The emulator is not able to execute the game yet and will probably freeze somewhere in the bios because of some instructions not yet implemented.  
Working on intro screen..

Done:
* Bios loading
* Game rom loading
* Gba memory mapping

Not even started:  
* Audio 
* Dma
* Keypad inputs
* Serial communication
* Timers

In progress:
* Arm/Thumb instruction sets (~80% done)
* GamePak:
	* read from GamePak
* Video:
	* Object 2-dimentional tile mapping
	* Object 256 color palette
	* object affine transformation
	* Objects priority
* Cpu interrupts:
	* V-blank
* Cpu timings