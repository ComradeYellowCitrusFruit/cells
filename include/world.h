/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_WORLD_H__
#define CELLS_WORLD_H__

#include <stdbool.h>
#include "include/cells.h"

#define INDEX_WORLD(w, x, y) (w.grid[(w.length * (y % w.height)) + (x % w.length)])

struct tile {
	uint8_t type; /* 0 for nothing, 1 for dead stuff, 2 for cell */
	union {
		struct cell cell;
		struct dead_thing dead;
	};
};

struct world {
	unsigned int iters;
	unsigned int food_gen_iters;

	unsigned int length;
	unsigned int height;
	struct tile *grid;
};

struct statistics {
	unsigned int pop;
	unsigned int death;
	unsigned int food;
	unsigned int old_age;
	unsigned int murder;
	unsigned int starve;
	unsigned int suicide;
	unsigned int births;
};

void init_world(struct world *w, unsigned int x, unsigned int y, 
	unsigned int i, unsigned int c);
void step_world(struct world *w, struct statistics *stats);
void free_world(struct world *w);

bool world_has_life(struct world *w);

void step_cell(struct tile *t, struct world *w, struct statistics *stats,
	uint32_t x, uint32_t y);

#endif