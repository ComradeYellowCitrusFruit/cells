/* spdx-identifier: GPL-3.0-only */

#include "include/math.h"
#include "include/genetics.h"

float strength_to_float(uint16_t strength) {
	uint32_t tmp = 0;
	tmp |= (strength >> 15) << 31; /* extract sign bit */
	tmp |= (((strength >> 10) & 0x1f) - 15) + 127; /* extract exponent */
	tmp |= (strength) & 0x3ff; /* extract and apply mantissa */
	return *(float*)&tmp; /* final conversion */
}

uint32_t isqrt(uint32_t i) {
	if(i <= 1)
		return i;

	uint32_t a0 = i/2;
	uint32_t a1 = (a0 + i/a0)/2;
	while(a1 < a0) {
		a0 = a1;
		a1 = (a0 + i/a0)/2;
	}
	return a0;
}

float map_range(float x, float src1, float src2, float dst1, float dst2) {
	return src1 + (((x - src1) * (dst2 - dst1))/(src2 - src1));
}