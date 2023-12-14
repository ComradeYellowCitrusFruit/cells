/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_GENETICS_H__
#define CELLS_GENETICS_H__

#include <stdint.h>

#define GENE_TYPE_BIT                0x1    << 31
#define GENE_CODE_BITS               0x7f   << 24
/* gene strength is optional for output genes.
*  in output genes, strength changes the input such that:
*  in = map_range(in, -1.0, 1.0, -4.0, 4.0) * strength
*/
#define GENE_STRENGTH_BITS           0xffff <<  8
/* connect bits determine which genes an input propagates to
*  zeros are skipped, unless both are zero, in which case the default is the
*  next output. they are also interpreted mod 4
*/
#define GENE_CONNECT_BITS            0xff   <<  0
/* output genes don't have connect bits, and thus can pack if they have
*  a strength assigned to them in the connect bit space
*/
#define GENE_OUTPUT_HAS_STRENGTH_BIT 0x1    <<  0 

/* the defines for actual genetic code */
/* inputs first */
#define GENE_OBSTACLE_X       0x1
#define GENE_OBSTACLE_Y       0x2
#define GENE_OBSTACLE_FORWARD 0x3
#define GENE_FOOD_X           0x4
#define GENE_FOOD_Y           0x5
#define GENE_FOOD_FORWARD     0x6
#define GENE_DENSITY          0x7
#define GENE_ENERGY           0x8
#define GENE_LAST_X           0x9
#define GENE_LAST_Y           0xa
#define GENE_AGE              0xb
#define GENE_OSCILATOR        0xc
#define GENE_PHEROMONES       0xe

/* outputs first */
#define GENE_MOVE_X          0x1
#define GENE_MOVE_Y          0x2
#define GENE_MOVE_FORWARD    0x3
#define GENE_EMIT_PHEROMONES 0x4
#define GENE_SET_OSCILATOR   0x5
#define GENE_KILL_FORWARD    0x6
#define GENE_COMMIT_SUICIDE  0x7

typedef uint32_t gene_t;

/* strength uses the fp16 format 
*  so we need to be able to convert it to fp32
*/
float strength_to_float(uint16_t strength);

/* handles inheritance + mutation */
void duplicate_genes(gene_t parent[4], gene_t child[4]);

#endif