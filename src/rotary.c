#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "event.h"
#include "rotary.h"

void tim2_isr (void)
{
	// Check that this is actually an update interrupt.
	if (TIM2_SR & TIM_SR_UIF)
		event_raise(TIM2_CR1 & TIM_CR1_DIR_DOWN
				? EVENT_ROTARY_CW
				: EVENT_ROTARY_CCW);

	// Acknowledge all interrupts.
	TIM2_SR = 0;
}

static void init_timer (void)
{
	// Each movement of the timer generates an update event.
	TIM2_PSC = 0;
	TIM2_ARR = 1;
	TIM2_CNT = 0;

	// Enable the encoder inputs.
	TIM2_CCMR1 = TIM_CCMR1_CC1S_IN_TI1 | TIM_CCMR1_CC2S_IN_TI2;

	// Set Slave Mode to Encoder Mode 1. No filtering, because debouncing
	// is done in hardware and the input pins have Schmitt triggers.
	TIM2_SMCR = TIM_SMCR_SMS_EM1;

	// Generate an update event to refresh all registers.
	TIM2_EGR = TIM_EGR_UG;
	TIM2_SR  = 0;

	// Generate interrupts on update events.
	TIM2_DIER = TIM_DIER_UIE;
	nvic_enable_irq(NVIC_TIM2_IRQ);

	// Start the timer.
	TIM2_CR1 = TIM_CR1_CEN;
}

static void init_gpio (void)
{
	// Setup PA0 as TIM2_CH1 and PA1 as TIM2_CH2.
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_INPUT
		, GPIO_CNF_INPUT_FLOAT
		, GPIO_TIM2_CH1_ETR | GPIO_TIM2_CH2
		) ;
}

void rotary_init (void)
{
	init_gpio();
	init_timer();
}
