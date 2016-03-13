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

void initFixedAllocator();

FixedAllocator* newFixedAllocator(uint16_t size, uint16_t align);

void* fixedAllocate(FixedAllocator* fixedAllocator);

void fixedFree(void* ptr);

void moveSlabToEmpty(FixedAllocator* fixedAllocator, Slab* slab);

void moveSlabToPartly(FixedAllocator* fixedAllocator, Slab* slab);


#endif /* __FIXED_ALLOC_H__ */