#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "ht1621.h"

#define SYSDIS	0x00	// 1000 0000 0000
#define SYSEN	0x02	// 1000 0000 0010
#define LCDOFF	0x04	// 1000 0000 0100
#define LCDON	0x06	// 1000 0000 0110
#define WDTDIS1	0x0A	// 1000 0000 1010
#define RC256	0x30	// 1000 0011 0000
#define BIAS	0x52	// 1000 0101 0010  1/3 duty, 4x com

// Zero buffer to clear the screen at boot time.
static const uint8_t zerobuf[7];

static void select (void)
{
	gpio_clear(GPIOA, GPIO4);
}

static void release (void)
{
	gpio_set(GPIOA, GPIO4);
}

static void init_spi (void)
{
	// Setup PA4 as CS.
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_OUTPUT_2_MHZ
		, GPIO_CNF_OUTPUT_PUSHPULL
		, GPIO4
		) ;

	// Setup PA5 as SCK (WR) and PA7 as MOSI (DATA).
	gpio_set_mode
		( GPIOA
		, GPIO_MODE_OUTPUT_2_MHZ
		, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL
		, GPIO_SPI1_SCK | GPIO_SPI1_MOSI
		) ;

	// Reset the SPI peripheral.
	release();
	SPI1_CR1 = 0;
	spi_reset(SPI1);

	// Initialize SPI as master.
	SPI1_CR2 = SPI_CR2_SSOE;			// Enable output
	SPI1_CR1
		= SPI_CR1_BAUDRATE_FPCLK_DIV_256	// 281.250 KHz
		| SPI_CR1_MSTR				// Master mode
		| SPI_CR1_SPE				// Enable SPI1
		;
}

void ht1621_draw (const uint8_t *buf)
{
	select();
	spi_send(SPI1, 0xA0);
	spi_send(SPI1, buf[0] >> 1);
	spi_send(SPI1, buf[0] << 7 | buf[1] >> 1);
	spi_send(SPI1, buf[1] << 7 | buf[2] >> 1);
	spi_send(SPI1, buf[2] << 7 | buf[3] >> 1);
	spi_send(SPI1, buf[3] << 7 | buf[4] >> 1);
	spi_send(SPI1, buf[4] << 7 | buf[5] >> 1);
	spi_send(SPI1, buf[5] << 7 | buf[6] >> 1);
	while (SPI1_SR & SPI_SR_BSY);
	release();
}

static void write_cmd (const uint8_t cmd)
{
	// The first byte is the 0x80 prefix plus the first four command bytes.
	// The second byte is the remainder of the command bytes.
	const uint8_t b0 = 0x80 | (cmd >> 4);	// 01 00 00 00 C7 C6 C5 C4
	const uint8_t b1 = cmd << 4;		// C3 C2 C1 C0 xx xx xx xx

	select();
	spi_send(SPI1, b0);
	spi_send(SPI1, b1);
	while (SPI1_SR & SPI_SR_BSY);
	release();
}

static void init_dev (void)
{
	write_cmd(BIAS);
	write_cmd(RC256);
	write_cmd(SYSDIS);
	write_cmd(WDTDIS1);
	write_cmd(SYSEN);
	write_cmd(LCDON);
	ht1621_draw(zerobuf);
}

void ht1621_init (void)
{
	init_spi();
	init_dev();
}
