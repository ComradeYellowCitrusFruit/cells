/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "include/cells.h"
#include "include/rng.h"
#include "include/world.h"

extern generator_handle main_rng;

void gen_random_cell(struct cell *c, generator_handle g) 
{
	if(g == NULL)
		g = main_rng;

	c->oscil_ctr = 1; /* no divide by zeros on my watch */
	/* every 100 updates flip, 
	*  aka the effect of 1.0 -> GENE_SET_OSCILATOR 
	*/ 
	c->oscil_dur = 100;

	c->updated = false;
	c->age = 0;
	c->compass = gen8(g) % 4;
	c->id = gen64(g);
	c->energy = 20; /* enough to have one child */

	gen_bytes(g, c->genes, sizeof(gene_t[4]));
}

uint32_t calc_death_energy(struct cell *c) 
{
	if(c->energy < 4)
		return 2;
	else if(c->energy < 32)
		return c->energy/2;
	else
		return c->energy/4;
}

#define ONE_IF_ALIVE(w, x, y) ((INDEX_WORLD((*w), x, y).type == 2) ? 1 : 0)

static inline float density(struct world *w, uint32_t x, uint32_t y)
{
	/* definitely super inefficient, or super efficient, dunno which */
	return (ONE_IF_ALIVE(w, x+1, y) + ONE_IF_ALIVE(w, x+1, y+1) + 
		ONE_IF_ALIVE(w, x, y+1) + ONE_IF_ALIVE(w, x-1, y+1) +
		ONE_IF_ALIVE(w, x-1, y) + ONE_IF_ALIVE(w, x-1, y-1) +
		ONE_IF_ALIVE(w, x, y-1) + ONE_IF_ALIVE(w, x+1, y-1) -
		4.0f) / 8.0f;
}

#undef ONE_IF_ALIVE

static inline uint32_t get_energy(struct tile *t) 
{
	if(t->type == 1) {
		return t->dead.energy;
	} else {
		return 0;
	}
} 

__attribute__((__pure__))
static inline struct tile *index_forward(struct world *w,
	uint32_t x, uint32_t y, int compass)
{
	switch(compass) {
		case NORTH:
		return &INDEX_WORLD((*w), x, y+1);

		case SOUTH:
		return &INDEX_WORLD((*w), x, y-1);

		case EAST:
		return &INDEX_WORLD((*w), x+1, y);

		case WEST:
		return &INDEX_WORLD((*w), x - 1, y);

		default:
		return index_forward(w, x, y, compass % 5);
	}
}

__attribute__((__pure__))
static inline struct tile *index_backward(struct world *w,
	uint32_t x, uint32_t y, int compass)
{
	switch(compass) {
		case NORTH:
		return &INDEX_WORLD((*w), x, y-1);

		case SOUTH:
		return &INDEX_WORLD((*w), x, y+1);

		case EAST:
		return &INDEX_WORLD((*w), x-1, y);

		case WEST:
		return &INDEX_WORLD((*w), x+1, y);

		default:
		return index_forward(w, x, y, compass % 5);
	}
}

static inline float input(struct cell *c, struct world *w, 
	uint32_t x, uint32_t y, gene_t g)
{
	switch((g & GENE_INPUT_BITS) >> 24) {
		case GENE_AGE:
		return map_range((float)c->age, 0, 2048, -1.0f, 1.0f);

		case GENE_ENERGY:
		return map_range((float)c->energy, 0, 200, -1.0f, 1.0f);

		case GENE_OSCILATOR:
		/* no division by zero! */
		if(c->oscil_ctr/c->oscil_dur & 1 == 0)
			return 1.0;
		else
			return -1.0;

		case GENE_FOOD_X:
		if(INDEX_WORLD((*w), x + 1, y).type == 1) 
			return 1.0;
		else if(INDEX_WORLD((*w), x - 1, y).type == 1) 
			return -1.0;

		case GENE_FOOD_Y:
		if(INDEX_WORLD((*w), x, y  + 1).type == 1) 
			return 1.0;
		else if(INDEX_WORLD((*w), x, y - 1).type == 1) 
			return -1.0;
		break;

		case GENE_FOOD_FORWARD:
		return  index_forward(w, x, y, c->compass) == 1 ? 1.0 : 
			(index_backward(w, x, y, c->compass) == 1 ? -1.0 : 0);

		case GENE_OBSTACLE_X:
		if(INDEX_WORLD((*w), x + 1, y).type == 2) 
			return 1.0;
		else if(INDEX_WORLD((*w), x - 1, y).type == 2) 
			return -1.0;

		case GENE_OBSTACLE_Y:
		if(INDEX_WORLD((*w), x, y  + 1).type == 2) 
			return 1.0;
		else if(INDEX_WORLD((*w), x, y - 1).type == 2) 
			return -1.0;

		case GENE_OBSTACLE_FORWARD:
		return  index_forward(w, x, y, c->compass) == 2 ? 1.0 : 
			(index_backward(w, x, y, c->compass) == 2 ? -1.0 : 0);

		case GENE_DENSITY:
		return density(w, x, y);

		case GENE_LAST_X:
		switch(c->compass) {
			case EAST:
			return 1.0;

			case WEST:
			return -1.0;
			break;

			default:
			return 0;
			break;
		}

		case GENE_LAST_Y:
		switch(c->compass) {
			case NORTH:
			return 1.0;

			case SOUTH:
			return -1.0;
			break;

			default:
			return 0;
			break;
		}

		default:
		return 0;
	}
}

static inline int output(struct cell **c, struct world *w,
	uint32_t *x, uint32_t *y, gene_t g, float in, int *energy)
{
	unsigned int tmp;
	struct tile *victim;
	switch(g & GENE_OUTPUT_BITS) {
		case GENE_MOVE_X:
gene_move_x:
		if(in > 1.0) {
			switch(INDEX_WORLD((*w), *x + 1, *y).type) {
				case 1:
				(*c)->energy += INDEX_WORLD((*w), *x + 1, *y)
					.dead.energy;
				case 0:
				break;

				default:
				return;
			}

			memcpy(
				&INDEX_WORLD((*w), *x + 1, *y),
				&INDEX_WORLD((*w), *x, *y), 
				sizeof(struct tile)
			);
			INDEX_WORLD((*w), *x, *y).type = 0;
			*c = &INDEX_WORLD((*w), ++(*x), *y).cell;
			(*c)->compass = EAST;
		} else if(in < -1.0) {
			switch(INDEX_WORLD((*w), *x - 1, *y).type) {
				case 1:
				(*c)->energy += INDEX_WORLD((*w), *x - 1, *y)
					.dead.energy;
				case 0:
				break;

				default:
				return;
			}

			memcpy(
				&INDEX_WORLD((*w), *x - 1, *y),
				&INDEX_WORLD((*w), *x, *y), 
				sizeof(struct tile)
			);
			INDEX_WORLD((*w), *x, *y).type = 0;
			*c = &INDEX_WORLD((*w), --(*x), *y).cell;
			(*c)->compass = WEST;
		}
		return;

		case GENE_MOVE_Y:
gene_move_y:
		if(in > 1.0) {
			switch(INDEX_WORLD((*w), *x, *y + 1).type) {
				case 1:
				(*c)->energy += INDEX_WORLD((*w), *x, *y + 1)
					.dead.energy;
				case 0:
				break;

				default:
				return;
			}

			memcpy(
				&INDEX_WORLD((*w), *x, *y + 1),
				&INDEX_WORLD((*w), *x, *y), 
				sizeof(struct tile)
			);
			INDEX_WORLD((*w), *x, *y).type = 0;
			*c = &INDEX_WORLD((*w), *x, ++(*y)).cell;
			(*c)->compass = NORTH;
		} else if(in < -1.0) {
			switch(INDEX_WORLD((*w), *x, *y  - 1).type) {
				case 1:
				(*c)->energy += INDEX_WORLD((*w), *x, *y - 1)
					.dead.energy;
				case 0:
				break;

				default:
				return;
			}

			memcpy(
				&INDEX_WORLD((*w), *x - 1, *y),
				&INDEX_WORLD((*w), *x, *y), 
				sizeof(struct tile)
			);
			INDEX_WORLD((*w), *x, *y).type = 0;
			*c = &INDEX_WORLD((*w), *x, --(*y)).cell;
			(*c)->compass = SOUTH;
		}
		return 0;

		case GENE_COMMIT_SUICIDE:
		if(fabsf(in) > 3.5f) {
die:
			/* suicide/death is indicated with a -1 returned,
			*  a null *c, and a 1 in INDEX((*w), *x, *y).type 
			*/
			INDEX_WORLD((*w), *x, *y).type = 1;
			INDEX_WORLD((*w), *x, *y)
				.dead.energy = calc_death_energy((*c));
			*c = NULL;
			return -1;
		}
		return 0;

		case GENE_MOVE_FORWARD:
		/* I am not gonna lie, i'm only using gotos cause im lazy */
		switch((*c)->compass) {
			case SOUTH:
			in *= -1;
			case NORTH:
			goto gene_move_y;

			case WEST:
			in *= -1;
			case EAST:
			goto gene_move_x;

			default:
			return 0;
		}

		case GENE_SET_OSCILATOR:
		tmp = abs((int)(in * 100));

		/* some weird sanitization */
		if(in < 0.1 && in > -0.1 && tmp != 1)
			return 0;

		/* no division by zero! */
		if((*c)->oscil_ctr/(*c)->oscil_dur & 1 == 0) {
			if(in < 0) {
				(*c)->oscil_ctr = tmp;
			} else {
				(*c)->oscil_ctr = 1;
			}
		} else {
			if(in < 0) {
				(*c)->oscil_ctr = 1;
			} else {
				(*c)->oscil_ctr = tmp;
			}
		}

		(*c)->oscil_dur = tmp;
		*energy++;
		return 0;

		case GENE_KILL_FORWARD:
		if(in < 2)
			victim = index_backward(w, x, y, (*c)->compass);
		else if(in > 2)
			victim = index_forward(w, x, y, (*c)->compass);
		else
			return 0;

		if(victim->type == 1) {
			(*c)->energy += victim->dead.energy;
			return 0;
		} else if(victim->type == 0)
			return 0;
		
		/* subtract the energy from the kill, but record the old energy
		*  that way we can subtract energy from them if they win
		*/
		tmp = (*c)->energy;
		(*c)->energy -= 2 + (victim->cell.energy <= 2)?
			-2 :
			victim->cell.energy - calc_death_energy(&victim->cell);

		if((*c)->energy == 0) {
			/* they beat us in a fight, die with honor
			*  but treat it like they killed us...
			*  except that we don't reroll if they died
			*/

			(*c)->energy = tmp;
			victim->cell.energy -= 2 + tmp - calc_death_energy(*c);
			goto die;
		}

		/* we beat them, now we reap the rewards */
		victim->type = 0;
		(*c)->energy += calc_death_energy(&victim->cell);
		
		return 0;
	}
}

void step_cell(struct tile *t, struct world *w, uint32_t x, uint32_t y)
{
	struct cell *c;
	if(t->type != 2 || t->cell.updated)
		return;

	c = &t->cell;

	if(!c->energy) {
		/* commit die */
		t->type = 1;
		t->dead.energy = calc_death_energy(c);
		return;
	}

	for(int i = 0; i < 4; i++) {
		float gene_input = 0;
		int actions;
		int tmp;
		c->genes[i] = SANITIZE_GENE(c->genes[i]);
		
		/* propagate the input */
		gene_input = input(c, w, x, y, c->genes[i]);
		
		/* do some weird math to make a strength that is limited
		*  but not flat, ie changes after hitting 4. Makes things more
		* interesting
		*/
		gene_input *= 4 * sinf(0.25 * strength_to_float(
			(uint16_t)((c->genes[i] & GENE_STRENGTH_BITS) >> 8)
		));
		gene_input = fminf(fmaxf(gene_input, -4.0), 4.0);

		tmp = output(&c, w, &x, &y, c->genes[i], gene_input, &actions);
		if(tmp == -1 && c == NULL) {
			/* we have committed suicide. let's return */
			return;
		}
	}

	c->updated = true;
}