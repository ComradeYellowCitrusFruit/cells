/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#ifndef CELLS_GENETICS_H__
#define CELLS_GENETICS_H__

#include <stdint.h>

/* where GENE is the gene, 
*  GENE.output is the integer at GENE[7:0], 
*  GENE.strength is the fp16 at GENE[23:8],
*  GENE.input is the integer at GENE[31:24],
*  output(GENE.output, input(GENE.input) * GENE.strength) 
*/

#define GENE_INPUT_BITS    (0xff   << 24)
#define GENE_STRENGTH_BITS (0xffff <<  8)
#define GENE_OUTPUT_BITS   (0xff   <<  0)

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

/* outputs second */
#define GENE_MOVE_X          0x1
#define GENE_MOVE_Y          0x2
#define GENE_MOVE_FORWARD    0x3
#define GENE_SET_OSCILATOR   0x4
#define GENE_KILL_FORWARD    0x5
#define GENE_COMMIT_SUICIDE  0x6

#define SANITIZE_GENE(g) ( \
        ((((g & GENE_INPUT_BITS) >> 24) % 0xf) << 24) | \
        (g & GENE_OUTPUT_BITS % 7) | \
        (g & GENE_STRENGTH_BITS)) /* TODO: sanitize strenth too*/

typedef uint32_t gene_t;

/* strength uses the fp16 format
*  so we need to be able to convert it to fp32
*/
__attribute__ ((__pure__)) float strength_to_float(uint16_t strength);

/* handles inheritance + mutation */
void duplicate_genes(gene_t parent[4], gene_t child[4]);

#endif