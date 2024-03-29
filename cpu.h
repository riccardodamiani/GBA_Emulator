#ifndef CPU_H
#define CPU_H

#include "interrupt.h"

#include <cstdint>

enum ARM_opcode {
	ARM_OP_INVALID,
	ARM_OP_B,	//branches
	ARM_OP_BL,
	ARM_OP_BX,
	ARM_OP_AND,	//alu operations
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
	ARM_OP_MVN,
	ARM_OP_LDRH,	//load/store halfword
	ARM_OP_LDRSB,
	ARM_OP_LDRSH,
	ARM_OP_STRH,
	ARM_OP_STM,		//load/store multiple
	ARM_OP_LDM,
	ARM_OP_LDR,		//load/store
	ARM_OP_STR,
	ARM_OP_MSR,
	ARM_OP_MRS,
	ARM_OP_MUL,
	ARM_OP_MLA,
	ARM_OP_MULL,	//multiply long
	ARM_OP_MLAL,		//multiply-accumulate long
	ARM_OP_UMULL,	//unsigned multiply long
	ARM_OP_UMLAL		//unsigned multiply-accumulate long
};

enum THUMB_opcode {
	THUMB_OP_INVALID,
	THUMB_OP_UNDEFINED,
	THUMB_OP_LSL_IMM,	//logical/arithm shift left
	THUMB_OP_LSR_IMM,	//logical right shift
	THUMB_OP_ASR_IMM,	//arithmetic shift right
	THUMB_OP_MOV_I,	//mov/cmp/add/sub immidiate
	THUMB_OP_CMP_I,
	THUMB_OP_ADD_I,
	THUMB_OP_SUB_I,	
	THUMB_OP_LDR_PC,	//load pc-relative
	THUMB_OP_STR_O,	//store register offset
	THUMB_OP_STRB_O,
	THUMB_OP_LDR_O,	//load register offset
	THUMB_OP_LDRB_O,
	//Alu operations
	THUMB_OP_AND,
	THUMB_OP_EOR,
	THUMB_OP_LSL,
	THUMB_OP_LSR,
	THUMB_OP_ASR,
	THUMB_OP_ADC,
	THUMB_OP_SBC,
	THUMB_OP_ROR,
	THUMB_OP_TST,	//TST
	THUMB_OP_NEG,
	THUMB_OP_CMP,
	THUMB_OP_CMN,
	THUMB_OP_ORR,
	THUMB_OP_MUL,
	THUMB_OP_BIC,
	THUMB_OP_MVN,	//MVN

	THUMB_OP_ADD_RR,	//add register-register
	THUMB_OP_SUB_RR,	//sub register-register
	THUMB_OP_ADD_RI,	//add register-immidiate
	THUMB_OP_SUB_RI,	//sub register-immidiate
	THUMB_OP_BEQ,	//conditional branches
	THUMB_OP_BNE,
	THUMB_OP_BCS,	
	THUMB_OP_BCC,
	THUMB_OP_BMI,
	THUMB_OP_BPL,
	THUMB_OP_BVS,
	THUMB_OP_BVC,
	THUMB_OP_BHI,
	THUMB_OP_BLS,
	THUMB_OP_BGE,
	THUMB_OP_BLT,
	THUMB_OP_BGT,
	THUMB_OP_BLE,
	THUMB_OP_SWI,	//software interrupt
	THUMB_OP_ADD_HRR,	//high register add
	THUMB_OP_CMP_HRR,	//high register compare
	THUMB_OP_MOV_HRR,	//high register move
	THUMB_OP_NOP,	//nop (MOV r8, r8)
	THUMB_OP_BX,	//branch exchange
	THUMB_OP_PUSH,	//push
	THUMB_OP_POP,	//pop
	THUMB_OP_LDMIA,	//load multiple 
	THUMB_OP_STMIA,	//store multiple
	THUMB_OP_ADD_SP,	//add offset to sp
	THUMB_OP_SUB_SP,		//sub offset to sp
	THUMB_OP_ADD_R_SP,	//get relative address from sp
	THUMB_OP_ADD_R_PC,	//get relative address from pc
	THUMB_OP_STR_SP,	//store sp-relative
	THUMB_OP_LDR_SP,	//load sp-relative
	THUMB_OP_LDRH,	//load halfword
	THUMB_OP_STRH,	//store halfword
	THUMB_OP_B,	//unconditional branch
	THUMB_OP_BL_F,	//long branch with link
	THUMB_OP_BL_LR_IMM,
	THUMB_OP_STR_I,	//load/store immidiate offset
	THUMB_OP_LDR_I,
	THUMB_OP_STRB_I,
	THUMB_OP_LDRB_I,

	//THUMB.8: load store sign-extended byte halfword
	THUMB_OP_STRH_R,	
	THUMB_OP_LDSB_R,
	THUMB_OP_LDRH_R,
	THUMB_OP_LDSH_R
};

struct CPSR_registers {
	uint32_t mode : 5,	//M0-M4 modes
		T : 1,		//state bit (0 = ARM, 1 = THUMB)
		F : 1,		//FIQ disable (0 = Enable, 1 = Disable)
		I : 1,		//IRQ disable (0 = enable, 1 = disable)
		A : 1,		//abort disable (0 = enable, 1 = disable)
		E : 1,		//endian
		reserved : 14,
		J : 1,		//Jazelle Mode    (1=Jazelle Bytecode instructions) (if supported)
		reserved2 : 2,
		Q : 1,	//sticky overflow (not supported)
		V : 1,		//Overflow Flag   (0=No Overflow, 1=Overflow)
		C : 1,		//Carry Flag      (0=Borrow/No Carry, 1=Carry/No Borrow)
		Z : 1,		//Z - Zero Flag       (0=Not Zero, 1=Zero)
		N : 1;		//N - Sign Flag       (0=Not Signed, 1=Signed)
};

enum PrivilegeMode {
	USER = 0x10,
	FIQ = 0x11,
	IRQ = 0x12,
	SUPERVISOR = 0x13,
	ABORT = 0x17,
	UNDEFINED = 0x1b,
	SYSTEM = 0x1f	
};

struct Registers {
	//active registers
	uint32_t R0, R1, R2, R3, R4, R5, R6, R7;
	uint32_t R8, R9, R10, R11, R12, R13, R14, R15;
	uint32_t CPSR, SPSR;
	CPSR_registers *CPSR_f;
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
	uint32_t getPC();

	void RaiseIRQ(Interrupt_Type type);
private:
	Registers reg;
	uint8_t shifter_carry_out;

	int32_t convert_24Bit_to_32Bit_signed(uint32_t val);

	//inline uint32_t shifterLeftRotate(uint32_t n, int bits);
	inline uint32_t shifterRightRotate(uint32_t n, int bits);
	inline uint32_t rightRotate(uint32_t n, int bits);
	inline uint32_t arithmRight(uint32_t n, int bits);


	void next_instruction();
	void next_instruction_thumb();
	void next_instruction_arm();

	void Reset();
	
	void RaiseFIQ();
	void RaiseUndefined();
	void RaiseSWI();

	void setPrivilegeMode(PrivilegeMode mode);
	void setPrivilegeMode(PrivilegeMode currentMode, PrivilegeMode mode);
	void saveBankReg(PrivilegeMode currentMode);
	void getBankReg(PrivilegeMode newMode);

	//THUMB instructions
	void execute_thumb(THUMB_opcode instruction, uint16_t opcode);
	bool thumbCheckCondition(uint16_t opcode);

	//THUMB.1
	inline void Thumb_LSL_IMM(uint16_t opcode);
	inline void Thumb_LSR_IMM(uint16_t opcode);
	inline void Thumb_ASR_IMM(uint16_t opcode);

	//THUMB.2
	inline void Thumb_ADD_RR(uint16_t opcode);
	inline void Thumb_SUB_RR(uint16_t opcode);
	inline void Thumb_ADD_RI(uint16_t opcode);
	inline void Thumb_SUB_RI(uint16_t opcode);

	//THUMB.3
	inline void Thumb_MOV_I(uint16_t opcode);
	inline void Thumb_SUB_I(uint16_t opcode);
	inline void Thumb_ADD_I(uint16_t opcode);
	inline void Thumb_CMP_I(uint16_t opcode);

	//THUMB.4
	inline void Thumb_AND(uint16_t opcode);
	inline void Thumb_LSR(uint16_t opcode);
	inline void Thumb_LSL(uint16_t opcode);
	inline void Thumb_ASR(uint16_t opcode);
	inline void Thumb_TST(uint16_t opcode);
	inline void Thumb_NEG(uint16_t opcode);
	inline void Thumb_MVN(uint16_t opcode);
	inline void Thumb_ORR(uint16_t opcode);
	inline void Thumb_CMP(uint16_t opcode);
	inline void Thumb_CMN(uint16_t opcode);
	inline void Thumb_ROR(uint16_t opcode);
	inline void Thumb_EOR(uint16_t opcode);
	inline void Thumb_MUL(uint16_t opcode);
	inline void Thumb_BIC(uint16_t opcode);

	//THUMB.5
	inline void Thumb_BX(uint16_t opcode);
	inline void Thumb_MOV_HRR(uint16_t opcode);
	inline void Thumb_ADD_HRR(uint16_t opcode);
	inline void Thumb_CMP_HRR(uint16_t opcode);

	//THUMB.6
	inline void Thumb_LDR_PC(uint16_t opcode);

	//THUMB.7
	inline void Thumb_STR_O(uint16_t opcode);
	inline void Thumb_LDR_O(uint16_t opcode);
	inline void Thumb_LDRB_O(uint16_t opcode);

	//THUMB.8
	inline void Thumb_LDRH_R(uint16_t opcode);
	inline void Thumb_STRH_R(uint16_t opcode);
	inline void Thumb_LDRSH(uint16_t opcode);
	inline void Thumb_LDRSB(uint16_t opcode);

	//THUMB.9
	inline void Thumb_STR_I(uint16_t opcode);
	inline void Thumb_LDR_I(uint16_t opcode);
	inline void Thumb_STRB_I(uint16_t opcode);
	inline void Thumb_LDRB_I(uint16_t opcode);

	//THUMB.10
	inline void Thumb_STRH_I(uint16_t opcode);
	inline void Thumb_LDRH_I(uint16_t opcode);

	//THUMB.11
	inline void Thumb_LDR_SP(uint16_t opcode);
	inline void Thumb_STR_SP(uint16_t opcode);

	//THUMB.12
	inline void Thumb_ADD_R_SP(uint16_t opcode);
	inline void Thumb_ADD_R_PC(uint16_t opcode);

	//THUMB.13
	inline void Thumb_SUB_SP(uint16_t opcode);
	inline void Thumb_ADD_SP(uint16_t opcode);

	//THUMB.14
	inline void Thumb_PUSH(uint16_t opcode);
	inline void Thumb_POP(uint16_t opcode);
	
	//THUMB.15
	inline void Thumb_LDMIA(uint16_t opcode);
	inline void Thumb_STMIA(uint16_t opcode);

	//THUMB.17
	inline void Thumb_SWI(uint16_t opcode);

	//THUMB.18
	inline void Thumb_B(uint16_t opcode);

	//THUMB.19
	inline void Thumb_BL_1(uint16_t opcode);
	inline void Thumb_BL_2(uint16_t opcode);

	//ARM instructions
	void execute_arm(ARM_opcode instruction, uint32_t opcode);
	bool arm_checkInstructionCondition(uint32_t opcode);

	inline void ARM_Shifter(uint8_t shiftType, uint8_t shift_amount, uint32_t val, uint32_t& result, bool enableSpecialShift);

	//branches implementation
	inline void Arm_B(uint32_t opcode);
	inline void Arm_BL(uint32_t opcode);
	inline void Arm_BX(uint32_t opcode);

	//ALU implementation
	inline void ARM_ALU_unpacker(uint32_t opcode, uint32_t **destReg, uint32_t& oper1, uint32_t& oper2, uint8_t& s);
	inline void ARM_ALU_oper2_getter(uint32_t opcode, uint32_t &oper2);
	inline void Arm_AND(uint32_t opcode);
	inline void Arm_EOR(uint32_t opcode);
	inline void Arm_SUB(uint32_t opcode);
	inline void Arm_RSB(uint32_t opcode);
	inline void Arm_ADD(uint32_t opcode);
	inline void Arm_ADC(uint32_t opcode);
	inline void Arm_TST(uint32_t opcode);
	inline void Arm_TEQ(uint32_t opcode);
	inline void Arm_CMP(uint32_t opcode);
	inline void Arm_ORR(uint32_t opcode);
	inline void Arm_MOV(uint32_t opcode);
	inline void Arm_BIC(uint32_t opcode);	

	inline void Arm_MSR(uint32_t opcode);
	inline void Arm_MRS(uint32_t opcode);

	//halfword transfer
	inline void ARM_SDTH_unpacker(uint32_t opcode, uint32_t& address, uint32_t** src_dest_reg, uint32_t &R15_as_src_correction);
	inline void Arm_LDRH(uint32_t opcode);
	inline void Arm_STRH(uint32_t opcode);
	inline void Arm_LDRSH(uint32_t opcode);
	inline void Arm_LDRSB(uint32_t opcode);

	//single transfer implementations
	inline void ARM_SDT_unpacker(uint32_t opcode, uint32_t &address, uint32_t** src_dest_reg, uint8_t& b,
		uint32_t& R15_as_src_correction);
	inline void Arm_LDR(uint32_t opcode);
	inline void Arm_STR(uint32_t opcode);

	//multiplications
	inline void Arm_MUL(uint32_t opcode);

	//multiply long
	inline void Arm_MULL(uint32_t opcode);

	//block data transfer
	inline void Arm_STM(uint32_t opcode);
	inline void Arm_STM_DEC(uint8_t paramP, uint16_t reg_list, uint32_t& address);
	inline void Arm_STM_INC(uint8_t paramP, uint16_t reg_list, uint32_t& address);
	inline void Arm_LDM(uint32_t opcode);
	inline void Arm_LDM_DEC(uint8_t paramP, uint16_t reg_list, uint32_t& address);
	inline void Arm_LDM_INC(uint8_t paramP, uint16_t reg_list, uint32_t& address);
};

#endif
