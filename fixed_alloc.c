#include "fixed_alloc.h"

extern FixedAllocator forAllocators;
extern FixedAllocator* forBigSlabStruct;
extern FixedAllocator* forBigSlabNode;

int initFixedAllocator() {
    forAllocators.size = sizeof(FixedAllocator);
    forAllocators.align = 4;
    forAllocators.empty = forAllocators.partly = forAllocators.full = NULL;

    forBigSlabStruct = newFixedAllocator(sizeof(Slab), 4);
    forBigSlabNode = newFixedAllocator(sizeof(SlabNode), 4);

    return (forBigSlabNode && forBigSlabStruct ? 0 : 1);
}

void* fixedAllocate(FixedAllocator* fixedAllocator) {
    Slab* slab = (fixedAllocator->partly ? fixedAllocator->partly : fixedAllocator->full);
    if (slab == NULL) {
        fixedAllocator->partly = newSlab(fixedAllocator->size, fixedAllocator->align);
        return fixedAllocate(fixedAllocator);
    } else {
        void* ptr = slabAlloc(slab);
        if (isSlabEmpty(slab)) {
            moveSlabToEmpty(fixedAllocator, slab);
        } else if (isSlabPartly(slab)) {
            moveSlabToPartly(fixedAllocator, slab);
        }
        return ptr;
    }
}

FixedAllocator* newFixedAllocator(uint16_t size, uint16_t align) {
    FixedAllocator* fixedAllocator = (FixedAllocator*) fixedAllocate(&forAllocators);
    fixedAllocator->size = size;
    fixedAllocator->align = align;
    fixedAllocator->empty = fixedAllocator->partly = fixedAllocator->full = NULL;
    return fixedAllocator;
}

void fixedFree(void* ptr) {
    Slab* slab = getSlabByPtr(ptr);
    slabFree(ptr);
    if (isSlabFull(slab)) {
        if (slab->prev) {
            slab->prev->next = slab->next;
        }
        if (slab->next) {
            slab->next->prev = slab->prev;
        }
        deleteSlab(slab);
    }
}

void moveSlabToEmpty(FixedAllocator* fixedAllocator, Slab* slab) {
    if (slab->prev) {
        slab->prev->next = slab->next;
    }
    if (slab->next) {
        slab->next->prev = slab->prev;
    }
    slab->prev = NULL;
    slab->next = fixedAllocator->empty;
    fixedAllocator->empty = slab;
}

void moveSlabToPartly(FixedAllocator* fixedAllocator, Slab* slab) {
    if (slab->prev) {
        slab->prev->next = slab->next;
    }
    if (slab->next) {
        slab->next->prev = slab->prev;
    }
    slab->prev = NULL;
    slab->next = fixedAllocator->partly;
    fixedAllocator->partly = slab;
}

void deleteFixedAllocator(FixedAllocator* fixedAllocator) {
    fixedFree(fixedAllocator);
}