/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_UTIL_H__
#define CELLS_UTIL_H__

#define ZERO_STRUCT(s) memset(&s, 0, sizeof(s))

#define unlikely(c) __builtin_expect(!!(c), 0)
#define likely(c) __builtin_expect(!!(c), 1)

#define die(b, m) if(unlikely(b)) { \
	fprintf(stderr, "Error in %s, line %d of %s: %s", \
	__PRETTY_FUNCTION__, __LINE__, __FILE__, m); exit(-1); }

/* primarily used in cell and rng code cause theres like
*  1/(2^32) type chances */

#endif