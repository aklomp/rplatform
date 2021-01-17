#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "clock.h"
#include "ht1621.h"
#include "led.h"
#include "rotary.h"
#include "switch.h"

static void init (void)
{
	clock_init();
	led_init();
	rotary_init();
	switch_init();
	ht1621_init();
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
