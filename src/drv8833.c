#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "drv8833.h"
#include "event.h"

#define PIN_FAULT	GPIO10	// PA10 is FAULT pin, active low
#define PIN_SLEEP	GPIO11	// PA11 is SLEEP pin, active low

static int16_t speed = 0;
static bool running = false;

void exti10_isr (void)
{
	if (exti_get_flag_status(EXTI10)) {
		exti_reset_request(EXTI10);
		event_raise(gpio_get(GPIOA, GPIO10)
				? EVENT_FAULT_END
				: EVENT_FAULT_START);
	}
}

static void config_outputs (void)
{
	// If not running, set the output to Brake/Slow Decay.
	uint16_t ch1 = TIM_CCMR1_CC1S_OUT | TIM_CCMR1_OC1M_FORCE_HIGH;
	uint16_t ch2 = TIM_CCMR1_CC2S_OUT | TIM_CCMR1_OC2M_FORCE_HIGH;

	if (running && speed < 0) {
		ch1 = TIM_CCMR1_CC1S_OUT | TIM_CCMR1_OC1M_PWM2;
		ch2 = TIM_CCMR1_CC2S_OUT | TIM_CCMR1_OC2M_FORCE_HIGH;
	} else if (running) {
		ch1 = TIM_CCMR1_CC1S_OUT | TIM_CCMR1_OC1M_FORCE_HIGH;
		ch2 = TIM_CCMR1_CC2S_OUT | TIM_CCMR1_OC2M_PWM2;
	}

	TIM1_CCMR1 = ch1 | ch2;
}

static void wakeup (void)
{
	gpio_set(GPIOA, PIN_SLEEP);
}

void drv8833_run (void)
{
	running = true;
	config_outputs();
}

void drv8833_pause (void)
{
	running = false;
	config_outputs();
}

static inline int16_t clamp (const int16_t n)
{
	if (n < -255)
		return -255;

	if (n > 255)
		return 255;

	return n;
}

int16_t drv8833_speed_get (void)
{
	return speed;
}

int16_t drv8833_speed_set (const int16_t new)
{
	speed = clamp(new);
	TIM1_CCR1 = TIM1_CCR2 = (speed < 0) ? -speed : speed;
	config_outputs();
	return speed;
}

int16_t drv8833_speed_add (const int16_t delta)
{
	return drv8833_speed_set(speed + delta);
}

static void init_timer (void)
{
	// TIM1 runs in the APB2 clock domain, which ticks at 72 MHz. Set the
	// PWM frequency arbitrarily to 72e6 / 64 / 256 steps = 4395 Hz.
	TIM1_PSC = 63;
	TIM1_CNT = 0;

	// Reload after 256 ticks (cycle length).
	TIM1_ARR = 256 - 1;

	// Setup output compare on both channels.
	TIM1_CCR1 = 0;
	TIM1_CCR2 = 0;
	TIM1_CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
	TIM1_CCMR1
		= TIM_CCMR1_CC1S_OUT | TIM_CCMR1_OC1M_FORCE_HIGH
		| TIM_CCMR1_CC2S_OUT | TIM_CCMR1_OC2M_FORCE_HIGH
		;

	// Enable output, refresh the registers, and start the timer.
	TIM1_BDTR = TIM_BDTR_MOE;
	TIM1_EGR  = TIM_EGR_UG;
	TIM1_CR1  = TIM_CR1_CEN;
}

static void init_fault_interrupt (void)
{
	// Setup an external interrupt, using the fact that GPIOn is defined
	// with the same bitmask as EXTIn.
	exti_select_source  (PIN_FAULT, GPIOA);
	exti_set_trigger    (PIN_FAULT, EXTI_TRIGGER_BOTH);
	exti_enable_request (PIN_FAULT);

	// Enable the interrupt line.
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);
}

static void init_gpios (void)
{
	// Setup PA8/PA9 as TIM1_CH1/TIM1_CH2 outputs.
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_OUTPUT_2_MHZ
		, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL
		, GPIO_TIM1_CH1 | GPIO_TIM1_CH2
		);

	// PA11 is the SLEEP output (active low).
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_OUTPUT_2_MHZ
		, GPIO_CNF_OUTPUT_PUSHPULL
		, PIN_SLEEP
		);

	// PA10 is the FAULT input (active low).
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_INPUT
		, GPIO_CNF_INPUT_PULL_UPDOWN
		, PIN_FAULT
		);

	// Add a pullup to the FAULT pin.
	gpio_set(GPIOA, PIN_FAULT);
}

void drv8833_init (void)
{
	init_gpios();
	init_timer();

	// Initial settings.
	drv8833_speed_set(0);
	drv8833_pause();

	// Bring the device out of sleep.
	wakeup();

	// Wait for the DRV8833 to wake up and set FAULT high (inactive).
	for (volatile int i = 10000; i > 0; i--)
		continue;

	// Setup the FAULT interrupt.
	init_fault_interrupt();
}
