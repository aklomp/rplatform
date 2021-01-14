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
	DISPLAY_CHAR_EMPTY,
	DISPLAY_CHAR_MINUS,
};

extern void display_speed (const int16_t speed);