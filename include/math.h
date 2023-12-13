/* spdx-identifier: GPL-3.0-only */

#ifndef CELLS_MATH_H__
#define CELLS_MATH_H__

#include <stdint.h>

float map_range(float x, float src1, float src2, float dst1, float dst2);
uint32_t isqrt(uint32_t i);

#endif