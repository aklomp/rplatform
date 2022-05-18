#include <string.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "anim_engaged.h"
#include "anim_rotate.h"
#include "display.h"
#include "drv8833.h"
#include "event.h"
#include "ht1621.h"
#include "version.h"

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
	[DISPLAY_CHAR_0]      = SA | SB | SC | SD | SE | SF     ,
	[DISPLAY_CHAR_1]      =      SB | SC                    ,
	[DISPLAY_CHAR_2]      = SA | SB |      SD | SE |      SG,
	[DISPLAY_CHAR_3]      = SA | SB | SC | SD |           SG,
	[DISPLAY_CHAR_4]      =      SB | SC |           SF | SG,
	[DISPLAY_CHAR_5]      = SA |      SC | SD |      SF | SG,
	[DISPLAY_CHAR_6]      = SA |      SC | SD | SE | SF | SG,
	[DISPLAY_CHAR_7]      = SA | SB | SC                    ,
	[DISPLAY_CHAR_8]      = SA | SB | SC | SD | SE | SF | SG,
	[DISPLAY_CHAR_9]      = SA | SB | SC | SD |      SF | SG,
	[DISPLAY_CHAR_A]      = SA | SB | SC |      SE | SF | SG,
	[DISPLAY_CHAR_B]      =           SC | SD | SE | SF | SG,
	[DISPLAY_CHAR_C]      = SA |           SD | SE | SF     ,
	[DISPLAY_CHAR_D]      =      SB | SC | SD | SE |      SG,
	[DISPLAY_CHAR_E]      = SA |           SD | SE | SF | SG,
	[DISPLAY_CHAR_F]      = SA |                SE | SF | SG,
	[DISPLAY_CHAR_I]      =      SB | SC                    ,
	[DISPLAY_CHAR_L]      =                SD | SE | SF     ,
	[DISPLAY_CHAR_N]      = SA | SB | SC |      SE | SF     ,
	[DISPLAY_CHAR_O]      = SA | SB | SC | SD | SE | SF     ,
	[DISPLAY_CHAR_R]      = SA |                SE | SF     ,
	[DISPLAY_CHAR_S]      = SA |      SC | SD |      SF | SG,
	[DISPLAY_CHAR_T]      =                SD | SE | SF | SG,
	[DISPLAY_CHAR_U]      =      SB | SC | SD | SE | SF     ,
	[DISPLAY_CHAR_DEGREE] = SA | SB |                SF | SG,
	[DISPLAY_CHAR_EMPTY]  =                                0,
	[DISPLAY_CHAR_MINUS]  =                               SG,
	[DISPLAY_CHAR_SA]     = SA                              ,
	[DISPLAY_CHAR_SB]     =      SB                         ,
	[DISPLAY_CHAR_SC]     =           SC                    ,
	[DISPLAY_CHAR_SD]     =                SD               ,
	[DISPLAY_CHAR_SE]     =                     SE          ,
	[DISPLAY_CHAR_SF]     =                          SF     ,
	[DISPLAY_CHAR_SG]     =                               SG,
};

// Flash messages.
static const uint8_t flash_msg[][6] = {
	[DISPLAY_FLASH_COARSE] = {
		DISPLAY_CHAR_C,
		DISPLAY_CHAR_O,
		DISPLAY_CHAR_A,
		DISPLAY_CHAR_R,
		DISPLAY_CHAR_S,
		DISPLAY_CHAR_E
	},
	[DISPLAY_FLASH_FAULT] = {
		DISPLAY_CHAR_F,
		DISPLAY_CHAR_A,
		DISPLAY_CHAR_U,
		DISPLAY_CHAR_L,
		DISPLAY_CHAR_T,
		DISPLAY_CHAR_EMPTY
	},
	[DISPLAY_FLASH_FINE] = {
		DISPLAY_CHAR_EMPTY,
		DISPLAY_CHAR_F,
		DISPLAY_CHAR_I,
		DISPLAY_CHAR_N,
		DISPLAY_CHAR_E,
		DISPLAY_CHAR_EMPTY
	},
	[DISPLAY_FLASH_SENSOR] = {
		DISPLAY_CHAR_S,
		DISPLAY_CHAR_E,
		DISPLAY_CHAR_N,
		DISPLAY_CHAR_S,
		DISPLAY_CHAR_O,
		DISPLAY_CHAR_R,
	},
};

// Display mode.
static enum {
	MODE_FLASH,
	MODE_NORMAL,
	MODE_VERSION,
} mode = MODE_VERSION;

// Normal and Flash display buffers, consisting of character codes.
static struct {
	uint8_t flash[6];
	uint8_t normal[6];
} digit;

// Step counter, counts steps per mode.
static uint8_t step;

// Extra flags to enable during a flash message.
static uint8_t flash_flags;

void sys_tick_handler (void)
{
	event_raise(EVENT_DISPLAY_STEP);
}

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

static void redraw_normal (void)
{
	digit.normal[0] = anim_rotate;
	draw_chars(digit.normal, anim_engaged);
}

static void redraw_flash (void)
{
	draw_chars(digit.flash, anim_engaged | flash_flags);
}

static void display_normal (void)
{
	// Go into Normal mode.
	mode = MODE_NORMAL;
	step = 0;
	redraw_normal();
}

void display_flash (const enum DisplayFlash msg)
{
	// Go into Flash mode.
	mode = MODE_FLASH;
	step = 0;
	flash_flags = 0;
	memcpy(digit.flash, flash_msg[msg], sizeof (digit.flash));
	redraw_flash();
}

void display_version (void)
{
	// Go into Version mode.
	mode = MODE_VERSION;
	step = 0;

	// Convert the version to display characters.
	for (uint32_t i = 0; i < sizeof (digit.normal); i++) {
		const char v = version[i];

		digit.normal[i] = v - ((v >= '0' && v <= '9') ? '0' : 'a' - 10);
	}

	// Display the version.
	draw_chars(digit.normal, 0);
}

void display_update (void)
{
	int16_t speed  = drv8833_speed_get();
	uint8_t uspeed = speed < 0 ? -speed : speed;

	digit.normal[1] = 0;			// Intentional leading zero
	digit.normal[2] = (uspeed / 100) % 10;	// Hundreds
	digit.normal[3] = (uspeed /  10) % 10;	// Tens
	digit.normal[4] = (uspeed /   1) % 10;	// Ones
	digit.normal[5] = DISPLAY_CHAR_EMPTY;	// Right-padding

	// Replace leading zeroes with the sign.
	for (uint32_t i = 1; i < sizeof (digit.normal) - 2; i++) {
		if (digit.normal[i + 1] != 0) {
			digit.normal[i] = speed < 0 ? DISPLAY_CHAR_MINUS : DISPLAY_CHAR_EMPTY;
			break;
		}
		digit.normal[i] = DISPLAY_CHAR_EMPTY;
	}

	// Go into Normal mode.
	display_normal();
}

void display_step (void)
{
	// Step the animations.
	anim_engaged_step();
	anim_rotate_step();

	// Increment the step counter.
	step++;

	switch (mode) {
	case MODE_NORMAL:
		redraw_normal();

		// After Normal mode has been running uninterruptedly for 11
		// seconds, issue a temperature request and subsequent
		// temperature response event. Repeat on a 12.8 s interval.
		if ((step & 0x7F) == 110)
			event_raise(EVENT_TEMPERATURE_REQUEST);

		if ((step & 0x7F) == 120)
			event_raise(EVENT_TEMPERATURE_RESPONSE);

		return;

	case MODE_FLASH:

		// Exit Flash mode after a set number of steps.
		step < 10 ? redraw_flash() : display_normal();
		break;

	case MODE_VERSION:

		// Exit Version mode after a set number of steps.
		if (step == 8)
			display_update();
		break;
	}
}

void display_init (void)
{
	// Setup the SysTick timer to tick at 10 Hz. This provides the timing
	// for animation frames.
	STK_CSR = 0;
	STK_CVR = STK_RVR = 900000 - 1;		// 10 Hz @ 9 MHz
	STK_CSR
		= STK_CSR_CLKSOURCE_AHB_DIV8	// 9 MHz
		| STK_CSR_TICKINT		// Enable interrupt
		| STK_CSR_ENABLE
		;

	// Setup the interrupt to trigger an event when the SysTick crosses
	// through zero.
	nvic_enable_irq(NVIC_SYSTICK_IRQ);

	// Do not draw anything to the display, because the ht1621 (LCD driver
	// chip) needs to be initialized first before writes can be performed.
}

void display_temperature (const int16_t temp)
{
	const uint8_t utemp = temp < 0 ? -temp / 16 : temp / 16;
	const uint8_t frac  = temp < 0 ? -temp % 16 : temp % 16;

	digit.flash[0] = (utemp / 10) % 10;
	digit.flash[1] = (utemp /  1) % 10;
	digit.flash[2] = (frac * 100 / 16 / 10) % 10;
	digit.flash[3] = (frac * 100 / 16 /  1) % 10;
	digit.flash[4] = DISPLAY_CHAR_DEGREE;
	digit.flash[5] = DISPLAY_CHAR_C;

	// Replace the first digit with the sign, if the digit is unused.
	if (digit.flash[0] == 0)
		digit.flash[0] = temp < 0 ? DISPLAY_CHAR_MINUS : DISPLAY_CHAR_EMPTY;

	// Go into Flash mode.
	mode = MODE_FLASH;
	step = 0;
	flash_flags = DISPLAY_FLAG_COLON;
	redraw_flash();
}
