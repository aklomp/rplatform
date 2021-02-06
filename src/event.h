#pragma once

#include <stdbool.h>

enum Event {
	EVENT_BLINK_DONE,
	EVENT_DISPLAY_STEP,
	EVENT_FAULT_END,
	EVENT_FAULT_START,
	EVENT_LEFT_SWITCH_DOWN,
	EVENT_RIGHT_SWITCH_DOWN,
	EVENT_RIGHT_SWITCH_UP,
	EVENT_ROTARY_CCW,
	EVENT_ROTARY_CW,
	EVENT_TEMPERATURE_REQUEST,
	EVENT_TEMPERATURE_RESPONSE,

	EVENT_count
};

extern void event_raise (const enum Event);
extern bool event_test_and_clear (const enum Event event);
