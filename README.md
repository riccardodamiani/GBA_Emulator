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
* Keypad inputs
* Save/load states
	* sram
* Audio
	* DMA sound channel A
  
Not even started:  
* Serial communication
* Timers
* Save/load states
	* eeprom
	* flash
  
In progress:
* Arm/Thumb instruction sets (~80% done)
* Video:
	* Object 2-dimentional tile mapping
* Cpu interrupts:
	* V-blank
* Cpu timings
* DMAs
* Audio
	* DMA sound channel B (disabled due to a bug)

## Keyboard map
Not customizable yet.
| GBA   	 	| Keyboard  	| 
|---------------|---------------|
| a 			| o 			|
| b 			| p 			|
| start 		| space 		|
| select		| left shift 	|
| left 			| a 			|
| right 		| d 			|
| up			| w 			|
| down 			| s 			|
| r button		| e 			|
| l button		| q 			|
| F1 button		| save state    |