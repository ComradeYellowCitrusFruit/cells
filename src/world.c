/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/world.h"
#include "include/math.h"
#include "include/rng.h"

extern generator_handle main_rng;

static void place_food(struct world *w, uint32_t num) 
{
	for(int i = 0; i < num * (w->food_gen_iters/4); i++) {
		unsigned int ax, ay;
		struct tile t;
		ax = gen32(main_rng) % w->length;
		ay = gen32(main_rng) % w->height;
		t = INDEX_WORLD((*w), ax, ay);

		switch(t.type) {
			case 1:
			case 2:
			i--;
			break;

			default:
			t.type = 1;
			t.dead.id = gen64(main_rng);
			t.dead.energy = isqrt(gen32(main_rng));
			break;
		}
	}
}

void init_world(struct world *w, unsigned int x, unsigned int y,
	unsigned int it, unsigned int c)
{
	c  = c ? c : isqrt((x*y))/8;
	it = (it > 4) ? it : 4 ;
	w->height = y;
	w->length = x;
	w->iters  = 0;
	w->food_gen_iters = it;
	w->grid = calloc(x * y, sizeof(struct tile));

	for(int i = 0; i < c; i++) {
		unsigned int ax, ay;
		struct tile t;
		ax = gen32(main_rng) % x;
		ay = gen32(main_rng) % y;
		t = INDEX_WORLD((*w), ax, ay);

		switch(t.type) {
			case 1:
			case 2:
			i--;
			break;

			default:
			t.type = 2;
			gen_random_cell(&t.cell, main_rng);
			break;
		}
	}

	place_food(w, c/4);
}

void free_world(struct world *w) 
{
	free(w->grid);
	w->grid = NULL;
}

bool world_has_life(struct world *w) 
{
	/* just use flat indexing, we don't need that fancy shit */
	for(int i = 0; i < w->height * w->length; i++)
		if(w->grid[i].type == 2)
			return true;

	return false;
}

void step_world(struct world *w, struct statistics *stats)
{
	/* first things first, put down new food */
	if((w->iters += 1) % w->food_gen_iters == 0)
		place_food(w, isqrt((w->length * w->height))/32);

	/* next, iterate through each cell */
	for(uint32_t y = 0; y < w->length; y++) {
		for(uint32_t x = 0; x < w->length; x++) {
			step_cell(&INDEX_WORLD((*w), x, y), w, stats, x, y);
		}
	}
}