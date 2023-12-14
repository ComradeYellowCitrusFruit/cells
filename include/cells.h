/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_CELLS_H__
#define CELLS_CELLS_H__

#include "include/genetics.h"
#include "include/rng.h"

struct cell {
	uint64_t id;

	uint32_t energy;
	unsigned int age;

	unsigned int oscilator_period;
	unsigned int oscilator_ctr;

	gene_t genes[4];
};

struct dead_thing {
	uint64_t id;
	uint32_t energy;
};

void gen_random_cell(struct cell *c, generator_handle g);

#endif