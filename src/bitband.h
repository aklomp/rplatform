#pragma once

#include <stdint.h>

#define BITBAND_SRAM_REF	UINT32_C(0x20000000)
#define BITBAND_SRAM_BASE	UINT32_C(0x22000000)

// Return the bit-banded address of an SRAM variable.
static inline volatile uint32_t *bitband_sram (const volatile void *ptr, const uint32_t bit)
{
	const uint32_t offs = ((const uint32_t) ptr - BITBAND_SRAM_REF) * 32 + bit * 4;
	const uint32_t addr = BITBAND_SRAM_BASE + offs;

	return (volatile uint32_t *) addr;
}
