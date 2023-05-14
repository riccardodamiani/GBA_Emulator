# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
In the current state the emulator is able to display the intro screen and boot into the game.  

Done:
* Bios loading
* Game rom loading
* Gba memory mapping
* GamePak:
	* read data from GamePak
* Video:
	* object affine transformation
	* Objects priority
	* graphic mode 0
	* background 3 of graphic mode 2
	* special effect: alpha blending
	* special effect: brightness adjust
	* graphic layers priority
	* 256/1 and 16/16 color palette
  
Not even started:  
* Audio 
* Keypad inputs
* Serial communication
* Timers
  
In progress:
* Arm/Thumb instruction sets (~80% done)
* Video:
	* Object 2-dimentional tile mapping
* Cpu interrupts:
	* V-blank
* Cpu timings
* DMAs