#include "lcd_controller.h"
#include "gba.h"
#include "memory_mapper.h"

#include <cstdint>

LcdController::LcdController() {
	h_cnt = 0;
	video_cnt = 0;

	DISPCNT = (dispCnt_struct *)GBA::memory.get_io_reg(0);
	DISPSTAT = (dispStat_struct*)GBA::memory.get_io_reg(4);
	VCOUNT = GBA::memory.get_io_reg(6);

	DISPSTAT->hblank_flag = 0;
	DISPSTAT->vblank_flag = 0;
}


LcdController::~LcdController() {

}


void LcdController::update_V_count(uint32_t cycles) {
	video_cnt += cycles;
	h_cnt = video_cnt / 4;

	if (video_cnt > 960) {	//h-blank
		DISPSTAT->hblank_flag = 1;

		if (video_cnt > 1232) {
			DISPSTAT->hblank_flag = 0;	//h-draw
			video_cnt %= 1232;
			*VCOUNT += 1;
			if (*VCOUNT >= 160) {	//v-blank
				DISPSTAT->vblank_flag = 1;
				if (*VCOUNT >= 228) {
					*VCOUNT %= 228;
					DISPSTAT->vblank_flag = 0;	//v-draw
				}
			}
		}
	}
}


void LcdController::update() {

}