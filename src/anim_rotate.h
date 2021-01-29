#pragma once

#include <stdbool.h>
#include <stdint.h>

extern uint8_t anim_rotate;

extern void anim_rotate_step (void);
extern void anim_rotate_set_coarse (const bool coarse);
