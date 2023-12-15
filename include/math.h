/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_MATH_H__
#define CELLS_MATH_H__

#include <stdint.h>

float map_range(float x, float src1, float src2, float dst1, float dst2);
__attribute__ ((__pure__)) uint32_t isqrt(uint32_t i);

#endif