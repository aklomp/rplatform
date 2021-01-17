#pragma once

#include <stdbool.h>

enum Event {
	EVENT_LEFT_SWITCH_DOWN,
	EVENT_RIGHT_SWITCH_DOWN,
	EVENT_RIGHT_SWITCH_UP,
	EVENT_ROTARY_CCW,
	EVENT_ROTARY_CW,

	EVENT_count
};

extern void event_raise (const enum Event);
extern bool event_test_and_clear (const enum Event event);
