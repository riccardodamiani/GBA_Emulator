#include "lcd_controller.h"
#include "gba.h"
#include "memory_mapper.h"
#include "interrupt.h"
#include "multithreadManager.h"
#include "dma.h"
#include "error.h"

#include <cstdint>

LcdController::LcdController() {
	h_cnt = 0;
	video_cnt = 0;

	DISPCNT = (dispCnt_struct *)GBA::memory.get_io_reg(0);
	DISPSTAT = (dispStat_struct*)GBA::memory.get_io_reg(4);
	VCOUNT = GBA::memory.get_io_reg(6);

	WIN0H = (obj_window_size*)GBA::memory.get_io_reg(0x40);
	WIN1H = (obj_window_size*)GBA::memory.get_io_reg(0x42);
	WIN0V = (obj_window_size*)GBA::memory.get_io_reg(0x44);
	WIN1V = (obj_window_size*)GBA::memory.get_io_reg(0x46);
	WININ = (window_control*)GBA::memory.get_io_reg(0x48);
	WINOUT = (obj_window_control*)GBA::memory.get_io_reg(0x4a);

	BLDCNT = GBA::memory.get_io_reg(0x50);
	BLDALPHA = (BLDALPHA_struct*)GBA::memory.get_io_reg(0x52);
	BLDY = (BLDY_struct*)GBA::memory.get_io_reg(0x54);
	BG0CNT = (BGCNT_struct*)GBA::memory.get_io_reg(8);
	BG_OFFSETS = (bg_scrolling_struct*)GBA::memory.get_io_reg(0x10);
	BG2_TRANSF_MATRIX = (Transf_Gba_Matrix*)GBA::memory.get_io_reg(0x20);
	GB2_REF_POINT = (BG_reference_point_struct*)GBA::memory.get_io_reg(0x28);
	BG3_TRANSF_MATRIX = (Transf_Gba_Matrix*)GBA::memory.get_io_reg(0x30);
	GB3_REF_POINT = (BG_reference_point_struct*)GBA::memory.get_io_reg(0x38);

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

bool LcdController::activeBg(helperParams& params, int bg_nr) {

	uint16_t dispcnt = 0;
	memcpy(&dispcnt, &params.DISPCNT, 2);
	return (dispcnt >> (8 + bg_nr)) & 1;
}

void LcdController::apply_special_effects(helperParams& params, SpecialEffectPixel& lowerPixel,
	graphicsPixel& upperPixel,
	SpecialEffectPixel& finalPixel) {

	uint8_t specialEffect = (params.BLDCNT >> 6) & 0b11;

	//check for alpha blending
	if (((upperPixel.type & LayerType::OBJ) && upperPixel.option.alphaBlending) ||	//is a semi-transparent object pixel
		(specialEffect == 1 &&	//alpha blending enabled
			((params.BLDCNT & (uint16_t)upperPixel.type) != 0))) {	//the layer is a first target layer for alpha blending 
		uint32_t eva = params.BLDALPHA.eva_coeff <= 16 ? params.BLDALPHA.eva_coeff : 16;
		uint32_t evb = params.BLDALPHA.evb_coeff <= 16 ? params.BLDALPHA.evb_coeff : 16;
		if (((lowerPixel.type << 8) & 0xff00 & params.BLDCNT) != 0 &&	//the below pixel is a second target for alpha blending
			lowerPixel.option.priority > upperPixel.option.priority) {	//the second target pixel has lower priority than the first target
			finalPixel.color.r = std::min<int>(255, upperPixel.pixel.color.r * eva / 16 +
				finalPixel.color.r * evb / 16);
			finalPixel.color.g = std::min<int>(255, upperPixel.pixel.color.g * eva / 16 +
				finalPixel.color.g * evb / 16);
			finalPixel.color.b = std::min<int>(255, upperPixel.pixel.color.b * eva / 16 +
				finalPixel.color.b * evb / 16);
			finalPixel.color.a = 255;

			return;
		}
	}

	finalPixel.color = upperPixel.pixel.color;
	finalPixel.option = upperPixel.pixel.option;
	finalPixel.type = upperPixel.type;

	//check for brigthness increase
	if (specialEffect == 2) {
		uint32_t evy = params.BLDY.evy_coeff <= 16 ? params.BLDY.evy_coeff : 16;
		
		finalPixel.color.r = std::min<int>(255, finalPixel.color.r + (255 - finalPixel.color.r) * evy / 16);
		finalPixel.color.g = std::min<int>(255, finalPixel.color.g + (255 - finalPixel.color.g) * evy / 16);
		finalPixel.color.b = std::min<int>(255, finalPixel.color.b + (255 - finalPixel.color.b) * evy / 16);
		return;
	}
	//check for brigthness decrease
	if (specialEffect == 3) {
		uint32_t evy = params.BLDY.evy_coeff <= 16 ? params.BLDY.evy_coeff : 16;

		finalPixel.color.r = std::min<int>(255, finalPixel.color.r - (255 - finalPixel.color.r) * evy / 16);
		finalPixel.color.g = std::min<int>(255, finalPixel.color.g - (255 - finalPixel.color.g) * evy / 16);
		finalPixel.color.b = std::min<int>(255, finalPixel.color.b - (255 - finalPixel.color.b) * evy / 16);
		return;
	}
	
}

void LcdController::order_bg_scanlines(graphicsScanline** layers, int activeLayers) {
	bool sorted = true;

	for (int i = 0; i < activeLayers - 1; i++) {
		uint8_t p0 = layers[i]->scanline[0].option.priority;
		uint8_t p1 = layers[i + 1]->scanline[0].option.priority;
		if (p0 < p1 ||	//lower priority
			(p0 == p1 && (layers[i]->type < layers[i + 1]->type))	//same priority but lower bg number
			) {
			graphicsScanline* t = layers[i];
			layers[i] = layers[i + 1];
			layers[i + 1] = t;

			sorted = false;
		}
	}
	if (sorted == false) {
		order_bg_scanlines(layers, activeLayers);
	}
}

void LcdController::order_layer_pixels(graphicsPixel** layerPixels, int activeLayers) {
	if (activeLayers < 2)
		return;

	graphicsPixel* objPixel = layerPixels[activeLayers - 1];
	
	if (objPixel->type != OBJ)
		return;

	for (int i = 0; i < activeLayers; i++) {
		if(objPixel->option.priority > layerPixels[i]->option.priority) {	//found the spot to put this pixel
			for (int j = activeLayers-2; j >= i; j--) {
				layerPixels[j + 1] = layerPixels[j];
			}
			layerPixels[i] = objPixel;
			return;
		}
	}
}

//returns 1 if the backgrouns pixel is visible in a window, 0 otherwise
inline uint8_t LcdController::get_bg_window_mask(helperParams& params, LayerType type, uint8_t objWindowMask, uint16_t x_coord, uint16_t y_coord) {
	uint8_t visible = 1;

	uint8_t objwin = params.DISPCNT.obj_wnd_enable * (params.WINOUT.obj_win_layer_enable & type);
	if (objwin) {
		visible = objWindowMask;
	}
	return visible;
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

	rgba_color* rgba_frameBuffer = (rgba_color*)params.screenBuffer;
	int activeLayers = 0;

	//5 layers: 4 backgrouns and one object layer
	graphicsScanline** layers;
	layers = new graphicsScanline* [5];

	uint8_t windowObjMask[240];	//mask scanline for window object

	get_bg_layer_scanline(params, layers, activeLayers);
	order_bg_scanlines(layers, activeLayers);

	getObjLayerScanline(params, layers, activeLayers, windowObjMask);

	if (!activeLayers) {
		memset(rgba_frameBuffer, 0xff, 240 * 4);
		//delete stuff
		for (int i = 0; i < activeLayers; i++) {
			delete layers[i];
		}
		delete[] layers;
		return;
	}
	
	//a temporary buffer to store layers pixels
	rgba_color backdropPixel;
	bool empty_pixel;	//used for backdrop
	graphicsPixel layerPixels[5];
	graphicsPixel** layerPixels_ptr;
	layerPixels_ptr = new graphicsPixel * [5];

	SpecialEffectPixel finalPixel;
	//update frame buffer
	for (int pixel = 0; pixel < 240; pixel++) {
		finalPixel.color = {0xff, 0xff, 0xff, 0xff};	//starts with all white
		backdropPixel = { 0xff, 0xff, 0xff, 0xff };	//starts with backdrop white
		finalPixel.option.priority = 0x69;	//a random number. Just need to make sure is not close to the numbers 0-4
		finalPixel.type = LayerType::NO_TYPE;
		empty_pixel = true;

		//copy the layers pixels into a separate buffer to order them by priority
		for (int i = 0; i < activeLayers; i++) {
			layerPixels[i] = { layers[i]->scanline[pixel], layers[i]->type, layers[i]->scanline[pixel].option};
			layerPixels_ptr[i] = &layerPixels[i];
		}
		//oder the layers by priority
		order_layer_pixels(layerPixels_ptr, activeLayers);

		//print all pixel one on top of the other. Look also for special effects
		for (int layer = 0; layer < activeLayers; layer++) {
			graphicsPixel& currentPixel = *layerPixels_ptr[layer];

			//save backdrop pixel
			if (currentPixel.type == LayerType::BG0) {
				backdropPixel = currentPixel.pixel.color;
				backdropPixel.a = 255;
			}
			//check for obj window visibility
			currentPixel.pixel.color.a *= get_bg_window_mask(params, currentPixel.type, windowObjMask[pixel], pixel, params.vCount);

			if (currentPixel.pixel.color.a == 0)	//ignores transparent pixels
				continue;
			empty_pixel = false;
			//check for alpha blending
			apply_special_effects(params, finalPixel, currentPixel, finalPixel);
		}
		//backdrop. if pixel is not visible use color of bg0 layer
		if (empty_pixel) {
			finalPixel.color = backdropPixel;
		}

		//print final color in the frame buffer
		rgba_frameBuffer[params.vCount * 240 + pixel] = finalPixel.color;
	}

	//delete stuff
	for (int i = 0; i < activeLayers; i++) {
		delete layers[i];
	}
	delete[] layers;
	delete[] layerPixels_ptr;
}

void LcdController::get_bg_layer_scanline(helperParams& params, 
	graphicsScanline** layers, int& activeLayers){

	switch (params.DISPCNT.bg_mode) {
	case 0:
		background_mode0(params, layers, activeLayers);
		break;
	case 1:
		//printError(ErrorType::WARNING, "Background mode 1 not supported");
		break;
	case 2:
		background_mode2(params, layers, activeLayers);
		return;
		break;
	case 3:
		//printError(ErrorType::WARNING, "Background mode 3 not supported");
		break;
	case 4:
		//printError(ErrorType::WARNING, "Background mode 4 not supported");
		break;
	case 5:
		//printError(ErrorType::WARNING, "Background mode 5 not supported");
		break;
	}	
}

//mode 0
void LcdController::background_mode0(helperParams& params, graphicsScanline** layers, int& activeLayers) {
	
	uint16_t dispcnt;
	memcpy(&dispcnt, &params.DISPCNT, 2);

	for (int bg_layer = 0; bg_layer < 4; bg_layer++) {
		if ((dispcnt >> (8 + bg_layer)) & 1) {	//if layer is enabled
			//create new scanline layer
			graphicsScanline* bg_scanline = new graphicsScanline();
			layers[activeLayers] = bg_scanline;
			activeLayers++;

			//background size in pixels
			V2Int bg_size = TextModeScreenSize_Trans[params.BGCNT[bg_layer].screen_size];

			//get each pixel of the scanline
			for (int screen_x = 0; screen_x < 240; screen_x++) {

				//Get the real coordinates of the background layer to draw
				int32_t bg_row_to_draw = ((float)params.vCount + params.BG_OFFSETS[bg_layer].VOFS.offset);
				int32_t bg_col_to_draw = ((float)screen_x + params.BG_OFFSETS[bg_layer].HOFS.offset);

				V2Int bg_coords = { bg_col_to_draw, bg_row_to_draw };

				//screen overflow always wrap around
				bg_coords.x %= bg_size.x;
				bg_coords.y %= bg_size.y;
				if (bg_coords.x < 0) bg_coords.x += bg_size.x;
				if (bg_coords.y < 0) bg_coords.y += bg_size.y;

				//get the pixel
				get_text_bg_pixel_color(bg_layer, params, bg_coords, bg_scanline->scanline[screen_x].color, params.BGCNT[bg_layer].screen_size);
				bg_scanline->scanline[screen_x].option.priority = params.BGCNT[bg_layer].bg_priority;
				bg_scanline->type = LayerType(1 << bg_layer);
			}
		}
	}
	
}

void LcdController::background_mode2(helperParams& params, graphicsScanline** layers, int& activeLayers) {
	if (params.DISPCNT.bg2_enable) {
		uint8_t* bg_tile_data = &params.vram_copy[0x4000 * params.BGCNT[2].ch_base_block];
		uint8_t* bg_map_base = &params.vram_copy[0x800 * params.BGCNT[2].screen_base_block];


	}

	if (params.DISPCNT.bg3_enable) {

		graphicsScanline* bg3_scanline = new graphicsScanline();
		layers[activeLayers] = bg3_scanline;
		activeLayers++;

		//background size in pixels
		V2Int bg_size = {128 << params.BGCNT[3].screen_size, 128 << params.BGCNT[3].screen_size };

		//Calculates the offsets of the real coordinates of the background layer to draw
		float r_y = -1.0 * (params.GB3_REF_POINT.y.sign * 2 - 1) * params.GB3_REF_POINT.y.integer +
			params.GB3_REF_POINT.y.fract / 256.0;
		float r_x = -1.0 * (params.GB3_REF_POINT.x.sign * 2 - 1) * params.GB3_REF_POINT.x.integer +
			params.GB3_REF_POINT.x.fract / 256.0;
		r_x = -fmod(r_x, bg_size.x);
		r_y = -fmod(r_y, bg_size.y);
		if (r_x < 0) r_x += bg_size.x;
		if (r_y < 0) r_y += bg_size.y;

		//get each pixel of the scanline
		for (int screen_x = 0; screen_x < 240; screen_x++) {
			
			//Get the real coordinates of the background layer to draw
			float bg_row_to_draw = ((float)params.vCount + r_y);
			float bg_col_to_draw = ((float)screen_x + r_x);

			vector2 bg_coords = { bg_col_to_draw, bg_row_to_draw};
			V2Int trans_coords;
			//apply the transformation (affine layer)
			bg_transform_pixel_coords(bg_coords, trans_coords, params.BG3_TRANSF_MATRIX, bg_size);

			//screen overflow
			if (params.BGCNT[3].display_overflow == 0) {	//transparent 
				if (trans_coords.x < 0 || trans_coords.y < 0 ||
					trans_coords.x >= bg_size.x || trans_coords.y >= bg_size.y) {
					continue;
				}
			}
			else {//wrap around
				trans_coords.x %= bg_size.x;
				trans_coords.y %= bg_size.y;
				if (trans_coords.x < 0) trans_coords.x += bg_size.x;
				if (trans_coords.y < 0) trans_coords.y += bg_size.y;
			}
			//get the pixel
			get_affine_bg_pixel_color(3, params, trans_coords, bg3_scanline->scanline[screen_x].color, bg_size);
			bg3_scanline->scanline[screen_x].option.priority = params.BGCNT[3].bg_priority;
			bg3_scanline->type = LayerType::BG3;
		}
	}
}

void LcdController::get_affine_bg_pixel_color(int bg_num, helperParams& params, V2Int coords, rgba_color& color, V2Int& bgSize) {
	uint8_t* bg_tile_data = &params.vram_copy[0x4000 * params.BGCNT[bg_num].ch_base_block];
	uint8_t* bg_map_base = &params.vram_copy[0x800 * params.BGCNT[bg_num].screen_base_block];

	int tile_nr = coords.x / 8 + (coords.y / 8) * bgSize.x / 8;
	//the tile info takes always 1 byte and you can only use 256/1 palette
	uint8_t* tileMem = &bg_tile_data[64 * bg_map_base[tile_nr]];

	V2Int tile_pixel_coords = { coords.x % 8, coords.y % 8 };
	int palette_color = tileMem[tile_pixel_coords.x + tile_pixel_coords.y * 8];

	gba_palette_color* palette = (gba_palette_color*)params.palette_copy;

	uint8_t alpha = 255;
	if (palette_color == 0) alpha = 0;

	gba_palette_color gba_color = palette[palette_color];
	color.r = gba_color.r * 8;
	color.g = gba_color.g * 8;
	color.b = gba_color.b * 8;
	color.a = alpha;
}

void LcdController::get_text_bg_pixel_color(int bg_num, helperParams& params, V2Int coords, rgba_color& color, uint8_t screen_size) {
	uint8_t* bg_tile_data = &params.vram_copy[0x4000 * params.BGCNT[bg_num].ch_base_block];
	
	uint8_t area_offset = 0;
	//a background is organized in 1 to 4 areas of 256x256 pixels (32x32 tiles).
	V2Int bgSize = TextModeScreenSize_Trans[screen_size];

	//calculates the tile map base offset due to tile areas
	if (screen_size == 3) {
		area_offset = (coords.x / 256) + (coords.y / 256) * 2;
	}
	else {
		area_offset = (coords.x / 256) + (coords.y / 256);
	}
	
	uint8_t* bg_map_base = &params.vram_copy[0x800 * params.BGCNT[bg_num].screen_base_block + area_offset * 0x800];

	//recaulculates the pixel coords for the current tile area
	coords.x %= 256;
	coords.y %= 256;

	//calculates the tile number in the tile memory
	int tile_nr = coords.x / 8 + (coords.y / 8) * 32;

	tile_info_struct tileInfo;
	gba_palette_color* palette = (gba_palette_color*)params.palette_copy;

	V2Int tile_pixel_coords = { coords.x % 8, coords.y % 8 };

	uint8_t palette_color;
	if (params.BGCNT[bg_num].palette) {	//palette 256/1
		memcpy(&tileInfo, &bg_map_base[tile_nr * 2], 2);
		uint8_t* tileMem = &bg_tile_data[64 * tileInfo.tile_nr];
		palette_color = tileMem[tile_pixel_coords.x + tile_pixel_coords.y * 8];
		tileInfo.palette = 0;
	}
	else {	//palette 16/16
		memcpy(&tileInfo, &bg_map_base[tile_nr * 2], 2);
		uint8_t* tileMem = &bg_tile_data[32 * tileInfo.tile_nr];

		//horizontal and vertical flip
		tile_pixel_coords.x = tileInfo.h_flip ? (7 - tile_pixel_coords.x) : tile_pixel_coords.x;
		tile_pixel_coords.y = tileInfo.v_flip ? (7 - tile_pixel_coords.y) : tile_pixel_coords.y;

		uint16_t pixel_index = tile_pixel_coords.x + tile_pixel_coords.y * 8;
		palette_color = tileMem[pixel_index / 2];
		palette_color >>= ((pixel_index & 1) * 4);
		palette_color &= 0xf;
	}

	uint8_t alpha = 255;
	if (palette_color == 0) alpha = 0;

	gba_palette_color gba_color = palette[tileInfo.palette * 16 + palette_color];
	color.r = gba_color.r * 8;
	color.g = gba_color.g * 8;
	color.b = gba_color.b * 8;
	color.a = alpha;
}

void LcdController::bg_transform_pixel_coords(vector2 src_coords, V2Int& dst_coords,
	Transf_Gba_Matrix& gba_matrix, V2Int& bgSize) {

	Matrix2x2 matrix = {
		-1.0 * (gba_matrix.A.sign * 2 - 1) * gba_matrix.A.integer + gba_matrix.A.fract / 256.0,
		-1.0 * (gba_matrix.B.sign * 2 - 1) * gba_matrix.B.integer + gba_matrix.B.fract / 256.0,
		-1.0 * (gba_matrix.C.sign * 2 - 1) * gba_matrix.C.integer + gba_matrix.C.fract / 256.0,
		-1.0 * (gba_matrix.D.sign * 2 - 1) * gba_matrix.D.integer + gba_matrix.D.fract / 256.0
	};

	vector2 p0 = { bgSize.x / 2, bgSize.y / 2 };
	vector2 p01 = { (float)src_coords.x - p0.x, (float)src_coords.y - p0.y };
	dst_coords = { (int)(matrix.A * p01.x + matrix.B * p01.y + p0.x),
		(int)(matrix.C * p01.x + matrix.D * p01.y + p0.y) };
}

void LcdController::getObjLayerScanline(helperParams& params, graphicsScanline** layers, int &activeLayers, uint8_t* windowObjMask) {
	
	if (!params.DISPCNT.obj_enable || activeLayers > 4) {
		return;
	}

	//create a new scanline layer
	graphicsScanline* obj_scanline = new graphicsScanline();
	layers[activeLayers] = obj_scanline;
	activeLayers++;

	memset(windowObjMask, 0, 240);

	obj_attribute* spriteAttributes = (obj_attribute*)params.oam_copy;
	graphicsScanline objRowBuffer;	//allocate 240 horizontal pixels
	int bufferLen;
	
	if (params.vCount >= 160)
		return;

	//for each sprite
	for (int i = 127; i >= 0; i--) {
		obj_attribute& currentObjAttr = spriteAttributes[i];	//get the object attribute
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
		//get the pixels visible in the current scanline
		getSpriteScanline(currentObjAttr, spriteSize, spriteRowToDraw, &objRowBuffer, params);

		//the sprite is a object window?
		if (currentObjAttr.obj_mode == 2 && params.DISPCNT.obj_wnd_enable) {
			//print the visible pixel in the obj window mask
			for (int pixel = 0; pixel < 240; pixel++) {
				if (objRowBuffer.scanline[pixel].color.a == 0)	//ignore transparent pixels
					continue;

				windowObjMask[pixel] = 1;
			}
			continue;
		}

		//copy the sprite row in the frame buffer
		for (int pixel = 0; pixel < 240; pixel++) {

			if (objRowBuffer.scanline[pixel].color.a == 0)	//ignore transparent pixels
				continue;

			obj_scanline->scanline[pixel] = objRowBuffer.scanline[pixel];
			obj_scanline->type = LayerType::OBJ;
		}
	}
}

void LcdController::getSpriteScanline(obj_attribute& attr, V2Int &spriteSize, int rowToDraw, graphicsScanline* objRowBuffer, helperParams& params) {
	
	memset(objRowBuffer, 0, sizeof(graphicsScanline));

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
			//at the end of each sprite attribute there are 16 bits of data containing one of 
			//the 4 values of the transformation matrix (A, B, C, D).
			//To get the whole matrix you have to get the last 2 bytes of 4 consecutive object attributes.
			//In memory there is space for 128 sprite attributes so there is also space for 128/4=32 transformation matrixes
			//the variable rot_scale_param_select identify which one of the 32 transformation matrixes in memory we need

			uint16_t rot_scale_param_select = (attr.rot_scale_par_sel << 0) | (attr.h_flip << 3) | (attr.v_flip << 4);

			transform_matrix = {
				spriteAttributes[rot_scale_param_select * 4].rot_scale_param,
				spriteAttributes[rot_scale_param_select * 4 + 1].rot_scale_param,
				spriteAttributes[rot_scale_param_select * 4 + 2].rot_scale_param,
				spriteAttributes[rot_scale_param_select * 4 + 3].rot_scale_param,
			};
			transformPixelCoords({ i - x_coord, rowToDraw }, transformedCoords, transform_matrix, spriteSize, double_size);
		}
		else {
			transformedCoords = { i - x_coord, rowToDraw };
			//horizontal and vertical flip
			transformedCoords.x = attr.h_flip ? (spriteSize.x - 1 - transformedCoords.x) : transformedCoords.x;
			transformedCoords.y = attr.v_flip ? (spriteSize.y - 1 - transformedCoords.y) : transformedCoords.y;
		}

		// sprite pixel is outside the sprite mem
		if (transformedCoords.x < 0 || transformedCoords.y < 0 ||
			transformedCoords.x >= spriteSize.x || transformedCoords.y >= spriteSize.y) {
			continue;
		}
		
		// get sprite pixel color
		getSpritePixel(attr, params, transformedCoords, objRowBuffer->scanline[i].color);
		objRowBuffer->scanline[i].option.priority = attr.priority;
		objRowBuffer->scanline[i].option.alphaBlending = attr.obj_mode == 0b1;		//semi-transparent
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
			uint16_t x_pixel_index = pixelCoords.x % 8;
			uint8_t palette_entry = tileRowMem[x_pixel_index / 2] >> ((x_pixel_index & 1) * 4);
			palette_entry &= 0xf;
			if(palette_entry == 0) alpha = 0;

			gba_palette_color gba_color = palette[palette_entry];
			color.r = gba_color.r * 8;
			color.g = gba_color.g * 8;
			color.b = gba_color.b * 8;
			color.a = alpha;
		}
	}
}

inline float fpd_16_To_float(gba_16_fpd fpd) {
	if (fpd.sign) {
		uint16_t temp;
		memcpy(&temp, &fpd, 2);
		temp ^= 0xffff;
		temp += 1;
		memcpy(&fpd, &temp, 2);
		return -1.0 * (fpd.integer + fpd.fract / 256.0);
	}
	return fpd.integer + fpd.fract / 256.0;
}

void LcdController::transformPixelCoords(V2Int src_coords, V2Int& dst_coords, Transf_Gba_Matrix& gba_matrix, V2Int& spriteSize, int doubleSize) {
	Matrix2x2 matrix = {
		fpd_16_To_float(gba_matrix.A),
		fpd_16_To_float(gba_matrix.B),
		fpd_16_To_float(gba_matrix.C),
		fpd_16_To_float(gba_matrix.D)
	};

	vector2 p0 = { spriteSize.x / 2, spriteSize.y / 2 };
	vector2 p01 = { (float)src_coords.x - p0.x * doubleSize, (float)src_coords.y - p0.y * doubleSize };
	dst_coords = { (int)(matrix.A * p01.x + matrix.B * p01.y + p0.x),
		(int)(matrix.C * p01.x + matrix.D * p01.y + p0.y) };

}

void LcdController::update_V_count(uint32_t cycles) {
	video_cnt += cycles;
	h_cnt = video_cnt / 4;

	//v-counter irq
	if (DISPSTAT->vcounter_irq_enable && DISPSTAT->LYC == *VCOUNT) {
		GBA::irq.setVCounterFlag();
	}

	//v-blank (nothing to display)
	if (*VCOUNT >= 160) {
		if (!DISPSTAT->vblank_flag) {
			DISPSTAT->vblank_flag = 1;
			if(DISPSTAT->vblank_irq_enable)
				GBA::irq.setVBlankFlag();
			GBA::memory.trigger_dma(Dma_Trigger::VBLANK);
		}
		if (video_cnt <= 960) {//h-draw in v-blank
			DISPSTAT->hblank_flag = 0;
		}
		else {//h-blank in v-blank
			DISPSTAT->hblank_flag = 1;
			if (video_cnt > 1232) {	//end of h-blank
				video_cnt %= 1232;
				*VCOUNT += 1;
			}
		}
		if (*VCOUNT >= 228) {//end of v-blank
			*VCOUNT %= 228;
			DISPSTAT->vblank_flag = 0;	//v-draw
			activeFrameBuffer = 1 - activeFrameBuffer;	//change frame buffer
			memset(frameBuffers[activeFrameBuffer], 0, 240 * 160 * 4);	//clean the buffer
		}
		return;
	}

	//h-draw
	if (video_cnt <= 960) {
		//first time in h-draw
		if (DISPSTAT->hblank_flag) {
			DISPSTAT->hblank_flag = 0;

			//draw the current scanline
			drawer->Wait();	//wait for the helper to finish the previous job

			//sets some registers
			drawerParams.DISPCNT = *DISPCNT;
			drawerParams.DISPSTAT = *DISPSTAT;

			drawerParams.WIN0H = *WIN0H;
			drawerParams.WIN1H = *WIN1H;
			drawerParams.WIN0V = *WIN0V;
			drawerParams.WIN1V = *WIN1V;
			drawerParams.WININ = *WININ;
			drawerParams.WINOUT = *WINOUT;

			drawerParams.vCount = *VCOUNT;
			drawerParams.BLDALPHA = *BLDALPHA;
			drawerParams.BLDCNT = *BLDCNT;
			drawerParams.BLDY = *BLDY;
			for (int i = 0; i < 4; i++) drawerParams.BGCNT[i] = BG0CNT[i];
			for (int i = 0; i < 4; i++) drawerParams.BG_OFFSETS[i] = BG_OFFSETS[i];
			drawerParams.GB2_REF_POINT = *GB2_REF_POINT;
			drawerParams.GB3_REF_POINT = *GB3_REF_POINT;
			drawerParams.BG2_TRANSF_MATRIX = *BG2_TRANSF_MATRIX;
			drawerParams.BG3_TRANSF_MATRIX = *BG3_TRANSF_MATRIX;

			drawerParams.screenBuffer = frameBuffers[activeFrameBuffer];
			drawer->startWork(1, helperRoutine, &drawerParams);	//start the new job
		}
	}else {	//h-blank
		if (!DISPSTAT->hblank_flag) {	//first time in h-blank
			DISPSTAT->hblank_flag = 1;
			GBA::memory.trigger_dma(Dma_Trigger::HBLANK);
			if(DISPSTAT->hblank_irq_enable)
				GBA::irq.setHBlankFlag();	//h-blank irq
		}

		if (video_cnt > 1232) {	//end of h-blank
			video_cnt %= 1232;
			*VCOUNT += 1;
		}
	}
}


void LcdController::update() {

}