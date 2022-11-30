#ifndef CPU_H
#define CPU_H

#include <cstdint>

enum ARM_opcode {
	ARM_OP_INVALID,
	ARM_OP_B,
	ARM_OP_BL,
	ARM_OP_AND,
	ARM_OP_EOR,
	ARM_OP_SUB,
	ARM_OP_RSB,
	ARM_OP_ADD,
	ARM_OP_ADC,
	ARM_OP_SBC,
	ARM_OP_RSC,
	ARM_OP_TST,
	ARM_OP_TEQ,
	ARM_OP_CMP,
	ARM_OP_CMN,
	ARM_OP_ORR,
	ARM_OP_MOV,
	ARM_OP_BIC,
	ARM_OP_MVN
};

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

	inline uint8_t leftRotate(uint8_t n, int bits);
	inline uint16_t leftRotate(uint16_t n, int bits);
	inline uint32_t leftRotate(uint32_t n, int bits);
	inline uint8_t rightRotate(uint8_t n, int bits);
	inline uint16_t rightRotate(uint16_t n, int bits);
	inline uint32_t rightRotate(uint32_t n, int bits);
	inline uint32_t arithmRight(uint32_t n, int bits);


	void next_instruction();
	void next_instruction_thumb();
	void next_instruction_arm();

	//ARM instructions
	void execute_arm(ARM_opcode instruction, uint32_t opcode);
	bool arm_checkInstructionCondition(uint32_t opcode);
	//decoding
	ARM_opcode decode_arm(uint32_t opcode);
	ARM_opcode ARM_IsBranch(uint32_t opcode);
	ARM_opcode ARM_IsAluInst(uint32_t opcode);

	//implementation
	inline void Arm_B(uint32_t opcode);
	inline void Arm_BL(uint32_t opcode);
	inline void Arm_CMP(uint32_t opcode);
	inline void Arm_MOV(uint32_t opcode);

};

#endif
