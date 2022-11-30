#include "cpu.h"
#include "gba.h"
#include "clock.h"

#include <cstdint>
#include <iostream>

Clock GBA::clock;

Cpu::Cpu() {
	reg = {};
}

void Cpu::runFor(uint32_t ticks) {
	unsigned long long startingTicks = GBA::clock.getTicks();

	unsigned long long endingTicks = startingTicks + ticks;

	while (GBA::clock.getTicks() < endingTicks) {
		next_instruction();
	}
}

void Cpu::next_instruction_arm() {

	uint32_t opcode = GBA::memory.read_32(reg.R15);

	ARM_opcode instruction = decode_arm(opcode);
	execute_arm(instruction, opcode);
}

void Cpu::next_instruction_thumb() {

}

//execute the next instruction
void Cpu::next_instruction() {
	CPSR_registers* status = (CPSR_registers *)&reg.CPSR;

	if (status->T) {	//thumb
		next_instruction_thumb();
		return;
	}

	next_instruction_arm();
}

ARM_opcode Cpu::decode_arm(uint32_t opcode) {

	ARM_opcode instr;

	if (instr = ARM_IsBranch(opcode)) return instr;
	if (instr = ARM_IsAluInst(opcode)) return instr;

	return ARM_opcode::ARM_OP_INVALID;
}

bool Cpu::arm_checkInstructionCondition(uint32_t opcode) {
	uint8_t condition = (opcode >> 28) & 0x0f;
	CPSR_registers* flag = (CPSR_registers*)&reg.CPSR;

	bool condition_met = false;

	switch (condition) {
	case 0:	//EQ (z = 1)
		condition_met = flag->Z == 1;
		break;
	case 1:		//NE (z = 0)
		condition_met = flag->Z == 0;
		break;
	case 2:		//CS/HS (c = 1)
		condition_met = flag->C == 1;
		break;
	case 3:		//CC/LO	(c = 0)
		condition_met = flag->C == 0;
		break;
	case 4:		//MI (N = 1) negative
		condition_met = flag->N == 1;
		break;
	case 5:		//PL (N = 0) >= 0
		condition_met = flag->N == 0;
		break;
	case 6:		//VS (V = 1) overflow
		condition_met = flag->V == 1;
		break;
	case 7:		//VC (V = 0) no overflow
		condition_met = flag->V == 0;
		break;
	case 8:		//HI (C = 1 or Z = 0)	unsigned higher
		condition_met = (flag->C == 1) || (flag->Z == 0);
		break;
	case 9:		//LS (C = 0 or Z=1)	unsigned lower or same
		condition_met = (flag->C == 0) || (flag->Z == 1);
		break;
	case 0xa:	//GE (N = V) signed greater or equal
		condition_met = flag->N == flag->V;
		break;
	case 0xb:	//LT (N!=V)	signed less than
		condition_met = flag->N != flag->V;
		break;
	case 0xc:	//GT (Z=0 and N=V)	signed greater than
		condition_met = (flag->Z == 0) && (flag->N == flag->V);
		break;
	case 0xd:	//LE (Z=1 or N!=V)	signed less or equal
		condition_met = (flag->Z == 1) || (flag->N != flag->V);
		break;
	case 0xe:	//AL always
		condition_met = true;
		break;
	case 0xf:	//NV never
		condition_met = false;
		break;
	}

	return condition_met;
}

void Cpu::execute_arm(ARM_opcode instruction, uint32_t opcode) {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	if (!arm_checkInstructionCondition(opcode)) {	//doesn't meet the condition
		reg.R15 += 4;
		//GBA::clock.addTicks(1);
		return;
	}

	switch (instruction) {
	case ARM_OP_B:	//branch
		Arm_B(opcode);
		break;

	case ARM_OP_BL:		//branch with link
		Arm_BL(opcode);
		break;

	case ARM_OP_CMP:
		Arm_CMP(opcode);
		reg.R15 += 4;
		break;

	case ARM_OP_MOV:
		Arm_MOV(opcode);
		reg.R15 += 4;
		break;

	default:
		std::cout << "!! Instruction not implemented: " << std::hex 
			<< "op " << opcode << ", instruction " << instruction << std::endl;
		system("pause");
		exit(1);
		break;
	}
}

/*void Cpu::execute_thumb() {
	CPSR_registers* status = (CPSR_registers*)&reg.CPSR;

	//align R15
	reg.R15 -= reg.R15 % 2;

	uint16_t instruction = GBA::memory.read_16(reg.R15);

	reg.R15 += 2;
}*/

int32_t convert_24Bit_to_32Bit_signed(uint32_t val) {

	if (val & 0x800000) {
		return static_cast<int32_t>(val | 0xff000000);
	}
	return static_cast<int32_t>(val & 0xffffff);
}

ARM_opcode Cpu::ARM_IsBranch(uint32_t opcode) {
	
	uint32_t branch_format = 0b0000'1010'0000'0000'0000'0000'0000'0000;
	uint32_t branch_with_link_format = 0b0000'1011'0000'0000'0000'0000'0000'0000;
	uint32_t mask = 0b0000'1111'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	//branch
	if (branch_format == opcode_format)
		return ARM_OP_B;

	//branch with link
	if (branch_with_link_format == opcode_format) {
		return ARM_OP_BL;
	}

	return ARM_OP_INVALID;
}

ARM_opcode Cpu::ARM_IsAluInst(uint32_t opcode) {
	uint32_t mask =		0b0000'1100'0000'0000'0000'0000'0000'0000;
	uint32_t format =	0b0000'0000'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if(format != opcode_format)	//not alu instruction
		return ARM_OP_INVALID;

	switch ((opcode >> 21) & 0x0f) {
	case 0:	//and
		return ARM_OP_AND;
		break;
	case 1:	//xor
		return ARM_OP_EOR;
		break;
	case 2:	//subtract
		return ARM_OP_SUB;
		break;
	case 3:	//subtract reversed
		return ARM_OP_RSB;
		break;
	case 4:	//add
		return ARM_OP_ADD;
		break;
	case 5:	//add with carry
		return ARM_OP_ADC;
		break;
	case 6:	//subtract with carry
		return ARM_OP_SBC;
		break;
	case 7:	//sub cy. reversed
		return ARM_OP_RSC;
		break;
	case 8:	//test
		return ARM_OP_TST;
		break;
	case 9:	//test exclusive
		return ARM_OP_TEQ;
		break;
	case 0xa:	//compare
		return ARM_OP_CMP;
		break;
	case 0xb:	//compare negative
		return ARM_OP_CMN;
		break;
	case 0xc:	//or
		return ARM_OP_ORR;
		break;
	case 0xd:	//move
		return ARM_OP_MOV;
		break;
	case 0xe:	//bit clear
		return ARM_OP_BIC;
		break;
	case 0xf:	//not
		return ARM_OP_MVN;
		break;
	}

	return ARM_OP_INVALID;

}

inline uint8_t Cpu::leftRotate(uint8_t n, int bits) {
	return (n << bits) | (n >> (8 - bits));
}

inline uint16_t Cpu::leftRotate(uint16_t n, int bits) {
	return (n << bits) | (n >> (16 - bits));
}

inline uint32_t Cpu::leftRotate(uint32_t n, int bits) {
	return (n << bits) | (n >> (32 - bits));
}

inline uint8_t Cpu::rightRotate(uint8_t n, int bits) {
	return (n >> bits) | (n << (8 - bits));
}

inline uint16_t Cpu::rightRotate(uint16_t n, int bits) {
	return (n >> bits) | (n << (16 - bits));
}

inline uint32_t Cpu::rightRotate(uint32_t n, int bits) {
	return (n >> bits) | (n << (32 - bits));
}

inline uint32_t Cpu::arithmRight(uint32_t n, int bits) {
	if (n & 0x80000000) {
		return n >> bits;
	}

	uint64_t ones = ((uint64_t)1 << bits) - 1;
	ones <<= (32 - bits);
	return (n >> bits) | (uint32_t)ones;
}


//branch
inline void Cpu::Arm_B(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R15 += 8 + offset * 4;
}

//branch with link
inline void Cpu::Arm_BL(uint32_t opcode) {
	uint32_t offset_24Bit = opcode & 0xffffff;
	int32_t offset = convert_24Bit_to_32Bit_signed(offset_24Bit);
	reg.R15 += 8 + offset * 4;
	reg.R14 = reg.R15 + 4;
}

inline void Cpu::Arm_CMP(uint32_t opcode) {
	uint8_t I = (opcode >> 25) & 1;
	uint8_t reg_1_code = (opcode >> 16) & 0x0f;

	uint32_t *Rn = &((uint32_t*)&reg)[reg_1_code];
	uint32_t val = 0;
	if (I) {	//immidiate 2nd operand 
		uint8_t Is = (opcode >> 8) & 0x0f;
		uint32_t nn = opcode & 0xff;
		val = rightRotate(nn, Is * 2);
	}
	else {
		uint8_t Is;
		if (opcode & 0x10) {	//bit 4 set: shift amount taken from a register
			uint8_t shift_reg_code = (opcode >> 8) & 0x0f;
			uint32_t* Rs = &((uint32_t*)&reg)[shift_reg_code];
			Is = *Rs & 0xff;
		}
		else { //bit 4 clear: immidiate shift amount 
			Is = (opcode >> 7) & 0x1f;
		}
		uint8_t operand_reg_code = opcode & 0x0f;
		uint32_t* Rm = &((uint32_t*)&reg)[operand_reg_code];
		uint8_t ST = (opcode >> 5) & 0x3;
		switch (ST) {
		case 0:		//logical left
			val = *Rm << Is;
			break;

		case 1:		//logical right
			val = *Rm >> Is;
			break;

		case 2:		//arithmetic right
			val = arithmRight(*Rm, Is);
			break;

		case 3:		//rotate right
			val = rightRotate(*Rm, Is);
			break;
		}
	}

	CPSR_registers* flag = (CPSR_registers*)&reg.CPSR;
	uint32_t result = *Rn - val;
	flag->Z = result == 0;
	flag->N = (result & 0x80000000) != 0;	//negative
	flag->C = !(*Rn < val);	//carry = !borrow
	flag->V = (((*Rn | val) ^ result) >> 31) & 1;	//overflow

}

inline void Cpu::Arm_MOV(uint32_t opcode) {
	CPSR_registers* flag = (CPSR_registers*)&reg.CPSR;

	uint8_t I = (opcode >> 25) & 1;
	uint8_t S = (opcode >> 20) & 1;	//set condition code
	uint8_t dest_reg_code = (opcode >> 12) & 0x0f;

	uint32_t* Rd = &((uint32_t*)&reg)[dest_reg_code];
	uint32_t val = 0;
	uint8_t c = flag->C;

	if (I) {	//immidiate 2nd operand 
		uint8_t Is = (opcode >> 8) & 0x0f;
		uint32_t nn = opcode & 0xff;
		val = rightRotate(nn, Is * 2);
	}
	else {
		uint8_t Is;

		uint8_t operand_reg_code = opcode & 0x0f;
		uint32_t* Rm = &((uint32_t*)&reg)[operand_reg_code];
		uint32_t Rm_value = 0;

		if (opcode & 0x10) {	//bit 4 set: shift amount taken from a register
			uint8_t shift_reg_code = (opcode >> 8) & 0x0f;
			uint32_t* Rs = &((uint32_t*)&reg)[shift_reg_code];
			Is = *Rs & 0xff;
			if(operand_reg_code == 0xf) Rm_value = 12;
		}
		else { //bit 4 clear: immidiate shift amount 
			Is = (opcode >> 7) & 0x1f;
			if (operand_reg_code == 0xf) Rm_value = 8;
		}
		
		Rm_value += *Rm;

		uint8_t ST = (opcode >> 5) & 0x3;
		if (Is) {	//there is a shift
			GBA::clock.addTicks(1);
			switch (ST) {
			case 0:		//logical left
				c = (Rm_value >> (32 - Is)) & 1;	//carry = last lost bit
				val = Rm_value << Is;
				break;

			case 1:		//logical right
				c = (Rm_value >> (Is - 1)) & 1;	//carry = last lost bit
				val = Rm_value >> Is;
				break;

			case 2:		//arithmetic right
				c = (Rm_value >> (Is - 1)) & 1;	//carry = last lost bit
				val = arithmRight(Rm_value, Is);
				break;

			case 3:		//rotate right
				c = (Rm_value >> (Is - 1)) & 1;	//carry = last rotated bit
				val = rightRotate(Rm_value, Is);
				break;
			}
		}
		else {	//no shift
			val = *Rm;
		}
	}
	*Rd = val;
	
	if (S & (dest_reg_code != 0xf)) {	//flags
		flag->Z = val == 0;
		flag->N = (val & 0x80000000) != 0;	//negative
		flag->C = c;
	}
}
