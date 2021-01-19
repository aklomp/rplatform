#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "anim_engaged.h"
#include "anim_rotate.h"
#include "clock.h"
#include "display.h"
#include "ds18b20.h"
#include "event.h"
#include "drv8833.h"
#include "event.h"
#include "ht1621.h"
#include "led.h"
#include "rotary.h"
#include "switch.h"
#include "onewire.h"

static void init (void)
{
	clock_init();
	led_init();
	display_init();
	rotary_init();
	switch_init();
	ht1621_init();
	drv8833_init();
	onewire_init();

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

		if (event_test_and_clear(EVENT_LEFT_SWITCH_DOWN)) {
			(coarse = !coarse)
				? display_flash(DISPLAY_FLASH_COARSE)
				: display_flash(DISPLAY_FLASH_FINE);
			anim_rotate_set_coarse(coarse);
		}

		if (event_test_and_clear(EVENT_RIGHT_SWITCH_DOWN)) {
			drv8833_run();
			anim_engaged_on();
		}

		if (event_test_and_clear(EVENT_RIGHT_SWITCH_UP)) {
			drv8833_pause();
			anim_engaged_off();
		}

		if (event_test_and_clear(EVENT_ROTARY_CCW)) {
			drv8833_speed_add(coarse ? -10 : -1);
			display_update();
		}

		if (event_test_and_clear(EVENT_ROTARY_CW)) {
			drv8833_speed_add(coarse ? 10 : 1);
			display_update();
		}

		if (event_test_and_clear(EVENT_TEMPERATURE_REQUEST))
			ds18b20_request_start();

		if (event_test_and_clear(EVENT_TEMPERATURE_RESPONSE))
			ds18b20_response_start();

		// Allow the DS18B20 module to handle internal OneWire events.
		ds18b20_handle_events();

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
