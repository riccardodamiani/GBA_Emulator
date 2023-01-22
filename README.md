# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
The emulator is not able to execute the game yet and will probably freeze somewhere in the bios because of some instructions not yet implemented.  
The execution of the bios is tested until address 0x19a4.  

Done:
* Bios loading
* Game rom loading
* Gba memory mapping

Not yet supported:  
* Audio 
* Cpu interrupts
* Cpu timings
* Dma
* Keypad inputs
* Serial communication
* Timers

In progress:
* Arm/Thumb instruction sets (~75% done)
* GamePak 
* Video