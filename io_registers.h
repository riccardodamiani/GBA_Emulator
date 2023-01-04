#ifndef IO_REGISTERS_H
#define IO_REGISTERS_H

struct Io_registers {

	//LCD I/O REGISTERS
	uint16_t DISPCNT;	//lcd control
	uint16_t GREEN_SWAP;	//green swap
	uint16_t DISPSTAT;	//General LCD Status (STAT,LYC)
	uint16_t VCOUNT;	//vertical counter (LY)

	uint16_t BG0CNT;	//BG0 control
	uint16_t BG1CNT;	//BG1 control
	uint16_t BG2CNT;	//BG2 control
	uint16_t BG3CNT;	//BG3 control

	uint16_t BG0HOFS;	//BG0 x-offset
	uint16_t BG0VOFS;	//BG0 y-offset
	uint16_t BG1HOFS;	//BG1 x-offset
	uint16_t BG1VOFS;	//BG1 y-offset
	uint16_t BG2HOFS;	//BG2 x-offset
	uint16_t BG2VOFS;	//BG2 y-offset
	uint16_t BG3HOFS;	//BG3 x-offset
	uint16_t BG3VOFS;	//BG3 y-offset

	uint16_t BG2PA;		//BG2 Rotation/Scaling Parameter A (dx)
	uint16_t BG2PB;		//BG2 Rotation/Scaling Parameter B (dmx)
	uint16_t BG2PC;		//BG2 Rotation/Scaling Parameter C (dy)
	uint16_t BG2PD;		//BG2 Rotation/Scaling Parameter D (dmy)
	uint32_t BG2X;		//BG2 Reference Point X-Coordinate
	uint32_t BG2Y;		//BG2 Reference Point Y-Coordinate

	uint16_t BG3PA;		//BG3 Rotation/Scaling Parameter A (dx)
	uint16_t BG3PB;		//BG3 Rotation/Scaling Parameter B (dmx)
	uint16_t BG3PC;		//BG3 Rotation/Scaling Parameter C (dy)
	uint16_t BG3PD;		//BG3 Rotation/Scaling Parameter D (dmy)
	uint32_t BG3X;		//BG3 Reference Point X-Coordinate
	uint32_t BG3Y;		//BG3 Reference Point Y-Coordinate

	uint16_t WIN0H;		//Window 0 Horizontal Dimensions
	uint16_t WIN1H;		//Window 1 Horizontal Dimensions
	uint16_t WIN0V;		//Window 0 Vertical Dimensions
	uint16_t WIN1V;		//Window 1 Vertical Dimensions
	uint16_t WININ;		//Inside of Window 0 and 1
	uint16_t WINOUT;	//Inside of OBJ Window & Outside of Windows

	uint16_t MOSAIC;	//Mosaic Size
	uint16_t NOT_USED_0;

	uint16_t BLDCNT;	//Color Special Effects Selection
	uint16_t BLDALPHA;	//Alpha Blending Coefficients
	uint16_t BLDY;		//Brightness (Fade-In/Out) Coefficient

	uint8_t NOT_USED_1[10];

	//SOUND REGISTERS
	uint16_t SOUND1CNT_L;		//Channel 1 Sweep register       (NR10)
	uint16_t SOUND1CNT_H;		//Channel 1 Duty/Length/Envelope (NR11, NR12)
	uint16_t SOUND1CNT_X;		//Channel 1 Frequency/Control    (NR13, NR14)
	uint16_t NOT_USED_2;

	uint16_t SOUND2CNT_L;		//Channel 2 Duty/Length/Envelope (NR21, NR22)
	uint16_t NOT_USED_3;
	uint16_t SOUND2CNT_H;		//Channel 2 Frequency/Control    (NR23, NR24)
	uint16_t NOT_USED_4;

	uint16_t SOUND3CNT_L;		//Channel 3 Stop/Wave RAM select (NR30)
	uint16_t SOUND3CNT_H;		//Channel 3 Length/Volume        (NR31, NR32)
	uint16_t SOUND3CNT_X;		//Channel 3 Frequency/Control    (NR33, NR34)
	uint16_t NOT_USED_5;

	uint16_t SOUND4CNT_L;		//Channel 4 Length/Envelope      (NR41, NR42)
	uint16_t NOT_USED_6;
	uint16_t SOUND4CNT_H;		//Channel 4 Frequency/Control    (NR43, NR44)
	uint16_t NOT_USED_7;

	uint16_t SOUNDCNT_L;		//Control Stereo/Volume/Enable   (NR50, NR51)
	uint16_t SOUNDCNT_H;		//Control Mixing/DMA Control
	uint16_t SOUNDCNT_X;		//Control Sound on/off           (NR52)
	uint16_t NOT_USED_8;

	uint16_t SOUNDBIAS;		//Sound PWM Control
	uint8_t NOT_USED_9[6];

	uint8_t WAVE_RAM[0x10];		//Channel 3 Wave Pattern RAM (2 banks!!)
	uint32_t FIFO_A;		//Channel A FIFO, Data 0-3
	uint32_t FIFO_B;		//Channel B FIFO, Data 0-3
	uint8_t NOT_USED_10[8];

	//DMA
	uint32_t DMA0SAD;		//DMA 0 Source Address
	uint32_t DMA0DAD;		//DMA 0 Destination Address
	uint16_t DMA0CNT_L;		//DMA 0 Word Count
	uint16_t DMA0CNT_H;		//DMA 0 Control

	uint32_t DMA1SAD;		//DMA 1 Source Address
	uint32_t DMA1DAD;		//DMA 1 Destination Address
	uint16_t DMA1CNT_L;		//DMA 1 Word Count
	uint16_t DMA1CNT_H;		//DMA 1 Control

	uint32_t DMA2SAD;		//DMA 2 Source Address
	uint32_t DMA2DAD;		//DMA 2 Destination Address
	uint16_t DMA2CNT_L;		//DMA 2 Word Count
	uint16_t DMA2CNT_H;		//DMA 2 Control

	uint32_t DMA3SAD;		//DMA 3 Source Address
	uint32_t DMA3DAD;		//DMA 3 Destination Address
	uint16_t DMA3CNT_L;		//DMA 3 Word Count
	uint16_t DMA3CNT_H;		//DMA 3 Control

	uint8_t NOT_USED_11[0x20];

	//TIMER REGISTERS
	uint16_t TM0CNT_L;		//Timer 0 Counter/Reload
	uint16_t TM0CNT_H;		//Timer 0 Control

	uint16_t TM1CNT_L;		//Timer 1 Counter/Reload
	uint16_t TM1CNT_H;		//Timer 1 Control

	uint16_t TM2CNT_L;		//Timer 2 Counter/Reload
	uint16_t TM2CNT_H;		//Timer 2 Control

	uint16_t TM3CNT_L;		//Timer 3 Counter/Reload
	uint16_t TM3CNT_H;		//Timer 3 Control

	uint8_t NOT_USED_12[0x10];

	//SERIAL COMMUNICATION (1)
	//uint32_t SIODATA32;		//SIO Data (Normal-32bit Mode; shared with below)
	uint16_t SIOMULTI0;		//SIO Data 0 (Parent)    (Multi-Player Mode)
	uint16_t SIOMULTI1;		//SIO Data 1 (1st Child)    (Multi-Player Mode)
	uint16_t SIOMULTI2;		//SIO Data 2 (2nd Child)    (Multi-Player Mode)
	uint16_t SIOMULTI3;		//SIO Data 3 (3rd Child)    (Multi-Player Mode)
	uint16_t SIOCNT;		//SIO Control Register
	uint16_t SIOMLT_SEND_DATA8;	//SIO Data (Local of MultiPlayer; shared below)
	//uint16_t SIODATA8;		//SIO Data (Normal-8bit and UART Mode)

	uint8_t NOT_USED_13[4];

	//KEYPAD INPUT
	uint16_t KEYINPUT;		//Key Status
	uint16_t KEYCNT;		//Key Interrupt Control

	//SERIAL COMMUNICATION (2)
	uint16_t RCNT;		//SIO Mode Select/General Purpose Data
	uint16_t IR;		//Ancient - Infrared Register (Prototypes only)
	uint8_t NOT_USED_14[8];
	uint16_t JOYCNT;		//SIO JOY Bus Control
	uint8_t NOT_USED_15[14];
	uint32_t JOY_RECV;		//SIO JOY Bus Receive Data
	uint32_t JOY_TRANS;		//SIO JOY Bus Transmit Data
	uint16_t JOYSTAT;		//SIO JOY Bus Receive Status
	uint8_t NOT_USED_16[0xa6];

	//INTERRUPT, WAIRSTATES AND POWER DOWN CONTROL
	uint16_t IE;		//Interrupt Enable Register
	uint16_t IF;		//Interrupt Request Flags / IRQ Acknowledge
	uint16_t WAITCNT;		//Game Pak Waitstate Control
	uint16_t NOT_USED_17;
	uint16_t IME;		//Interrupt Master Enable Register
	uint8_t NOT_USED_18[0xf6];

	uint8_t POSTFLG;		//Undocumented - Post Boot Flag
	uint8_t HALTCNT;		//Undocumented - Power Down Control

	uint8_t NOT_USED_19[0xfe];	//0x4000302 - 0x40003ff
	uint8_t NOT_USED_20[0x404];	//0x4000400 - 0x4000803
};	

#endif
