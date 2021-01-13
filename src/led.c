#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "led.h"
#include "util.h"

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

void led_on (void)
{
	TIM3_CCMR2 = TIM_CCMR2_CC4S_OUT | TIM_CCMR2_OC4M_FORCE_HIGH;
}

void led_off (void)
{
	TIM3_CCMR2 = TIM_CCMR2_CC4S_OUT | TIM_CCMR2_OC4M_FORCE_LOW;
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

static void init_slave_timer (void)
{
	// Set the period to 1/100 second (10 ms). With 40 samples, one blink
	// lasts 400 ms.
	TIM3_PSC = 100 - 1;
	TIM3_ARR = 7200;
	TIM3_CNT = 0;

	// Setup PWM mode. The value of TIM3_CCR4 is supplied by DMA.
	TIM3_CCER  = TIM_CCER_CC4E;
	TIM3_CCMR2 = TIM_CCMR2_OC4M_PWM1;

	// Issue a DMA request at every update event targeting TIM3_CCR4.
	TIM3_DIER = TIM_DIER_UDE;
	TIM3_DCR  = 0x10;

	// Slave off TIM4 in Gated mode.
	TIM3_SMCR = TIM_SMCR_TS_ITR3 | TIM_SMCR_SMS_GM;

	// Generate an update event to refresh all registers and start the
	// timer. It will run when gated by TIM4.
	TIM3_EGR = TIM_EGR_UG;
	TIM3_CR1 = TIM_CR1_CEN;
}

static void init_master_timer (void)
{
	// TIM4 is the master timer. It gates TIM3, which is set up to blink
	// the LED. 2400 * 36000 = 1200 ms @ 72 MHz.
	TIM4_PSC = 2400 - 1;
	TIM4_ARR = 36000;
	TIM4_CNT = 0;

	// Use the Enable status of this timer to gate TIM3.
	TIM4_CR2 = TIM_CR2_MMS_ENABLE;

	// Generate an update event to refresh all registers. This is necessary
	// for One-Pulse Mode to work. Then start the timer in One-Pulse Mode.
	TIM4_EGR = TIM_EGR_UG;
	TIM4_CR1 = TIM_CR1_OPM | TIM_CR1_CEN;
}

void led_init (void)
{
	init_gpio();
	init_dma();
	init_slave_timer();
	init_master_timer();
}
