# Gameboy Advance Emulator  
Open source gba emulator in C++.

## Progress  
The emulator is not able to execute the game yet and will probably freeze somewhere in the bios because of some instructions not yet implemented.  
The execution of the bios is tested and works great until address 0x1004 in the startup phase.  
  
Done:
* Bios loading
* Game rom loading

Not yet supported:  
* Video
* Audio 
* Cpu interrupts
* Cpu timings
* Dma
* Keypad inputs
* Serial communication
* Timers

In progress:
* Arm/Thumb instruction sets (~65% done)
* Gba memory mapping
* GamePak 