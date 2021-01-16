#include <stdbool.h>
#include <stdint.h>

#include "bitband.h"
#include "event.h"

static volatile uint32_t flags;

void event_raise (const enum Event event)
{
	*bitband_sram(&flags, event) = 1;
}

bool event_test_and_clear (const enum Event event)
{
	const bool isset = flags & (1 << event);

	if (isset)
		*bitband_sram(&flags, event) = 0;

	return isset;
}
