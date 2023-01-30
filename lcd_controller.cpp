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
	
	obj_attribute* spriteAttributes = (obj_attribute*)params.oam_copy;
	rgba_color objRowBuffer[128];	//allocate 128 pixels
	int bufferLen;
	rgba_color* rgba_frameBuffer = (rgba_color*)params.screenBuffer;

	if (params.vCount >= 160)
		return;

	for (int i = 127; i >= 0; i--) {
		obj_attribute& currentObjAttr = spriteAttributes[i];
		if (currentObjAttr.x_coord == 0 && currentObjAttr.y_coord == 0)
			continue;

		if (!currentObjAttr.rot_scale_flag && currentObjAttr.double_or_obj_disable)	//object disabled
			continue;

		V2Int spriteSize = sprites_tiles_table[currentObjAttr.obj_shape][currentObjAttr.obj_size];
		int double_size = (currentObjAttr.double_or_obj_disable & currentObjAttr.rot_scale_flag) + 1;
		int rowToDraw = (params.vCount - currentObjAttr.y_coord) / double_size;

		if (rowToDraw < 0 || rowToDraw >= spriteSize.y)
			continue;

		int pixel_count = 0;
		getSpriteRowMem(currentObjAttr, spriteSize, rowToDraw, objRowBuffer, pixel_count, params);

		//copy the sprite row on the frame buffer
		for (int pixel = 0; pixel < pixel_count; pixel++) {
			int pixel_x = (currentObjAttr.x_coord + pixel) % 512;
			if (pixel_x < 0 || pixel_x >= 240) continue;

			rgba_frameBuffer[params.vCount * 240 + pixel_x] = objRowBuffer[pixel];
		}
	}

}

void LcdController::getSpriteRowMem(obj_attribute& attr, V2Int &spriteSize, int rowToDraw, rgba_color *objRowBuffer, int &pixel_count, helperParams& params) {
	
	uint8_t* spritesMem = (params.DISPCNT.bg_mode < 3) ? params.vram_copy + 0x10000 : params.vram_copy + 0x14000;
	

	if (!params.DISPCNT.obj_vram_map) {
		uint8_t obj_mem_tile_size;
		obj_mem_tile_size = 32 - attr.palette * 16;	//256 pixels for 16 color mode, 128 pixels for 256 color mode

		uint32_t rowTile = (attr.tile_number / (1 + attr.palette)) + obj_mem_tile_size * (rowToDraw / 8);
		uint32_t lineInTileToDraw = rowToDraw % 8;
		int double_pixel = (attr.double_or_obj_disable & attr.rot_scale_flag) + 1;
		pixel_count = spriteSize.x * double_pixel;
		for (int i = 0; i < spriteSize.x / 8; i++) {
			uint8_t* tileMem = spritesMem + (rowTile + i) * (0x20 + attr.palette * 0x20);
			uint8_t* tileRowMem = tileMem + lineInTileToDraw * 8 / (2 - attr.palette);

			if (attr.palette) {	//256 color palette
				gba_palette_color* palette = (gba_palette_color*)(params.palette_copy + 0x200);

				for (int pixel = 0; pixel < 8; pixel++) {
					gba_palette_color gba_color = palette[tileRowMem[pixel]];
					for (int d = 0; d < double_pixel; d++) {
						rgba_color& rgba_pixel_color = objRowBuffer[double_pixel * (i * 8 + pixel) + d];
						rgba_pixel_color.r = gba_color.r * 8;
						rgba_pixel_color.g = gba_color.g * 8;
						rgba_pixel_color.b = gba_color.b * 8;
						rgba_pixel_color.a = 255;
					}
				}
			}
			else {	//16 color palette
				gba_palette_color* palette = (gba_palette_color*)(params.palette_copy + 0x200 + attr.palette_num * 32);

				for (int pixel = 0; pixel < 8; pixel++) {
					int palette_entry = (tileRowMem[pixel / 2] >> (4 - 4 * (pixel % 2))) & 0b1111;
					gba_palette_color gba_color = palette[palette_entry];
					for (int d = 0; d < double_pixel; d++) {
						rgba_color& rgba_pixel_color = objRowBuffer[double_pixel * (i * 8 + pixel) + d];
						rgba_pixel_color.r = gba_color.r * 8;
						rgba_pixel_color.g = gba_color.g * 8;
						rgba_pixel_color.b = gba_color.b * 8;
						rgba_pixel_color.a = 255;
					}
				}
			}
			
		}
	}
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
			drawerParams.screenBuffer = frameBuffers[activeFrameBuffer];
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
					memset(frameBuffers[activeFrameBuffer], 0, 240 * 160 * 4);	//clean the buffer
				}
			}
		}
	}
}


void LcdController::update() {

}