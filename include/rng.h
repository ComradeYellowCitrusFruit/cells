/* spdx-identifier: GPL-3.0-only */

#ifndef CELLS_RNG_H__
#define CELLS_RNG_H__

#include <stdint.h>

typedef struct rng_generator *generator_handle;

generator_handle new_generator();
void free_generator(generator_handle h);

void reseed(generator_handle h);
void refresh(generator_handle h);

uint8_t gen8(generator_handle h);
uint16_t gen16(generator_handle h);
uint32_t gen32(generator_handle h);
uint64_t gen64(generator_handle h);
void gen_bytes(generator_handle h, void *data, uint64_t bytes);

#endif