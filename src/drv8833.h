#pragma once

#include <stdint.h>

extern int16_t drv8833_speed_get (void);
extern int16_t drv8833_speed_set (const int16_t speed);
extern int16_t drv8833_speed_add (const int16_t delta);
extern void drv8833_run (void);
extern void drv8833_pause (void);
extern void drv8833_init (void);
