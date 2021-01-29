#include "anim_rotate.h"
#include "display.h"
#include "drv8833.h"

static enum State {
	STATE_OFF,
	STATE_STEP1,
	STATE_STEP2,
	STATE_STEP3,
	STATE_STEP4,
	STATE_STEP5,
	STATE_STEP6,
} state = STATE_STEP1;

uint8_t anim_rotate = 0;

static bool coarse = true;

static const enum State map_next_ccw_coarse[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP6,
	[STATE_STEP2] = STATE_STEP1,
	[STATE_STEP3] = STATE_STEP2,
	[STATE_STEP4] = STATE_STEP3,
	[STATE_STEP5] = STATE_STEP4,
	[STATE_STEP6] = STATE_STEP5,
};

static const enum State map_next_ccw_fine[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP4,
	[STATE_STEP2] = STATE_STEP1,
	[STATE_STEP3] = STATE_STEP2,
	[STATE_STEP4] = STATE_STEP3,
};

static const enum State map_next_cw_coarse[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP2,
	[STATE_STEP2] = STATE_STEP3,
	[STATE_STEP3] = STATE_STEP4,
	[STATE_STEP4] = STATE_STEP5,
	[STATE_STEP5] = STATE_STEP6,
	[STATE_STEP6] = STATE_STEP1,
};

static const enum State map_next_cw_fine[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP2,
	[STATE_STEP2] = STATE_STEP3,
	[STATE_STEP3] = STATE_STEP4,
	[STATE_STEP4] = STATE_STEP1,
};

static const uint8_t map_flags_coarse[] = {
	[STATE_OFF]   = DISPLAY_CHAR_EMPTY,
	[STATE_STEP1] = DISPLAY_CHAR_SA,
	[STATE_STEP2] = DISPLAY_CHAR_SB,
	[STATE_STEP3] = DISPLAY_CHAR_SC,
	[STATE_STEP4] = DISPLAY_CHAR_SD,
	[STATE_STEP5] = DISPLAY_CHAR_SE,
	[STATE_STEP6] = DISPLAY_CHAR_SF,
};

static const uint8_t map_flags_fine[] = {
	[STATE_OFF]   = DISPLAY_CHAR_EMPTY,
	[STATE_STEP1] = DISPLAY_CHAR_SA,
	[STATE_STEP2] = DISPLAY_CHAR_SB,
	[STATE_STEP3] = DISPLAY_CHAR_SG,
	[STATE_STEP4] = DISPLAY_CHAR_SF,
};

void anim_rotate_step (void)
{
	const bool cw = drv8833_speed_get() >= 0;

	if (coarse) {
		state = cw ? map_next_cw_coarse[state] : map_next_ccw_coarse[state];
		anim_rotate = map_flags_coarse[state];
	} else {
		state = cw ? map_next_cw_fine[state] : map_next_ccw_fine[state];
		anim_rotate = map_flags_fine[state];
	}
}

void anim_rotate_set_coarse (const bool _coarse)
{
	coarse = _coarse;
	if (state != STATE_OFF)
		state = STATE_STEP1;
}
