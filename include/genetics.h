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

typedef uint32_t gene_t;

/* strength uses the fp16 format 
*  so we need to be able to convert it to fp32
*/
float strength_to_float(uint16_t strength);

/* handles inheritance + mutation */
void duplicate_genes(gene_t parent[4], gene_t child[4]);

#endif