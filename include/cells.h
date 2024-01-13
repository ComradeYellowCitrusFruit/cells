/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_CELLS_H__
#define CELLS_CELLS_H__

#include <stdbool.h>
#include "include/genetics.h"
#include "include/rng.h"

#define COMPASS_UP 1
#define COMPASS_DOWN 2
#define COMPASS_LEFT 3
#define COMPASS_RIGHT 4

#define NORTH 1
#define SOUTH 2
#define EAST 4
#define WEST 3

struct cell {
	uint64_t id;

	uint32_t energy;
	unsigned int age;

	unsigned int oscil_dur;
	unsigned int oscil_ctr;

	int compass;

	gene_t genes[4];

        bool updated;
};

struct dead_thing {
	uint64_t id;
	uint32_t energy;
};

void gen_random_cell(struct cell *c, generator_handle g);

uint32_t calc_death_energy(struct cell *c);

#endif