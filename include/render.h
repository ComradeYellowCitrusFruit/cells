/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_RENDERER_H__
#define CELLS_RENDERER_H__

#include "include/genetics.h"
#include "include/world.h"

#define RGBA(r, g, b, a) ((((r) & 0xff) << 24) | (((g) & 0xff) << 16) | (((b) & 0xff) << 8) | (a & 0xff))

typedef uint32_t (*color_fn)(struct tile*);

struct render_settings {
	/* 1 for rgb, 2 for rgba */
	int encoding;
	bool write_to_file;

	color_fn color_gen;
};

struct render_settings render_defaults();
void *render(struct world *w, const char *filename, 
        struct render_settings settings);

void free_render_buffer(void *renderbuf);

uint32_t default_colorgen(struct tile *t);

#endif