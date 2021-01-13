#pragma once

#include <stdint.h>

// The bitmap array for the LCD has the following structure.
//
// 0: T1 T2 T3 T4 1F 1G 1E 1D         A
// 1: 1A 1B 1C -- 2F 2G 2E 2D       F   B
// 2: 2A 2B 2C C1 3F 3G 3E 3D         G
// 3: 3A 3B 3C -- 4F 4G 4E 4D       E   C
// 4: 4A 4B 4C D1 5F 5G 5E 5D         D
// 5: 5A 5B 5C -- 6F 6G 6E 6D
// 6: 6A 6B 6C -- -- -- -- --
//
extern void ht1621_draw (const uint8_t *buf);
extern void ht1621_init (void);
