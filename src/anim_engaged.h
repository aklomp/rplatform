#pragma once

#include <stdint.h>

// Global display flags mask for the engaged animation.
extern uint8_t anim_engaged;

extern void anim_engaged_step (void);
extern void anim_engaged_off  (void);
extern void anim_engaged_on   (void);
