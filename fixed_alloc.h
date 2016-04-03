#ifndef __FIXED_ALLOC_H__
#define __FIXED_ALLOC_H__

#include <stdint.h>
#include "utils.h"
#include "memory.h"
#include "slab_alloc.h"

struct FixedAllocator {
    uint16_t size;
    uint16_t align;
    Slab* empty;
    Slab* partly;
    Slab* full;
};

typedef struct FixedAllocator FixedAllocator;

int initFixedAllocator();

FixedAllocator* newFixedAllocator(uint16_t size, uint16_t align);

void* fixedAllocate(FixedAllocator* fixedAllocator);

void fixedFree(void* ptr);

void deleteFixedAllocator(FixedAllocator* fixedAllocator);

void moveSlabToEmpty(FixedAllocator* fixedAllocator, Slab* slab);

void moveSlabToPartly(FixedAllocator* fixedAllocator, Slab* slab);

static inline int isSlabEmpty(Slab* slab) {
    return (slab->counter == slab->limit);
}

static inline int isSlabPartly(Slab* slab) {
    return (0 < slab->counter && slab->counter < slab->limit);
}

static inline int isSlabFull(Slab* slab) {
    return (slab->counter == 0);
}

void cutSlab(Slab* slab);

void changeHeadIfNeed(FixedAllocator *fixedAllocator, Slab *slab);

#endif /* __FIXED_ALLOC_H__ */