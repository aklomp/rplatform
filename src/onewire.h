#pragma once

#include <stdbool.h>
#include <stdint.h>

extern void onewire_init        (void);
extern void onewire_reset_start (void);
extern bool onewire_reset_end   (void);
extern void onewire_xfer_start  (const uint8_t *buf, const uint32_t len);
extern void onewire_xfer_end    (uint8_t *buf, const uint32_t len);
