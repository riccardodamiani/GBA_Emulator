#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H

#include <cstdint>

#include "multithreadManager.h" 

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

struct helperParams {
	dispCnt_struct DISPCNT;
	dispStat_struct DISPSTAT;
	uint16_t vCount;
	uint8_t *palette_copy;
	uint8_t *oam_copy;
	uint8_t *vram_copy;
	uint8_t *screenBUffer;
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
