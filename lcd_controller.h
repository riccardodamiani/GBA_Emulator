#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H

#include <cstdint>

#include "multithreadManager.h" 

struct V2Int {
	int32_t x, y;
};

V2Int const sprites_tiles_table[3][4] = {
	{{8, 8}, {16, 16}, {32, 32}, {64, 64}},
	{{16, 8}, {32, 8}, {32, 16}, {64, 32}},
	{{8, 16}, {8, 32}, {16, 32}, {32, 64}}
};

struct vector2 {
	float x, y;
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

struct bg_offset_struct {
	uint16_t offset : 9,	// (0-511)
		not_used : 7;
};

struct obj_window_size {
	uint16_t c2 : 8,
		c1 : 8;
};

struct window_control {
	uint16_t w0_layer_enable : 5,
		w0_spec_eff : 1,
		not_used : 2,
		w1_layer_enable : 5,
		w1_spec_eff : 1,
		not_used_2 : 2;
};

struct obj_window_control {
	uint16_t out_layer_enable : 5,
		out_spec_eff : 1,
		not_used : 2,
		obj_win_layer_enable : 5,
		obj_win_spec_eff : 1,
		not_used_2 : 2;
};

struct bg_scrolling_struct {
	bg_offset_struct HOFS, VOFS;
};

struct BGCNT_struct {
	uint16_t bg_priority : 2,	// (0-3, 0=Highest)
		ch_base_block : 2,	//(0-3, in units of 16 KBytes) (=BG Tile Data)
		not_used : 2,	//used only in NDS
		mosaic : 1,
		palette : 1,	// (0=16/16, 1=256/1)
		screen_base_block : 5,	//(0-31, in units of 2 KBytes) (=BG Map Data)
		display_overflow : 1,	// Display Area Overflow (0=Transparent, 1=Wraparound)
		screen_size : 2;	// 0-3
/*
Internal Screen Size (dots) and size of BG Map (bytes):
  Value  Text Mode      Rotation/Scaling Mode
  0      256x256 (2K)   128x128   (256 bytes)
  1      512x256 (4K)   256x256   (1K)
  2      256x512 (4K)   512x512   (4K)
  3      512x512 (8K)   1024x1024 (16K)
*/
};

const V2Int const TextModeScreenSize_Trans[4] = {
	{256, 256},
	{512, 256},
	{256, 512},
	{512, 512}
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

struct gba_16_fpd {	//16 bit fixed point decimal
	uint16_t fract : 8,
		integer : 7,
		sign : 1;
};

struct gba_32_fpd {	//32 bit fixed point decimal
	uint32_t fract : 8,
		integer : 19,
		sign : 1,
		unused : 4;
};

struct tile_info_struct {
	uint16_t tile_nr : 10,
		h_flip : 1,
		v_flip : 1,
		palette : 4;
};

struct Transf_Gba_Matrix {
	gba_16_fpd A, B, C, D;
};

struct Matrix2x2 {
	float A, B, C, D;
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
	gba_16_fpd rot_scale_param;
};

enum LayerType {
	NO_TYPE = 0,
	BG0 = 1,
	BG1 = 2,
	BG2 = 4,
	BG3 = 8,
	OBJ = 16,
	BD = 32
};

struct pixelOptionsStruct {
	uint8_t priority : 7,
		alphaBlending : 1;
};

struct SpecialEffectPixel {
	rgba_color color;
	pixelOptionsStruct option;
	LayerType type;
};


struct ScanlinePixel {
	rgba_color color;
	pixelOptionsStruct option;
};

struct graphicsScanline {
	ScanlinePixel scanline[240];
	LayerType type;
};

struct graphicsPixel {
	ScanlinePixel pixel;
	LayerType type;
	pixelOptionsStruct option;
};


//alpha blending parameters
struct BLDALPHA_struct {
	uint32_t eva_coeff : 5,	//ev coefficient for target a
		unused1 : 3,
		evb_coeff : 5,	//ev coefficient for target b
		unused2 : 3;
};// formula: I = MIN ( 31, I1st*EVA + I2nd*EVB )

//brightness coefficient
struct BLDY_struct {
	uint32_t evy_coeff : 5,
		unused : 27;
};

struct BG_reference_point_struct {
	gba_32_fpd x, y;
};

struct helperParams {
	dispCnt_struct DISPCNT;
	dispStat_struct DISPSTAT;
	uint16_t vCount;
	uint16_t BLDCNT;
	obj_window_size WIN0H, WIN1H, WIN0V, WIN1V;
	window_control WININ;
	obj_window_control WINOUT;
	BLDALPHA_struct BLDALPHA;
	BLDY_struct BLDY;
	bg_scrolling_struct BG_OFFSETS[4];
	BGCNT_struct BGCNT[4];
	BG_reference_point_struct GB2_REF_POINT, GB3_REF_POINT;
	Transf_Gba_Matrix BG2_TRANSF_MATRIX, BG3_TRANSF_MATRIX;

	uint8_t *palette_copy;
	uint8_t *oam_copy;
	uint8_t *vram_copy;
	uint8_t *screenBuffer;
};

class LcdController {
public:
	LcdController();
	~LcdController();
	void update_V_count(uint32_t cycles);
	void update();
	const uint32_t const* getBufferToRender();
	static bool activeBg(helperParams& params, int bg_nr);
	static void helperRoutine(int start_index, int end_index, void *args);
	static void getObjLayerScanline(helperParams& params, graphicsScanline** layers, int& activeLayers, uint8_t* windowObjMask);
	static void getSpriteScanline(obj_attribute& attr, V2Int& spriteSize, int line, graphicsScanline* objRowBuffer, helperParams& params);
	static void transformPixelCoords(V2Int src_coords, V2Int& dst_coords, Transf_Gba_Matrix& gba_matrix, V2Int& spriteSize, int doubleSize);
	static void getSpritePixel(obj_attribute& attr, helperParams& params, V2Int coords, rgba_color& color);

	static void get_bg_layer_scanline(helperParams& params, graphicsScanline** layers, int& activeLayers);
	static void background_mode0(helperParams& params, graphicsScanline** layers, int& asctiveLayers);
	static void background_mode2(helperParams& params, graphicsScanline** layers, int& asctiveLayers);
	static void bg_transform_pixel_coords(vector2 src_coords, V2Int& dst_coords, Transf_Gba_Matrix& gba_matrix, V2Int& bgSize);
	static void get_affine_bg_pixel_color(int bg_num, helperParams& params, V2Int coords, rgba_color& color, V2Int& bgSize);
	static void get_text_bg_pixel_color(int bg_num, helperParams& params, V2Int coords, rgba_color& color, V2Int& bgSize);

	static void apply_special_effects(helperParams& params, SpecialEffectPixel& lowerPixel, graphicsPixel& upperPixel, SpecialEffectPixel& finalPixel);
	static void order_bg_scanlines(graphicsScanline** layers, int activeLayers);
	static void order_layer_pixels(graphicsPixel** layer_pixels, int activeLayers);
	inline static uint8_t get_bg_window_mask(helperParams& params, LayerType type, uint8_t objWindowMask, uint16_t x_coord, uint16_t y_coord);

private:
	uint32_t video_cnt;
	uint32_t h_cnt;
	dispCnt_struct* DISPCNT;
	dispStat_struct* DISPSTAT;
	uint16_t* VCOUNT;
	uint16_t* BLDCNT;
	obj_window_size *WIN0H, *WIN1H, *WIN0V, *WIN1V;
	window_control *WININ;
	obj_window_control *WINOUT;
	BLDALPHA_struct *BLDALPHA;
	BLDY_struct *BLDY;
	BGCNT_struct* BG0CNT;
	bg_scrolling_struct* BG_OFFSETS;
	BG_reference_point_struct *GB2_REF_POINT, *GB3_REF_POINT;
	Transf_Gba_Matrix *BG2_TRANSF_MATRIX, *BG3_TRANSF_MATRIX;

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
