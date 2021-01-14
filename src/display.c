#include "display.h"
#include "ht1621.h"

// Device bitmaps for segments.
enum Segment {
	SA = 1 << 7,
	SB = 1 << 6,
	SC = 1 << 5,
	SD = 1 << 0,
	SE = 1 << 1,
	SF = 1 << 3,
	SG = 1 << 2,
};

// Device bitmaps for characters.
static const uint8_t chars[] = {
	[DISPLAY_CHAR_0]     = SA | SB | SC | SD | SE | SF     ,
	[DISPLAY_CHAR_1]     =      SB | SC                    ,
	[DISPLAY_CHAR_2]     = SA | SB |      SD | SE |      SG,
	[DISPLAY_CHAR_3]     = SA | SB | SC | SD |           SG,
	[DISPLAY_CHAR_4]     =      SB | SC |           SF | SG,
	[DISPLAY_CHAR_5]     = SA |      SC | SD |      SF | SG,
	[DISPLAY_CHAR_6]     = SA |      SC | SD | SE | SF | SG,
	[DISPLAY_CHAR_7]     = SA | SB | SC                    ,
	[DISPLAY_CHAR_8]     = SA | SB | SC | SD | SE | SF | SG,
	[DISPLAY_CHAR_9]     = SA | SB | SC | SD |      SF | SG,
	[DISPLAY_CHAR_EMPTY] =                                0,
	[DISPLAY_CHAR_MINUS] =                               SG,
};

static void draw_chars (const uint8_t *buf, const uint8_t flags)
{
	// Device bitmap.
	uint8_t bitmap[7] = { 0 };

	// Convert symbolic characters to device bitmap values. Each character
	// is split over two bytes in the device bitmap.
	for (uint8_t i = 0; i < 6; i++) {
		bitmap[i + 0] |= chars[buf[i]] & 0x0F;
		bitmap[i + 1] |= chars[buf[i]] & 0xF0;
	}

	// Convert flags to device bitmap values.
	if (flags & DISPLAY_FLAG_COLON)
		bitmap[2] |= 0x10;

	if (flags & DISPLAY_FLAG_DOT)
		bitmap[4] |= 0x10;

	if (flags & DISPLAY_FLAG_T1)
		bitmap[0] |= 0x80;

	if (flags & DISPLAY_FLAG_T2)
		bitmap[0] |= 0x40;

	if (flags & DISPLAY_FLAG_T3)
		bitmap[0] |= 0x20;

	if (flags & DISPLAY_FLAG_T4)
		bitmap[0] |= 0x10;

	// Display the bitmap.
	ht1621_draw(bitmap);
}

void display_speed (const int16_t speed)
{
	uint8_t uspeed  = speed < 0 ? -speed : speed;
	uint8_t digit[] = {
		[0] = 0,			// Intentional leading zero
		[1] = 0,			// Intentional leading zero
		[2] = (uspeed / 100) % 10,	// Hundreds
		[3] = (uspeed /  10) % 10,	// Tens
		[4] = (uspeed /   1) % 10,	// Ones
		[5] = DISPLAY_CHAR_EMPTY,	// Right-padding
	};

	// Replace leading zeroes with the sign.
	for (uint32_t i = 0; i < sizeof (digit) - 2; i++) {
		if (digit[i + 1] != 0) {
			digit[i] = speed < 0 ? DISPLAY_CHAR_MINUS : DISPLAY_CHAR_EMPTY;
			break;
		}
		digit[i] = DISPLAY_CHAR_EMPTY;
	}

	draw_chars(digit, 0);
}
