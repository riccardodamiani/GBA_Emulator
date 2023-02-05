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
	rgba_color objRowBuffer[240];	//allocate 240 horizontal pixels
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

		int y_coord = currentObjAttr.y_coord;
		while (y_coord + spriteSize.y * double_size > 255) y_coord -= 255;
		int spriteRowToDraw = (params.vCount - y_coord);

		//not visible
		if (spriteRowToDraw < 0)
			continue;

		// sprite row outside sprite rectangle
		if (spriteRowToDraw >= spriteSize.y * double_size)
			continue;

		int pixel_count = 0;
		getSpriteScanline(currentObjAttr, spriteSize, spriteRowToDraw, objRowBuffer, params);

		//copy the sprite row in the frame buffer
		for (int pixel = 0; pixel < 240; pixel++) {

			if (objRowBuffer[pixel].a == 0)	//ignore transparent pixels
				continue;

			rgba_frameBuffer[params.vCount * 240 + pixel] = objRowBuffer[pixel];
		}
	}

}

void LcdController::getSpriteScanline(obj_attribute& attr, V2Int &spriteSize, int rowToDraw, rgba_color *objRowBuffer, helperParams& params) {
	
	memset(objRowBuffer, 0, 240 * sizeof(rgba_color));

	int double_size = (attr.double_or_obj_disable & attr.rot_scale_flag) + 1;
	obj_attribute* spriteAttributes = (obj_attribute*)params.oam_copy;
	int x_coord = attr.x_coord;
	while(x_coord + spriteSize.x * double_size >= 512) x_coord -= 512;

	for (int i = std::max(0, x_coord); i < 240; i++) {

		V2Int transformedCoords;
		Transf_Gba_Matrix transform_matrix;

		// screen pixel is outside the sprite rectangle
		if (i - x_coord >= spriteSize.x * double_size) {
			continue;
		}

		if (attr.rot_scale_flag) {	//apply affine transformation
			transform_matrix = {
				spriteAttributes[attr.rot_scale_par_sel * 4].rot_scale_param,
				spriteAttributes[attr.rot_scale_par_sel * 4 + 1].rot_scale_param,
				spriteAttributes[attr.rot_scale_par_sel * 4 + 2].rot_scale_param,
				spriteAttributes[attr.rot_scale_par_sel * 4 + 3].rot_scale_param,
			};
			transformPixelCoords({ i - x_coord, rowToDraw }, transformedCoords, transform_matrix, spriteSize, double_size);
		}
		else {
			transformedCoords = { i - x_coord, rowToDraw };
		}

		// sprite pixel is outside the sprite mem
		if (transformedCoords.x < 0 || transformedCoords.y < 0 ||
			transformedCoords.x >= spriteSize.x || transformedCoords.y >= spriteSize.y) {
			continue;
		}
		
		// get sprite pixel color
		getSpritePixel(attr, params, transformedCoords, objRowBuffer[i]);
	}

}

void LcdController::getSpritePixel(obj_attribute& attr, helperParams& params, V2Int pixelCoords, rgba_color& color) {
	uint8_t* spritesMem = (params.DISPCNT.bg_mode < 3) ? params.vram_copy + 0x10000 : params.vram_copy + 0x14000;
	
	if (!params.DISPCNT.obj_vram_map) {
		uint8_t obj_mem_tile_size;
		obj_mem_tile_size = 32 - attr.palette * 16;	//256 pixels for 16 color mode, 128 pixels for 256 color mode

		uint32_t rowTile = (attr.tile_number / (1 + attr.palette)) + obj_mem_tile_size * (pixelCoords.y / 8);
		uint32_t lineInTileToDraw = pixelCoords.y % 8;
		
		uint8_t* tileMem = spritesMem + (rowTile + pixelCoords.x / 8) * (0x20 + attr.palette * 0x20);
		uint8_t* tileRowMem = tileMem + lineInTileToDraw * 8 / (2 - attr.palette);

		if (attr.palette) {	//256 color palette
			gba_palette_color* palette = (gba_palette_color*)(params.palette_copy + 0x200);

			uint8_t alpha = 255;
			if (tileRowMem[pixelCoords.x % 8] == 0) alpha = 0;

			gba_palette_color gba_color = palette[tileRowMem[pixelCoords.x % 8]];
			color.r = gba_color.r * 8;
			color.g = gba_color.g * 8;
			color.b = gba_color.b * 8;
			color.a = alpha;
		}
		else {	//16 color palette
			gba_palette_color* palette = (gba_palette_color*)(params.palette_copy + 0x200 + attr.palette_num * 32);

			uint8_t alpha = 255;
			if (tileRowMem[pixelCoords.x % 8] == 0) alpha = 0;

			int palette_entry = (tileRowMem[(pixelCoords.x % 8) / 2] >> (4 - 4 * ((pixelCoords.x % 8) % 2))) & 0b1111;
			gba_palette_color gba_color = palette[palette_entry];
			color.r = gba_color.r * 8;
			color.g = gba_color.g * 8;
			color.b = gba_color.b * 8;
			color.a = alpha;
		}
	}
}

void LcdController::transformPixelCoords(V2Int src_coords, V2Int& dst_coords, Transf_Gba_Matrix& gba_matrix, V2Int& spriteSize, int doubleSize) {
	Matrix2x2 matrix = {
		-1.0 * (gba_matrix.PA.sign * 2 - 1) * gba_matrix.PA.integer + gba_matrix.PA.fract / 256.0,
		-1.0 * (gba_matrix.PB.sign * 2 - 1) * gba_matrix.PB.integer + gba_matrix.PB.fract / 256.0,
		-1.0 * (gba_matrix.PC.sign * 2 - 1) * gba_matrix.PC.integer + gba_matrix.PC.fract / 256.0,
		-1.0 * (gba_matrix.PD.sign * 2 - 1) * gba_matrix.PD.integer + gba_matrix.PD.fract / 256.0
	};

	vector2 p0 = { spriteSize.x / 2, spriteSize.y / 2 };
	vector2 p01 = { (float)src_coords.x - p0.x * doubleSize, (float)src_coords.y - p0.y * doubleSize };
	dst_coords = { (int)(matrix.PA * p01.x + matrix.PB * p01.y + p0.x),
		(int)(matrix.PC * p01.x + matrix.PD * p01.y + p0.y) };

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