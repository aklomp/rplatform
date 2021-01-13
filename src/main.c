#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "clock.h"
#include "led.h"

static void init (void)
{
	clock_init();
	led_init();
}

static void loop (void)
{
	for (;;) {

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
