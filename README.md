# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
The emulator is not able to execute the game yet and will probably freeze somewhere in the bios because of some instructions not yet implemented.  
The execution of the bios is tested until address 0x15ba.  

Done:
* Bios loading
* Game rom loading
* Gba memory mapping
* Cpu timings

Not yet supported:  
* Audio 
* Dma
* Keypad inputs
* Serial communication
* Timers

In progress:
* Arm/Thumb instruction sets (~80% done)
* GamePak 
* Video
* Cpu interrupts