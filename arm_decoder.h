#ifndef ARM_DECODER_H
#define ARM_DECODER_H

#include "cpu.h"

class ArmDecoder {
public:
	static ARM_opcode decode(uint32_t opcode);
private:
	static ARM_opcode ARM_IsBranch(uint32_t opcode);	//branches
	static ARM_opcode ARM_IsAluInst(uint32_t opcode);	//data processing
	static ARM_opcode ARM_IsSDTInst(uint32_t opcode);	//single data transfer
	static ARM_opcode ARM_IsMSR_MRS(uint32_t opcode);	//MSR/MRS
	static ARM_opcode ARM_IsBlockDataTransfer(uint32_t opcode);
private:
};

#endif
