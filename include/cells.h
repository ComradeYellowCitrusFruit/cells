#ifndef CELLS_CELLS_H__
#define CELLS_CELLS_H__

#include "include/genetics.h"

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

#endif