#ifndef THUMB_DECODER_H
#define THUMB_DECODER_H

#include "cpu.h"

class ThumbDecoder {
public:
	static THUMB_opcode decode(uint16_t opcode);
private:
	static THUMB_opcode decode_0(uint16_t opcode);
	static THUMB_opcode decode_1(uint16_t opcode);
	static THUMB_opcode decode_2(uint16_t opcode);
	static THUMB_opcode decode_3(uint16_t opcode);
	static THUMB_opcode decode_4(uint16_t opcode);
	static THUMB_opcode decode_5(uint16_t opcode);
	static THUMB_opcode decode_6(uint16_t opcode);
	static THUMB_opcode decode_7(uint16_t opcode);
};
#endif
