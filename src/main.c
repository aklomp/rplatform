#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "clock.h"
#include "display.h"
#include "event.h"
#include "drv8833.h"
#include "event.h"
#include "ht1621.h"
#include "led.h"
#include "rotary.h"
#include "switch.h"

static void init (void)
{
	clock_init();
	led_init();
	display_init();
	rotary_init();
	switch_init();
	ht1621_init();
	drv8833_init();

	display_update();
}

static void loop (void)
{
	bool coarse = true;

	for (;;) {

		if (event_test_and_clear(EVENT_DISPLAY_STEP))
			display_step();

		if (event_test_and_clear(EVENT_FAULT_END))
			led_off();

		if (event_test_and_clear(EVENT_FAULT_START)) {
			led_on();
			display_flash(DISPLAY_FLASH_FAULT);
		}

		if (event_test_and_clear(EVENT_LEFT_SWITCH_DOWN))
			(coarse = !coarse)
				? display_flash(DISPLAY_FLASH_COARSE)
				: display_flash(DISPLAY_FLASH_FINE);

		if (event_test_and_clear(EVENT_RIGHT_SWITCH_DOWN))
			drv8833_run();

		if (event_test_and_clear(EVENT_RIGHT_SWITCH_UP))
			drv8833_pause();

		if (event_test_and_clear(EVENT_ROTARY_CCW)) {
			drv8833_speed_add(coarse ? -10 : -1);
			display_update();
		}

		if (event_test_and_clear(EVENT_ROTARY_CW)) {
			drv8833_speed_add(coarse ? 10 : 1);
			display_update();
		}

		// Wait for the next event. This loop is safe against race
		// conditions from multiple simultaneous events, because unlike
		// interrupts, events are buffered.
		__asm__ ("wfe");
	}
}

int main (void)
{
	init();
	loop();
}
