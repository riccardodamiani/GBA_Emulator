
#include "thumb_decoder.h"

THUMB_opcode ThumbDecoder::decode(uint16_t opcode) {
	uint8_t code = (opcode >> 13) & 0b111;
	switch (code) {
	case 0:
		//move shifted register
		//add/subtract
		return THUMB_OP_INVALID;
		break;
	case 1:
		//move/compare/add/subtract immidiate
		return THUMB_OP_INVALID;
		break;
	case 2:
		//alu operation
		//hi reg operation/branch exchange
		//load pc relative
		//load/store with register offset
		//load store sign-extended byte/halfword
		return THUMB_OP_INVALID;
		break;
	case 3:
		//load store with immidiate offset
		return THUMB_OP_INVALID;
		break;
	case 4:
		//load store halfword
		//load/store sp-relative
		return THUMB_OP_INVALID;
		break;
	case 5:
		//get relative address
		//add offset stack pointer
		//push pop
		return THUMB_OP_INVALID;
		break;
	case 6:
		//multiple load/store
		//conditional branch
		//software interrupt
		return THUMB_OP_INVALID;
		break;
	case 7:
		//unconditional branch
		//long branch with link
		return THUMB_OP_INVALID;
		break;
	}
	return THUMB_OP_INVALID;
}

THUMB_opcode ThumbDecoder::decode_0(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_1(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_2(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_3(uint16_t opcode) {

}


THUMB_opcode ThumbDecoder::decode_4(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_5(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_6(uint16_t opcode) {

}

THUMB_opcode ThumbDecoder::decode_7(uint16_t opcode) {

}