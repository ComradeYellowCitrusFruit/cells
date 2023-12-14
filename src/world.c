/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/


#include <stdint.h>
#include <stdlib.h>
#include "include/world.h"
#include "include/math.h"
#include "include/rng.h"

extern generator_handle main_rng;

void gen_random_cell(struct cell *c, generator_handle g) 
{
	if(g == NULL)
		g = main_rng;

	c->oscilator_ctr = 0;
	/* every 100 updates flip, 
	*  aka the effect of 1.0 -> GENE_SET_OSCILATOR 
	*/ 
	c->oscilator_period = 100;

	c->age = 0;
	c->id = gen64(g);
	c->energy = 20; /* enough to have one child */

	gen_bytes(g, c->genes, sizeof(gene_t[4]));
}

void init_world(struct world *w,
	unsigned int x, unsigned int y, unsigned int i) 
{
	int num_cells = isqrt((x*y))/8;
	i = (i > 4) ? i : 4 ;
	w->height = y;
	w->length = x;
	w->iters  = 0;
	w->food_gen_iters = i;
	w->grid = calloc(x * y, sizeof(struct tile));

	for(int i = 0; i < num_cells; i++) {
		unsigned int ax, ay;
		ax = gen32(main_rng) % x;
		ay = gen32(main_rng) % y;

		switch(INDEX_WORLD((*w), ax, ay).type) {
			case 1:
			case 2:
			i--;
			break;

			default:
			INDEX_WORLD((*w), ax, ay).type = 2;
			gen_random_cell(
				&INDEX_WORLD((*w), ax, ay).cell,
				main_rng
			);
			break;
		}
	}

	for(int i = 0; i < num_cells * (i/4); i++) {
		unsigned int ax, ay;
		ax = gen32(main_rng) % x;
		ay = gen32(main_rng) % y;

		switch(INDEX_WORLD((*w), ax, ay).type) {
			case 1:
			case 2:
			i--;
			break;

			default:
			INDEX_WORLD((*w), ax, ay).type = 1;
			INDEX_WORLD((*w), ax, ay)
				.dead.id = gen64(main_rng);
			INDEX_WORLD((*w), ax, ay)
				.dead.energy = isqrt(gen32(main_rng));
			break;
		}
	}
}