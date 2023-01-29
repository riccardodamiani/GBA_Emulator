#include "lcd_controller.h"
#include "gba.h"
#include "memory_mapper.h"
#include "interrupt.h"
#include "multithreadManager.h"

#include <cstdint>

LcdController::LcdController() {
	h_cnt = 0;
	video_cnt = 0;

	DISPCNT = (dispCnt_struct *)GBA::memory.get_io_reg(0);
	DISPSTAT = (dispStat_struct*)GBA::memory.get_io_reg(4);
	VCOUNT = GBA::memory.get_io_reg(6);

	DISPSTAT->hblank_flag = 0;
	DISPSTAT->vblank_flag = 0;

	activeFrameBuffer = 0;

	//allocate frame buffers
	frameBuffers[0] = new uint8_t[240 * 160 * 4];
	frameBuffers[1] = new uint8_t[240 * 160 * 4];
	whiteFrameBuffer = new uint8_t[240 * 160 * 4];

	//init frame buffers
	memset(frameBuffers[0], 0, 240 * 160 * 4);
	memset(frameBuffers[1], 0, 240 * 160 * 4);
	memset(whiteFrameBuffer, 255, 240 * 160 * 4);

	//setup multithreading stuff
	oam_copy = new uint8_t[0x400];
	palette_copy = new uint8_t[0x400];
	vram_copy = new uint8_t[0x18000];

	drawerParams.oam_copy = oam_copy;
	drawerParams.palette_copy = palette_copy;
	drawerParams.vram_copy = vram_copy;

	drawer = new MultithreadManager(1);
}


LcdController::~LcdController() {

	//cleanup everything
	delete frameBuffers[0];
	delete frameBuffers[1];
	delete whiteFrameBuffer;

	drawer->destroy();
	delete drawer;

	delete oam_copy;
	delete palette_copy;
	delete vram_copy;
}


const uint32_t const* LcdController::getBufferToRender() {
	if (DISPCNT->forced_blank) {
		return (uint32_t *)whiteFrameBuffer;
	}

	return (uint32_t*)frameBuffers[1 - activeFrameBuffer];	//return the finished buffer
}

void LcdController::helperRoutine(int start_index, int end_index, void* args) {
	helperParams &params = *(helperParams*)args;

	//copy the video memory in a protected location
	uint8_t* palette = GBA::memory.getMemoryAddr(5);
	uint8_t* vram = GBA::memory.getMemoryAddr(6);
	uint8_t* oam = GBA::memory.getMemoryAddr(7);
	memcpy(params.palette_copy, palette, 0x400);
	memcpy(params.vram_copy, vram, 0x18000);
	memcpy(params.oam_copy, oam, 0x400);

	if (params.DISPCNT.obj_enable) {
		printSprites(params);
	}
}

void LcdController::printSprites(helperParams& params) {

}

void LcdController::update_V_count(uint32_t cycles) {
	video_cnt += cycles;
	h_cnt = video_cnt / 4;

	if (video_cnt > 960) {	//h-blank
		if (!DISPSTAT->hblank_flag && *VCOUNT < 160) {	//first time in h-blank: draw the current scanline

			drawer->Wait();	//wait for the helper to finish the previous job
			drawerParams.DISPCNT = *DISPCNT;
			drawerParams.DISPSTAT = *DISPSTAT;
			drawerParams.vCount = *VCOUNT;
			drawerParams.screenBUffer = frameBuffers[activeFrameBuffer];
			drawer->startWork(1, helperRoutine, &drawerParams);	//start the new job
		}
		DISPSTAT->hblank_flag = 1;

		if (video_cnt > 1232) {
			DISPSTAT->hblank_flag = 0;	//h-draw
			video_cnt %= 1232;
			GBA::irq.setHBlankFlag();	//h-blank irq
			*VCOUNT += 1;

			//v-counter irq
			if (DISPSTAT->vcounter_irq_enable && DISPSTAT->LYC == *VCOUNT) {
				GBA::irq.setVCounterFlag();
			}

			if (*VCOUNT >= 160) {	//v-blank
				if (!DISPSTAT->vblank_flag && DISPSTAT->vblank_irq_enable) {	//v-blank irq
					GBA::irq.setVBlankFlag();
				}
				DISPSTAT->vblank_flag = 1;
				if (*VCOUNT >= 228) {
					*VCOUNT %= 228;
					DISPSTAT->vblank_flag = 0;	//v-draw
					activeFrameBuffer = 1 - activeFrameBuffer;	//change frame buffer
				}
			}
		}
	}
}


void LcdController::update() {

}