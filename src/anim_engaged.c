#include "anim_engaged.h"
#include "display.h"
#include "drv8833.h"

static enum State {
	STATE_OFF,
	STATE_STEP1,
	STATE_STEP2,
	STATE_STEP3,
	STATE_STEP4,
} state;

static const enum State map_next_ccw[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP2,
	[STATE_STEP2] = STATE_STEP3,
	[STATE_STEP3] = STATE_STEP4,
	[STATE_STEP4] = STATE_STEP1,
};

static const enum State map_next_cw[] = {
	[STATE_OFF]   = STATE_OFF,
	[STATE_STEP1] = STATE_STEP4,
	[STATE_STEP2] = STATE_STEP1,
	[STATE_STEP3] = STATE_STEP2,
	[STATE_STEP4] = STATE_STEP3,
};

static const uint8_t map_flags[] = {
	[STATE_OFF]   = 0,
	[STATE_STEP1] = DISPLAY_FLAG_T1,
	[STATE_STEP2] = DISPLAY_FLAG_T1 | DISPLAY_FLAG_T2,
	[STATE_STEP3] = DISPLAY_FLAG_T1 | DISPLAY_FLAG_T3,
	[STATE_STEP4] = DISPLAY_FLAG_T1 | DISPLAY_FLAG_T4,
};

uint8_t anim_engaged = 0;

extern void anim_engaged_step (void)
{
	state = drv8833_speed_get() >= 0
		? map_next_cw[state]
		: map_next_ccw[state];
	anim_engaged = map_flags[state];
}

void anim_engaged_off (void)
{
	state = STATE_OFF;
	anim_engaged = map_flags[state];
}

void anim_engaged_on (void)
{
	state = STATE_STEP1;
	anim_engaged = map_flags[state];
}
