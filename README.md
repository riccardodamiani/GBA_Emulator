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
* DMAs
	* DMA enable and registers load
	* Repeat bit
	* Transfer trigger (h-blank, v-blank, FIFOs)
* Audio
	* DMA sound channel A
* Interrupts:
	* H-blank, V-blank, V-couter, keypad, DMAs
 
Partially done:
* Arm/Thumb instruction sets (~80% done)
* Video:
	* Object 2-dimentional tile mapping
* Cpu timings
* Audio
	* DMA sound channel B (disabled due to a bug)
* Video:
	* graphic mode 2

To do:  
* Serial communication
* Timers
* Save/load states
	* eeprom
	* flash
* Video:
	* Graphic modes 1, 3, 4, 5
* Interrupts:
	* Timers


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