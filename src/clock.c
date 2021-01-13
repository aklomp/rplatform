#include <libopencm3/stm32/rcc.h>

#include "util.h"

static const uint32_t map_clock[] = {
	RCC_DMA1,
	RCC_GPIOA,
	RCC_GPIOB,
	RCC_TIM3,
	RCC_TIM4,
};

static const uint32_t map_reset[] = {
	RST_TIM3,
	RST_TIM4,
};

void clock_init (void)
{
	// Run at 72 MHz.
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	// Enable peripheral clocks.
	FOREACH (map_clock, clock)
		rcc_periph_clock_enable(*clock);

	// Reset peripherals.
	FOREACH (map_reset, reset)
		rcc_periph_reset_pulse(*reset);
}
