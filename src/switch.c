#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>

#include "event.h"
#include "switch.h"

void exti0_isr (void)
{
	exti_reset_request(EXTI0);
	event_raise(EVENT_LEFT_SWITCH_DOWN);
}

void exti9_5_isr (void)
{
	if (exti_get_flag_status(EXTI9)) {
		exti_reset_request(EXTI9);
		event_raise(gpio_get(GPIOB, GPIO9)
				? EVENT_RIGHT_SWITCH_UP
				: EVENT_RIGHT_SWITCH_DOWN);
	}
}

static void init_switch (const uint32_t gpio, const uint8_t irqn)
{
	// Setup the GPIO as floating input. The switches are debounced in
	// hardware and the input pins have Schmitt triggers.
	gpio_set_mode
		( GPIOB
		, GPIO_MODE_INPUT
		, GPIO_CNF_INPUT_FLOAT
		, gpio
		) ;

	// Setup an external interrupt, using the fact that GPIOn is defined
	// with the same bitmask as EXTIn.
	exti_select_source  (gpio, GPIOB);
	exti_enable_request (gpio);

	// Enable the interrupt line.
	nvic_enable_irq(irqn);
}

void switch_init (void)
{
	// Left switch is PB0.
	init_switch(GPIO0, NVIC_EXTI0_IRQ);
	exti_set_trigger(GPIO0, EXTI_TRIGGER_FALLING);

	// Right switch is PB9.
	init_switch(GPIO9, NVIC_EXTI9_5_IRQ);
	exti_set_trigger(GPIO9, EXTI_TRIGGER_BOTH);

	// Force an event on the right switch to honor the initial setting.
	EXTI_SWIER = GPIO9;
}
