#include "dma.h"
#include "gba.h"

#include <cstdint>

Dma::Dma(uint8_t num) {
	_dmaNr = num;
	_srcAddr = 0;
	_dstAddr = 0;
	_transfLen = 0;
	_transfCounter = 0;
	_reload_on_repeat = 0;
	_fifoTransfer = false;

	switch (_dmaNr) {
	case 0:
		_cnt = (dma_control_struct*)GBA::memory.get_io_reg(0xBA);
		break;
	case 1:
		_cnt = (dma_control_struct*)GBA::memory.get_io_reg(0xc6);
		break;
	case 2:
		_cnt = (dma_control_struct*)GBA::memory.get_io_reg(0xd2);
		break;
	case 3:
		_cnt = (dma_control_struct*)GBA::memory.get_io_reg(0xdc);
		break;
	default:
		throw "Invalid DMA channel specified";
		break;
	}
}

//reload some stuff on repeat
void Dma::load_on_repeat(void) {
	if (!_reload_on_repeat)
		return;

	_fifoTransfer = false;

	switch (_dmaNr) {
	case 0:	//DMA0
	{
		//read destination address
		if (_cnt->dst_cnt == 3) {	//increment + reload
			_dstAddr = GBA::memory.read_32(0x40000B4);
			_dstAddr &= 0x7FFFFFF;	//internal memory only
		}
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000B8);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		break;
	}
	case 1:		//DMA1
	{
		//read destination address
		if (_cnt->dst_cnt == 3) {	//increment + reload
			_dstAddr = GBA::memory.read_32(0x40000c0);
			_dstAddr &= 0x7FFFFFF;	//internal memory only
		}
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000c4);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		_fifoTransfer = _cnt->repeat
			&& (_dstAddr == 0x40000A0 || _dstAddr == 0x40000A4);
		break;
	}
	case 2:		//DMA2
	{
		//read destination address
		if (_cnt->dst_cnt == 3) {	//increment + reload
			_dstAddr = GBA::memory.read_32(0x40000cc);
			_dstAddr &= 0x7FFFFFF;	//internal memory only
		}
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000d0);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		_fifoTransfer = _cnt->repeat
			&& (_dstAddr == 0x40000A0 || _dstAddr == 0x40000A4);
		break;
	}
	case 3:		//DMA3
	{
		//read destination address
		if (_cnt->dst_cnt == 3) {	//increment + reload
			_dstAddr = GBA::memory.read_32(0x40000d8);
			_dstAddr &= 0xFFFFFFF;		//any memory
		}
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000dc);	//16 bit
		if (_transfLen == 0) _transfLen = 0x10000;
		_transfCounter = 0;
		break;
	}
	}
}

void Dma::enable_dma(void) {
	_fifoTransfer = false;

	switch (_dmaNr) {
	case 0:	//DMA0
	{
		//read source address
		_srcAddr = GBA::memory.read_32(0x40000B0);
		_srcAddr &= 0x7FFFFFF;	//internal memory only
		//read destination address
		_dstAddr = GBA::memory.read_32(0x40000B4);
		_dstAddr &= 0x7FFFFFF;	//internal memory only
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000B8);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		_reload_on_repeat = 0;
		break;
	}
	case 1:		//DMA1
	{
		//read source address
		_srcAddr = GBA::memory.read_32(0x40000bc);
		_srcAddr &= 0xFFFFFFF;	//any memory
		//read destination address
		_dstAddr = GBA::memory.read_32(0x40000c0);
		_dstAddr &= 0x7FFFFFF;	//internal memory only
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000c4);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		_reload_on_repeat = 0;
		//is FIFO transfer?
		_fifoTransfer = _cnt->repeat
			&& (_dstAddr == 0x40000A0 || _dstAddr == 0x40000A4);
		break;
	}
	case 2:		//DMA2
	{
		//read source address
		_srcAddr = GBA::memory.read_32(0x40000c8);
		_srcAddr &= 0xFFFFFFF;		//any memory
		//read destination address
		_dstAddr = GBA::memory.read_32(0x40000cc);
		_dstAddr &= 0x7FFFFFF;	//internal memory only
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000d0);
		_transfLen &= 0x3fff;	//14 bit only
		if (_transfLen == 0) _transfLen = 0x4000;
		_transfCounter = 0;
		_reload_on_repeat = 0;
		//is FIFO transfer?
		_fifoTransfer = _cnt->repeat
			&& (_dstAddr == 0x40000A0 || _dstAddr == 0x40000A4);
		break;
	}
	case 3:		//DMA3
	{
		//read source address
		_srcAddr = GBA::memory.read_32(0x40000d4);
		_srcAddr &= 0xFFFFFFF;		//any memory
		//read destination address
		_srcAddr = GBA::memory.read_32(0x40000d8);
		_srcAddr &= 0xFFFFFFF;		//any memory
		//read transfer lenght
		_transfLen = GBA::memory.read_16(0x40000dc);	//16 bit
		if (_transfLen == 0) _transfLen = 0x10000;
		_transfCounter = 0;
		_reload_on_repeat = 0;
		break;
	}
	}
}

void Dma::trigger(Dma_Trigger trigger) {
	if (!_cnt->enable)
		return;

	switch (_cnt->timing) {
	case 0:	//immidiately
		full_run_dma();
		break;
	case 1:	//vblank
		if (trigger == Dma_Trigger::VBLANK) {
			load_on_repeat();
			full_run_dma();
		}
		break;

	case 2:	//hblank
		if (trigger == Dma_Trigger::HBLANK) {
			load_on_repeat();
			full_run_dma();
		}
		break;
	case 3:	//special trigger
		switch (_dmaNr) {
		case 1:	//DMA1 & DMA2 special triggers are FIFO
		case 2:
			if (trigger == Dma_Trigger::FIFO) {
				load_on_repeat();
				full_run_dma();
			}
			break;
		case 3:	//DMA3 special trigger is video capture
			if (trigger == Dma_Trigger::VIDEO_CAPTURE) {
				load_on_repeat();
				full_run_dma();
			}
			break;
		}
		break;
	}
}

void Dma::disable() {
	_reload_on_repeat = false;
	_fifoTransfer = false;
	_cnt->enable = 0;
}

void Dma::full_run_dma(void) {
	int8_t inc_transform[4] = {1, -1, 0, 1};

	if (_fifoTransfer) {	//fifo transfer
		int32_t src_inc_mod = inc_transform[_cnt->src_cnt];
		int32_t dst_inc_mod = 1;
		for (_transfCounter = 0; _transfCounter < 4; _transfCounter++) {
			uint32_t read_val = GBA::memory.read_32(_srcAddr + _transfCounter * 4 * src_inc_mod);
			GBA::memory.write_32(_dstAddr + _transfCounter * 4 * dst_inc_mod, read_val);
		}
	}else if (_cnt->type == 0){
		//16 bit transfer
		int32_t src_inc_mod = inc_transform[_cnt->src_cnt];
		int32_t dst_inc_mod = inc_transform[_cnt->dst_cnt];
		for (_transfCounter = 0; _transfCounter < _transfLen; _transfCounter++) {
			uint16_t read_val = GBA::memory.read_16(_srcAddr + _transfCounter * 2 * src_inc_mod);
			GBA::memory.write_16(_dstAddr + _transfCounter * 2 * dst_inc_mod, read_val);
		}
	}
	else {
		//32 bit transfer
		for (_transfCounter = 0; _transfCounter < _transfLen; _transfCounter++) {
			int32_t src_inc_mod = inc_transform[_cnt->src_cnt];
			int32_t dst_inc_mod = inc_transform[_cnt->dst_cnt];
			for (_transfCounter = 0; _transfCounter < _transfLen; _transfCounter++) {
				uint32_t read_val = GBA::memory.read_32(_srcAddr + _transfCounter * 4 * src_inc_mod);
				GBA::memory.write_32(_dstAddr + _transfCounter * 4 * dst_inc_mod, read_val);
			}
		}
	}
	_transfCounter = 0;
	if (!_cnt->repeat) {	//no repeat
		_cnt->enable = 0;
	}
	if (_cnt->irq)		//irq upon end of word count
		GBA::irq.setDMAFlag(_dmaNr);
	_reload_on_repeat = _cnt->repeat;
} 
