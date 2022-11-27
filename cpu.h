#ifndef CPU_H
#define CPU_H

#include <cstdint>

struct CPSR_registers {
	uint8_t mode : 5,	//M0-M4 modes
		T : 1,		//state bit (0 = ARM, 1 = THUMB)
		F : 1,		//FIQ disable (0 = Enable, 1 = Disable)
		I : 1;		//IRQ disable (0 = enable, 1 = disable)
	uint16_t A : 1,		//abort disable (0 = enable, 1 = disable)
		E : 1,		//endian
		reserved : 14;
	uint8_t J : 1,		//Jazelle Mode    (1=Jazelle Bytecode instructions) (if supported)
		reserved2 : 2,
		Q : 1,	//sticky overflow (not supported)
		V : 1,		//Overflow Flag   (0=No Overflow, 1=Overflow)
		C : 1,		//Carry Flag      (0=Borrow/No Carry, 1=Carry/No Borrow)
		Z : 1,		//Z - Zero Flag       (0=Not Zero, 1=Zero)
		N : 1;		//N - Sign Flag       (0=Not Signed, 1=Signed)
};

struct Registers {
	//active registers
	uint32_t R0, R1, R2, R3, R4, R5, R6, R7;
	uint32_t R8, R9, R10, R11, R12, R13, R14, R15;
	uint32_t CPSR, SPSR;
	//standard banked registers
	uint32_t R8_std, R9_std, R10_std, R11_std, R12_std, R13_std, R14_std;
	//fiq banked registers
	uint32_t R8_fiq, R9_fiq, R10_fiq, R11_fiq, R12_fiq, R13_fiq, R14_fiq, SPSR_fiq;
	//supervisor banked registers
	uint32_t R13_svc, R14_svc, SPSR_svc;
	//abort banked registers
	uint32_t R13_abt, R14_abt, SPSR_abt;
	//irq banked registers
	uint32_t R13_irq, R14_irq, SPSR_irq;
	//undefined banked registers
	uint32_t R13_und, R14_und, SPSR_und;
};

class Cpu {
public:
	Cpu();
	void runFor(uint32_t ticks);
private:
	Registers reg;

	void next_instruction();
	void execute();
	void execute_arm();
	void execute_thumb();

	bool isBranchOrBranchWithLink(uint32_t opcode);

};

#endif
