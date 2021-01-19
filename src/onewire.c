#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "event.h"
#include "onewire.h"

static uint8_t txbuf[88];
static volatile uint8_t rxbuf[88];

static enum {
	STATE_RESET,
	STATE_XFER,
} state;

void tim4_isr (void)
{
	// Acknowledge all interrupts.
	TIM4_SR = 0;

	// The timer completed, raise the appropriate event.
	switch (state) {
	case STATE_RESET:
		event_raise(EVENT_ONEWIRE_RESET_DONE);
		break;

	case STATE_XFER:
		event_raise(EVENT_ONEWIRE_XFER_DONE);
		break;

	default:
		break;
	}
}

static inline void import_request (const uint8_t *src, const uint32_t len)
{
	// Populate the pulse width array. For every 0, the bus is pulled low
	// for 60 us. For every 1, the bus is pulled low for 5 us.
	for (uint32_t byte = 0; byte < len; byte++)
		for (uint32_t bit = 0; bit < 8; bit++)
			txbuf[byte * 8 + bit] = (src[byte] >> bit) & 1 ? 5 : 60;
}

static inline void export_response (uint8_t *dst, const uint32_t len)
{
	for (uint32_t byte = 0; byte < len; byte++) {
		dst[byte] = 0;
		for (uint32_t bit = 0; bit < 8; bit++) {
			dst[byte] |= rxbuf[byte * 8 + bit] & GPIO4 ? 1 << bit : 0;
		}
	}
}

static void init_dma_tx (const uint32_t nbits)
{
	// DMA1_CH3 serves TIM3_UP.
	DMA1_CCR3   = 0;
	DMA1_IFCR   = DMA_IFCR_CGIF3;
	DMA1_CPAR3  = (uint32_t) &TIM3_DMAR;
	DMA1_CMAR3  = (uint32_t) txbuf;
	DMA1_CNDTR3 = nbits;
	DMA1_CCR3
		= DMA_CCR_MINC		// Increment memory pointer
		| DMA_CCR_DIR		// Memory-to-peripheral
		| DMA_CCR_MSIZE_8BIT	// 8-bit source
		| DMA_CCR_PSIZE_16BIT	// 16-bit destination (zero-extend)
		| DMA_CCR_EN		// Enable and wait for requests
		;
}

static void init_dma_rx (const uint32_t nbits)
{
	// Setup the DMA request. DMA1_CH2 serves TIM3_CH3.
	DMA1_CCR2   = 0;
	DMA1_IFCR   = DMA_IFCR_CGIF2;
	DMA1_CPAR2  = (uint32_t) &GPIOB_IDR;
	DMA1_CMAR2  = (uint32_t) rxbuf;
	DMA1_CNDTR2 = nbits;
	DMA1_CCR2
		= DMA_CCR_MINC		// Increment memory pointer
		| DMA_CCR_MSIZE_8BIT	// 8-bit destination
		| DMA_CCR_PSIZE_16BIT	// 16-bit source (use low byte)
		| DMA_CCR_EN		// Enable and wait for requests
		;
}

static void init_master_timer (const uint32_t usec)
{
	TIM4_CR1  = 0;
	TIM4_CNT  = 0;
	TIM4_DIER = 0;

	// Set the timer prescaler to 1 us.
	TIM4_PSC = 72 - 1;
	TIM4_ARR = usec;

	// Use the Enable status of this timer to gate TIM3.
	TIM4_CR2 = TIM_CR2_MMS_ENABLE;

	// Generate an update event to refresh all registers.
	TIM4_EGR = TIM_EGR_UG;
	TIM4_SR  = 0;

	// Enable the update interrupt to notify when the timer is finished.
	TIM4_DIER = TIM_DIER_UIE;
	nvic_enable_irq(NVIC_TIM4_IRQ);

	// Start the timer in One-Pulse Mode.
	TIM4_CR1 = TIM_CR1_OPM | TIM_CR1_CEN;
}

static void init_xfer_timer (void)
{
	TIM3_CR1 = 0;
	TIM3_CNT = 0;
	TIM3_SR  = 0;

	// Set the timer prescaler to 1 us.
	TIM3_PSC = 72 - 1;

	// Set the bit window size to 80 us.
	TIM3_ARR = 80;

	// CCR1 pulls the line low for a given period, and then high. The
	// length of the low period depends on the value of the bit to
	// transfer, so the value is written by DMA at every update event.
	TIM3_CCMR1 = TIM_CCMR1_OC1M_PWM2;
	TIM3_DCR   = 0x0D;

	// CCR3 issues a DMA request after 10 us that samples the bus to read
	// the value returned by the device.
	TIM3_CCR3  = 10;
	TIM3_CCMR2 = TIM_CCMR2_OC3M_PWM2;

	// Enable CCR1 and CCR3 and their DMA requests.
	TIM3_CCER = TIM_CCER_CC1E | TIM_CCER_CC3E;
	TIM3_DIER = TIM_DIER_UDE | TIM_DIER_CC3DE;

	// Configure TIM3 to slave off TIM4 in Gated mode.
	TIM3_SMCR = TIM_SMCR_TS_ITR3 | TIM_SMCR_SMS_GM;

	// Generate an update event to refresh the registers and start the
	// timer. It will run when gated by TIM4.
	TIM3_EGR = TIM_EGR_UG;
	TIM3_CR1 = TIM_CR1_CEN;
}

void onewire_xfer_start (const uint8_t *buf, const uint32_t len)
{
	state = STATE_XFER;

	// Import the buffer into the transmit buffer.
	import_request(buf, len);

	// Setup the DMA for the request pulses and the response reads.
	init_dma_tx(len * 8);
	init_dma_rx(len * 8);

	// Setup the slave timer, which is gated by the master timer.
	init_xfer_timer();

	// Set the master gate window size to the number of bits * 80 us.
	init_master_timer(80 * 8 * len);
}

void onewire_xfer_end (uint8_t *buf, const uint32_t len)
{
	// Populate the output buffer with the response.
	export_response(buf, len);
}

static void init_reset_timer (void)
{
	TIM3_CR1 = 0;
	TIM3_CNT = 0;
	TIM3_SR  = 0;

	// Set the timer prescaler to 1 us.
	TIM3_PSC = 72 - 1;

	// Set the timer period to 800 us: 500 us for the reset pulse, then 200
	// us for the device response.
	TIM3_ARR = 800;

	// Configure CCR1 to pull the line high after 500 us.
	TIM3_CCR1  = 500;
	TIM3_CCMR1 = TIM_CCMR1_OC1M_PWM2;

	// Configure CCR3 to issue a DMA request after 600 us that will sample
	// the bus to check if the device pulled it low to signal its presence.
	TIM3_CCR3  = 600;
	TIM3_CCMR2 = TIM_CCMR2_OC3M_PWM2;
	TIM3_DIER  = TIM_DIER_CC3DE;
	TIM3_CCER  = TIM_CCER_CC1E | TIM_CCER_CC3E;

	// Configure TIM3 to slave off TIM4 in Gated mode.
	TIM3_SMCR = TIM_SMCR_TS_ITR3 | TIM_SMCR_SMS_GM;

	// Refresh the registers and start the timer in One-Pulse Mode.
	TIM3_EGR = TIM_EGR_UG;
	TIM3_CR1 = TIM_CR1_OPM | TIM_CR1_CEN;
}

void onewire_reset_start (void)
{
	state = STATE_RESET;

	// Setup Rx DMA to read one bit at the sample moment.
	init_dma_rx(1);

	// Setup both timers. The master timer runs for the same length as the
	// slave timer.
	init_reset_timer();
	init_master_timer(800);
}

bool onewire_reset_end (void)
{
	// Return whether the device pulled the line low.
	return !(rxbuf[0] & GPIO4);
}

static void init_gpio (void)
{
	// Setup PB4 as TIM3_CH1 (remapped).
	gpio_set_mode
		( GPIOB
		, GPIO_MODE_OUTPUT_2_MHZ
		, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN
		, GPIO_TIM3_PR_CH1
		) ;

	// Remap the pin to use the alternate function. We also need to disable
	// JNTRST to free PB4. Next time let's use a different pin, preferably
	// a UART which can be made to drive OneWire directly.
	gpio_primary_remap
		( AFIO_MAPR_SWJ_CFG_FULL_SWJ_NO_JNTRST
		, AFIO_MAPR_TIM3_REMAP_PARTIAL_REMAP
		) ;
}

void onewire_init (void)
{
	init_gpio();
}
