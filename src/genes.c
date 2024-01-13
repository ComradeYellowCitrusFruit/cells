/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <string.h>
#include "include/genetics.h"
#include "include/rng.h"
#include "include/util.h"

extern generator_handle main_rng;

/* handles inheritance + mutation */
void duplicate_genes(gene_t parent[4], gene_t child[4]) {
	uint16_t dice_roll_a = gen16(main_rng);
	memcpy(child, parent, 16);

	if(dice_roll_a == 0xabcd) {
		child[gen8(main_rng) % 4] ^= 1 << gen8(main_rng) % 8;

		if(unlikely(
			(gen64(main_rng) & 0xffffffffffff) == 0x63616d626961
		))
			parent[gen8(main_rng) % 4] ^= 1 << gen8(main_rng) % 8;
	} else if(dice_roll_a == 0xef00) {
		parent[gen8(main_rng) % 4] ^= 1 << gen8(main_rng) % 8;
		
		if(unlikely(
			(gen64(main_rng) & 0xffffffffffff) == 0x63616d626961
		))
			child[gen8(main_rng) % 4] ^= 1 << gen8(main_rng) % 8;
	}
}