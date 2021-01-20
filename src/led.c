#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "event.h"
#include "led.h"
#include "util.h"

// Whether the LED is on or off. The state is latched in a separate variable so
// that the LED can be meaningfully set/unset during its blinking phase.
static bool on = false;

// Timer update counter, to count the pulses that make up the blinks.
static uint8_t count = 0;

// PWM map for the blinking LED at boot time. int(x^2.5) for x from 0..19, and
// then in reverse. Dwell longer on the small numbers, because perceived LED
// intensity is not linear.
static const uint16_t pulses[] = {
	   0,    1,    5,   15,   32,
	  55,   88,  129,  181,  243,
	 316,  401,  498,  609,  733,
	 871, 1024, 1191, 1374, 1573,
	1573, 1374, 1191, 1024,  871,
	 733,  609,  498,  401,  316,
	 243,  181,  129,   88,   55,
	  32,   15,    5,    1,    0,
};

// Number of blinks of the LED at boot time. After this amount of blinks, the
// timer is disabled and the LED becomes a normal on/off GPIO. The timer is
// sorely needed for the OneWire module.
#define BLINKS		3

// Number of timer updates needed for all the blinks.
#define UPDATES		(BLINKS * NELEM(pulses))

void led_on (void)
{
	on = true;
	if (count >= UPDATES)
		gpio_set(GPIOB, GPIO1);
}

void led_off (void)
{
	on = false;
	if (count >= UPDATES)
		gpio_clear(GPIOB, GPIO1);
}

void tim3_isr (void)
{
	// Acknowledge all interrupts.
	TIM3_SR = 0;

	// Stop after the set number of updates.
	if (count++ < UPDATES)
		return;

	// Disable TIM3 and unregister the interrupt.
	TIM3_CR1 = 0;
	nvic_disable_irq(NVIC_TIM3_IRQ);

	// Reset PB1 as a normal GPIO pin.
	gpio_set_mode
		( GPIOB
		, GPIO_MODE_OUTPUT_50_MHZ
		, GPIO_CNF_OUTPUT_PUSHPULL
		, GPIO1
		) ;

	// Set the LED to the hidden latched state.
	on ? led_on() : led_off();

	// Raise the BLINK_DONE event, to indicate that TIM3 is available for
	// use by the OneWire module.
	event_raise(EVENT_BLINK_DONE);
}

static void init_gpio (void)
{
	// Setup PB1 as TIM3_CH4.
	gpio_set_mode
		( GPIOB
		, GPIO_MODE_OUTPUT_50_MHZ
		, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL
		, GPIO_TIM3_CH4
		) ;
}

static void init_dma (void)
{
	// DMA1_CH3 serves TIM3_UP.
	DMA1_CCR3   = 0;
	DMA1_IFCR   = DMA_IFCR_CGIF3;
	DMA1_CPAR3  = (uint32_t) &TIM3_DMAR;
	DMA1_CMAR3  = (uint32_t) pulses;
	DMA1_CNDTR3 = NELEM(pulses);
	DMA1_CCR3
		= DMA_CCR_MINC		// Increment memory pointer
		| DMA_CCR_DIR		// Memory-to-peripheral
		| DMA_CCR_MSIZE_16BIT	// 16-bit source
		| DMA_CCR_PSIZE_16BIT	// 16-bit destination
		| DMA_CCR_CIRC		// Circular mode (repeat pattern)
		| DMA_CCR_EN		// Enable and wait for requests
		;
}

static void init_timer (void)
{
	// Set the period to 1/100 second (10 ms).
	TIM3_PSC = 100 - 1;
	TIM3_ARR = 7200;
	TIM3_CNT = 0;
	TIM3_SR  = 0;

	// Setup PWM mode. The value of TIM3_CCR4 is supplied by DMA.
	TIM3_CCER  = TIM_CCER_CC4E;
	TIM3_CCMR2 = TIM_CCMR2_OC4M_PWM1;

	// Issue an interrupt and a DMA request (targeting TIM3_CCR4) at every
	// update event.
	TIM3_DIER = TIM_DIER_UDE | TIM_DIER_UIE;
	TIM3_DCR  = 0x10;
	nvic_enable_irq(NVIC_TIM3_IRQ);

	// Generate an update event to refresh all registers and start the
	// timer.
	TIM3_EGR = TIM_EGR_UG;
	TIM3_CR1 = TIM_CR1_CEN;
}

void led_init (void)
{
	init_gpio();
	init_dma();
	init_timer();
}
