#ifndef DMA_H
#define DMA_H

#include <cstdint>

struct dma_control_struct {
	uint16_t unused : 5,
		dst_cnt : 2,
		src_cnt : 2,
		repeat : 1,
		type : 1,
		gamepak_drq : 1,
		timing : 2,
		irq : 1,
		enable : 1;
};

enum Dma_Trigger {
	EMPTY_TRIGGER = 0,	//used to check if dma needs to be triggered immidiately
	VBLANK = 1,
	HBLANK = 2,
	FIFO = 3,
	VIDEO_CAPTURE = 4
};

class Dma {
public:
	Dma(uint8_t num);
	void trigger(Dma_Trigger trigger);
	void enable_dma(void);
	void full_run_dma(void);
	void load_on_repeat(void);
	void disable();
private:
	uint32_t _srcAddr, _dstAddr, _transfLen, _transfCounter;
	dma_control_struct* _cnt;
	uint8_t _dmaNr, _reload_on_repeat;
	bool _fifoTransfer;
};

#endif
