#include "arm_decoder.h"

ARM_opcode ArmDecoder::decode(uint32_t opcode) {
	ARM_opcode instr;

	if (instr = ARM_IsBranch(opcode)) return instr;		//branches
	if (instr = ARM_IsSDTHInst(opcode)) return instr;	//load/store halfword
	if (instr = ARM_IsMultiplication(opcode)) return instr;
	if (instr = ARM_IsMull(opcode)) return instr;	//multiply long, multiply-accum long
	if (instr = ARM_IsAluInst(opcode)) return instr;	//data processing
	if (instr = ARM_IsSDTInst(opcode)) return instr;	//single store/load
	if (instr = ARM_IsBlockDataTransfer(opcode)) return instr;	//block data transfer (push/pop)

	return ARM_opcode::ARM_OP_INVALID;
}

ARM_opcode ArmDecoder::ARM_IsBranch(uint32_t opcode) {

	uint32_t branch_format = 0b0000'1010'0000'0000'0000'0000'0000'0000;
	uint32_t branch_with_link_format = 0b0000'1011'0000'0000'0000'0000'0000'0000;
	uint32_t brench_exchange_format = 0b0000'0001'0010'1111'1111'1111'0000'0000;

	uint32_t mask = 0b0000'1111'0000'0000'0000'0000'0000'0000;
	uint32_t brench_exchange_mask = 0b0000'1111'1111'1111'1111'1111'0000'0000;

	uint32_t opcode_format = opcode & mask;

	//branch
	if (branch_format == opcode_format)
		return ARM_OP_B;

	//branch with link
	if (branch_with_link_format == opcode_format) {
		return ARM_OP_BL;
	}

	opcode_format = opcode & brench_exchange_mask;
	if (brench_exchange_format == opcode_format)
		return ARM_OP_BX;

	return ARM_OP_INVALID;
}

//multiply long and multiply accumulate long
ARM_opcode ArmDecoder::ARM_IsMull(uint32_t opcode) {
	uint32_t mask =   0b0000'1111'1000'0000'0000'0000'1111'0000;
	uint32_t format = 0b0000'0000'1000'0000'0000'0000'1001'0000;

	uint32_t opcode_format = opcode & mask;
	if (format != opcode_format)	//not this instruction
		return ARM_OP_INVALID;

	if ((opcode >> 22) & 1) {
		return (opcode >> 21) & 1 ? ARM_OP_UMLAL : ARM_OP_UMULL;
	}
	return (opcode >> 21) & 1 ? ARM_OP_MLAL : ARM_OP_MULL;
}

ARM_opcode ArmDecoder::ARM_IsAluInst(uint32_t opcode) {
	uint32_t mask = 0b0000'1100'0000'0000'0000'0000'0000'0000;
	uint32_t format = 0b0000'0000'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if (format != opcode_format)	//not alu instruction
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
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_TST;
		break;
	}
	case 9:	//test exclusive
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_TEQ;
		break;
	}
	case 0xa:	//compare
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_CMP;
		break;
	}

	case 0xb:	//compare negative
	{
		ARM_opcode inst;
		if (inst = ARM_IsMSR_MRS(opcode)) return inst;	//check MSR, MRS instructions
		return ARM_OP_CMN;
		break;
	}
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

ARM_opcode ArmDecoder::ARM_IsMSR_MRS(uint32_t opcode) {
	uint32_t mrs_mask = 0b0000'1111'1011'1111'0000'1111'1111'1111;
	uint32_t msr_mask = 0b0000'1111'1011'1111'1111'1111'1111'0000;
	uint32_t msri_mask = 0b0000'1101'1011'1111'1111'0000'0000'0000;

	uint32_t mrs_format = 0b0000'0001'0000'1111'0000'0000'0000'0000;
	uint32_t msr_format = 0b0000'0001'0010'1001'1111'0000'0000'0000;
	uint32_t msri_format = 0b0000'0001'0010'1000'1111'0000'0000'0000;

	uint32_t opcode_format = opcode & mrs_mask;
	if (opcode_format == mrs_format) return ARM_OP_MRS;

	opcode_format = opcode & msr_mask;
	if (opcode_format == msr_format) return ARM_OP_MSR;

	opcode_format = opcode & msri_mask;
	if (opcode_format == msri_format) return ARM_OP_MSR;

	return ARM_OP_INVALID;

}


//halfword data transfer
ARM_opcode ArmDecoder::ARM_IsSDTHInst(uint32_t opcode) {
	uint32_t mask =		0b0000'1110'0000'0000'0000'0000'1001'0000;
	uint32_t format =	0b0000'0000'0000'0000'0000'0000'1001'0000;

	if ((opcode & mask) != format) {
		return ARM_OP_INVALID;
	}

	if ((opcode >> 20) & 1) {	//load from memory
		switch ((opcode >> 5) & 0b11) {
		case 1:
			return ARM_OP_LDRH;
			break;
		case 2:
			return ARM_OP_LDRSB;
			break;
		case 3:
			return ARM_OP_LDRSH;
			break;
		}
	}
	else {	//store to memory
		switch ((opcode >> 5) & 0b11) {
		case 1:
			return ARM_OP_STRH;
			break;
		}
	}

	return ARM_OP_INVALID;
}

ARM_opcode ArmDecoder::ARM_IsSDTInst(uint32_t opcode) {
	uint32_t mask = 0b0000'1100'0000'0000'0000'0000'0000'0000;
	uint32_t format = 0b0000'0100'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if (format != opcode_format)	//not signle data transfer instruction
		return ARM_OP_INVALID;

	if ((opcode >> 20) & 1) {	//20th bit = load/store bit
		return ARM_OP_LDR;
	}
	return ARM_OP_STR;
	
}

ARM_opcode ArmDecoder::ARM_IsBlockDataTransfer(uint32_t opcode) {
	uint32_t mask = 0b0000'1110'0000'0000'0000'0000'0000'0000;
	uint32_t format = 0b0000'1000'0000'0000'0000'0000'0000'0000;

	uint32_t opcode_format = opcode & mask;

	if (format != opcode_format)
		return ARM_OP_INVALID;

	if ((opcode >> 20) & 1) {	//20th bit = load/store bit
		return ARM_OP_LDM;
	}
	return ARM_OP_STM;
}

ARM_opcode ArmDecoder::ARM_IsMultiplication(uint32_t opcode) {
	uint32_t mask = 0b0000'1110'0000'0000'0000'0000'1111'0000;
	uint32_t format = 0b0000'0000'0000'0000'0000'0000'1001'0000;

	uint32_t opcode_format = opcode & mask;

	if (format != opcode_format)
		return ARM_OP_INVALID;

	switch ((opcode >> 21) & 0b1111) {
	case 0:
		return ARM_OP_MUL;
		break;
	case 1:
		return ARM_OP_MLA;
		break;
	}

	return ARM_OP_INVALID;
}