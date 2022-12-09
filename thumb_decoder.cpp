
#include "thumb_decoder.h"

THUMB_opcode ThumbDecoder::decode(uint16_t opcode) {
	uint8_t code = (opcode >> 13) & 0b111;
	switch (code) {
	case 0:
		//move shifted register
		//add/subtract
		return decode_0(opcode);
		break;
	case 1:
		//move/compare/add/subtract immidiate
		return decode_1(opcode);
		break;
	case 2:
		//alu operation
		//hi reg operation/branch exchange
		//load pc relative
		//load/store with register offset
		//load store sign-extended byte/halfword
		return decode_2(opcode);
		break;
	case 3:
		//load store with immidiate offset
		return decode_3(opcode);
		break;
	case 4:
		//load store halfword
		//load/store sp-relative
		return decode_4(opcode);
		break;
	case 5:
		//get relative address
		//add offset stack pointer
		//push pop
		return decode_5(opcode);
		break;
	case 6:
		//multiple load/store
		//conditional branch
		//software interrupt
		return decode_6(opcode);
		break;
	case 7:
		//unconditional branch
		//long branch with link
		return decode_7(opcode);
		break;
	}
	return THUMB_OP_INVALID;
}

//move shifted register
//add/subtract
THUMB_opcode ThumbDecoder::decode_0(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//move/compare/add/subtract immidiate
THUMB_opcode ThumbDecoder::decode_1(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//alu operation
//hi reg operation/branch exchange
//load pc relative
//load/store with register offset
//load store sign-extended byte/halfword
THUMB_opcode ThumbDecoder::decode_2(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//load store with immidiate offset
THUMB_opcode ThumbDecoder::decode_3(uint16_t opcode) {
	return THUMB_OP_INVALID;
}


//load store halfword
//load/store sp-relative
THUMB_opcode ThumbDecoder::decode_4(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//get relative address
//add offset stack pointer
//push pop
THUMB_opcode ThumbDecoder::decode_5(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//multiple load/store
//conditional branch
//software interrupt
THUMB_opcode ThumbDecoder::decode_6(uint16_t opcode) {
	return THUMB_OP_INVALID;
}

//unconditional branch
//long branch with link
THUMB_opcode ThumbDecoder::decode_7(uint16_t opcode) {
	return THUMB_OP_INVALID;
}