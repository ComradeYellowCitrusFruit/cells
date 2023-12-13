/* spdx-identifier: GPL-3.0-only */

#ifndef CELLS_WORLD_H__
#define CELLS_WORLD_H__

#include <stdbool.h>
#include "include/cells.h"

struct tile {
	uint8_t type; /* 0 for nothing, 1 for dead stuff, 2 for cell */
	union {
		struct cell cell;
		struct dead_thing dead;
	};
};

struct world {
	unsigned int iters;

	unsigned int length;
	unsigned int height;
	struct tile *grid;
};

void init_world(struct world *w);
void step_world(struct world *w);
void free_world(struct world *w);

bool world_has_life(struct world *w);

#endif