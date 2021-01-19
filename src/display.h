#pragma once

#include <stdint.h>

enum DisplayFlag {
	DISPLAY_FLAG_COLON = 1 << 0,
	DISPLAY_FLAG_DOT   = 1 << 1,
	DISPLAY_FLAG_T1    = 1 << 2,
	DISPLAY_FLAG_T2    = 1 << 3,
	DISPLAY_FLAG_T3    = 1 << 4,
	DISPLAY_FLAG_T4    = 1 << 5,
};

enum DisplayChar {
	DISPLAY_CHAR_0,
	DISPLAY_CHAR_1,
	DISPLAY_CHAR_2,
	DISPLAY_CHAR_3,
	DISPLAY_CHAR_4,
	DISPLAY_CHAR_5,
	DISPLAY_CHAR_6,
	DISPLAY_CHAR_7,
	DISPLAY_CHAR_8,
	DISPLAY_CHAR_9,
	DISPLAY_CHAR_A,
	DISPLAY_CHAR_C,
	DISPLAY_CHAR_E,
	DISPLAY_CHAR_F,
	DISPLAY_CHAR_I,
	DISPLAY_CHAR_L,
	DISPLAY_CHAR_N,
	DISPLAY_CHAR_O,
	DISPLAY_CHAR_R,
	DISPLAY_CHAR_S,
	DISPLAY_CHAR_T,
	DISPLAY_CHAR_U,
	DISPLAY_CHAR_DEGREE,
	DISPLAY_CHAR_EMPTY,
	DISPLAY_CHAR_MINUS,
	DISPLAY_CHAR_SA,
	DISPLAY_CHAR_SB,
	DISPLAY_CHAR_SC,
	DISPLAY_CHAR_SD,
	DISPLAY_CHAR_SE,
	DISPLAY_CHAR_SF,
	DISPLAY_CHAR_SG,
};

enum DisplayFlash {
	DISPLAY_FLASH_COARSE,
	DISPLAY_FLASH_FAULT,
	DISPLAY_FLASH_FINE,
	DISPLAY_FLASH_SENSOR,
};

extern void display_flash  (const enum DisplayFlash msg);
extern void display_update (void);
extern void display_step   (void);
extern void display_init   (void);
extern void display_temperature (const int16_t temp);
