#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H

#include <cstdint>

#include "multithreadManager.h" 

struct V2Int {
	uint32_t x, y;
};

V2Int const sprites_tiles_table[3][4] = {
	{{8, 8}, {16, 16}, {32, 32}, {64, 64}},
	{{16, 8}, {32, 8}, {32, 16}, {64, 32}},
	{{8, 16}, {8, 32}, {16, 32}, {32, 64}}
};

struct dispCnt_struct {
	uint16_t bg_mode : 3,	//(0-5=Video Mode 0-5, 6-7=Prohibited)
		cgb_mode : 1,	//(0=GBA, 1=CGB; can be set only by BIOS opcodes)
		disp_frame_select : 1,	//(0-1=Frame 0-1) (for BG Modes 4,5 only)
		hblank_int_free : 1,	//(1=Allow access to OAM during H-Blank)
		obj_vram_map : 1,	//(0 = Two dimensional, 1 = One dimensional)
		forced_blank : 1,		//(1=Allow FAST access to VRAM,Palette,OAM)
		bg0_enable : 1,
		bg1_enable : 1,
		bg2_enable : 1,
		bg3_enable : 1,
		obj_enable : 1,
		wnd0_enable : 1,
		wnd1_enable : 1,
		obj_wnd_enable : 1;
};

struct dispStat_struct {
	uint16_t vblank_flag : 1,	//(Read only) (1=VBlank) (set in line 160..226; not 227)
		hblank_flag : 1,		//(Read only) (1=HBlank) (toggled in all lines, 0..227)
		vcounter_flag : 1,		//(Read only) (1=Match)  (set in selected line)     (R)
		vblank_irq_enable : 1,	//(1=Enable)                          (R/W)
		hblank_irq_enable : 1,		//(1=Enable)                          (R/W)
		vcounter_irq_enable : 1,	//(1=Enable)                          (R/W)
		not_used : 2,
		LYC : 8;	//(0..227)                            (R / W)
};

struct gba_palette_color {
	uint16_t r : 5,
		g : 5,
		b : 5,
		not_used : 1;
};

struct rgba_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct obj_attribute {
	uint16_t y_coord : 8,	//y coord on screen
		rot_scale_flag : 1,	//1 = enable
		// when rot_scale_flag == 1, 1 = double size
		//else 1 = don't display sprite
		double_or_obj_disable : 1,
		obj_mode : 2,	//(0=Normal, 1=Semi-Transparent, 2=OBJ Window, 3=Prohibited)
		obj_mosaic : 1,	//1 = active
		palette : 1,		//(0=16/16, 1=256/1)
		obj_shape : 2;	//(0=Square,1=Horizontal,2=Vertical,3=Prohibited)
	uint16_t x_coord : 9,	//x coord on screen
		//when rot_scale_flag == 1
			//9-13: rotation scale parameter selection
		//else the following:
	rot_scale_par_sel: 3,	//first 3 bit of rotation scale parameter selection
		h_flip : 1,	//horizontal flip
		v_flip : 1,	//vertical flip
		obj_size : 2;	//(0..3, depends on OBJ Shape, see Attr 0)
	/*Size  Square   Horizontal  Vertical
		0     8x8      16x8        8x16
		1     16x16    32x8        8x32
		2     32x32    32x16       16x32
		3     64x64    64x32       32x64*/
	uint16_t tile_number : 10,	//tile number
		priority : 2,	//priority relative to background (0-3; 0=Highest)
		palette_num : 4;	//(0-15) (Not used in 256 color/1 palette mode)
	uint16_t just_to_fill_some_space_because_of_the_stupid_way_object_attribute_memory_was_organized;
};

struct helperParams {
	dispCnt_struct DISPCNT;
	dispStat_struct DISPSTAT;
	uint16_t vCount;
	uint8_t *palette_copy;
	uint8_t *oam_copy;
	uint8_t *vram_copy;
	uint8_t * screenBuffer;
};

class LcdController {
public:
	LcdController();
	~LcdController();
	void update_V_count(uint32_t cycles);
	void update();
	const uint32_t const* getBufferToRender();
	static void helperRoutine(int start_index, int end_index, void *args);
	static void printSprites(helperParams& params);
	static void getSpriteRowMem(obj_attribute& attr, V2Int& spriteSize, int line, rgba_color* objRowBuffer, int& pixel_count, helperParams& params);
private:
	uint32_t video_cnt;
	uint32_t h_cnt;
	dispCnt_struct* DISPCNT;
	dispStat_struct* DISPSTAT;
	uint16_t* VCOUNT;
	uint8_t* frameBuffers[2];
	uint8_t* whiteFrameBuffer;
	uint8_t activeFrameBuffer;
	uint8_t print_new_scanline;

	//helper stuff
	MultithreadManager *drawer;
	helperParams drawerParams;
	uint8_t* oam_copy;
	uint8_t* palette_copy;
	uint8_t* vram_copy;
};

#endif
