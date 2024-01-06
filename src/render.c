/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <stdio.h>
#include <stdlib.h>
#include "include/render.h"
#include "include/world.h"
#include "include/math.h"
#include "include/lib/stb_image_write.h"

#define COLOR_HASH(x) ({ \
	uint32_t __v = __builtin_popcount(x); \
	__v = (__v) ? __v * __v * __v * 2 : 1; \
	(isqrt(__v) & 0xff); \
})

#define INDEX_RGBA(c, i) (0xff & (x >> (32 - (8 * (i + 1)))))

uint32_t default_colorgen(struct tile *t) 
{
	if(t->type == 0)
		return RGBA(0xff, 0xff, 0xff, 0);
	else if(t->type == 1) {
		uint32_t tmp = 0xff - ((t->dead.energy & 0xff) >> 2);
		return RGBA(0xff - tmp, 0 - tmp, 0 - tmp, 0xff);
	} else {
		uint32_t r = COLOR_HASH(t->cell.genes[0]),
			 g = COLOR_HASH(t->cell.genes[1]),
			 b = COLOR_HASH(t->cell.genes[2]),
			 a = 255,
			 tmp = 0xff - ((t->cell.energy & 0xff) >> 2);
		return RGBA(r - tmp, g - tmp, b - tmp, a - tmp);
	}
}

struct render_settings render_defaults()
{
	struct render_settings ret;
	ret.color_gen = default_colorgen;
	ret.encoding = 2;
	ret.write_to_file = false;

	return ret;
}

void *render(struct world *w, const char *filename,
	struct render_settings settings) 
{
	int color_size = (settings.encoding - 1) ? 3 : 4;
	uint8_t *fb = malloc(w->height * w->length * color_size);
	unsigned int x = w->length, y =  w->height;
	
	for(int i = 0; i < x; i++) {
		for(int j = y-1; j != 0; j--) {
			uint32_t c = settings.color_gen(
				&INDEX_WORLD((*w), i, j)
			);

			fb[((x * j) + i) * 3 + 0] = INDEX_RGBA(c, 0);
			fb[((x * j) + i) * 3 + 1] = INDEX_RGBA(c, 1);
			fb[((x * j) + i) * 3 + 2] = INDEX_RGBA(c, 2);

			/* this will compile to a cmov right? */
			if(color_size == 4)
				fb[((x * j) + i) * 3 + 3] = INDEX_RGBA(c, 3);
		}
	}

	if((settings.write_to_file && filename != NULL) || filename != NULL)
		stbi_write_png(filename, x, y, color_size, fb, 0);
	
	return fb;
}

void free_render_buffer(void *renderbuf) {
	free(renderbuf);
}