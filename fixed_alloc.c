#include <stdbool.h>
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
    lock(&fixedAllocator->lock);
    Slab* slab;

    while (true) {
        slab = (fixedAllocator->partly ? fixedAllocator->partly : fixedAllocator->full);

        if (slab == NULL) {
            fixedAllocator->partly = slab = newSlab(fixedAllocator);
            break;
        } else {
            if (isSlabEmpty(slab)) {
                moveSlabToEmpty(fixedAllocator, slab);
                continue;
            } else if (isSlabPartly(slab)) {
                moveSlabToPartly(fixedAllocator, slab);
            }
            break;
        }
    }
    
    void* ptr = slabAlloc(slab);
    unlock(&fixedAllocator->lock);
    
    return ptr;
}

FixedAllocator* newFixedAllocator(uint16_t size, uint16_t align) {
    FixedAllocator* fixedAllocator = (FixedAllocator*) fixedAllocate(&forAllocators);
    fixedAllocator->size = size;
    fixedAllocator->align = align;
    fixedAllocator->lock = false;
    fixedAllocator->empty = fixedAllocator->partly = fixedAllocator->full = NULL;
    return fixedAllocator;
}

void fixedFree(void* ptr) {
    Slab* slab = getSlabByPtr(ptr);
    FixedAllocator* fixedAllocator = slab->fixedAllocator;
    lock(&fixedAllocator->lock);
    slabFree(ptr);
    if (isSlabFull(slab)) {
        changeHeadIfNeed(slab->fixedAllocator, slab);
        cutSlab(slab);
        deleteSlab(slab);
    }
    unlock(&fixedAllocator->lock);
}

void moveSlabToEmpty(FixedAllocator* fixedAllocator, Slab* slab) {
    changeHeadIfNeed(fixedAllocator, slab);
    cutSlab(slab);
    slab->next = fixedAllocator->empty;
    fixedAllocator->empty = slab;
}

void moveSlabToPartly(FixedAllocator* fixedAllocator, Slab* slab) {
    changeHeadIfNeed(fixedAllocator, slab);
    cutSlab(slab);
    slab->next = fixedAllocator->partly;
    fixedAllocator->partly = slab;
}

void deleteFixedAllocator(FixedAllocator* fixedAllocator) {
    fixedFree(fixedAllocator);
}

void cutSlab(Slab* slab) {
    if (slab->prev) {
        slab->prev->next = slab->next;
    }
    if (slab->next) {
        slab->next->prev = slab->prev;
    }
    slab->prev = slab->next = NULL;
}

void changeHeadIfNeed(FixedAllocator *fixedAllocator, Slab *slab) {
    Slab* next = slab->next;
    if (slab == fixedAllocator->empty) {
        fixedAllocator->empty = next;
    } else if (slab == fixedAllocator->partly) {
        fixedAllocator->partly = next;
    } else if (slab == fixedAllocator->full) {
        fixedAllocator->full = next;
    }
}