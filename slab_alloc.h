#ifndef __SLAB_ALLOC_H__
#define __SLAB_ALLOC_H__

#include <stdint.h>
#include "utils.h"

typedef struct Slab Slab;

typedef struct SlabNode SlabNode;

struct SlabNode {
    SlabNode* next;
    void* data;
} __attribute__((packed));

struct Slab {
    uint16_t counter;
    uint16_t limit;
    uint16_t size;
    SlabNode* ctrl;
    Slab* next;
    Slab* prev;
} __attribute__((packed));

#include "buddy_alloc.h"

#define BIG_OBJECT_SIZE (PAGE_SIZE >> 3)

// small slab funcs

Slab* newSmallSlab(uint16_t size, uint16_t align);

void* smallSlabAlloc(Slab* slab);

void smallSlabFree(Slab* slab, void* ptr);

void smallSlabDelete(Slab* slab);


// big slab funcs

Slab* newBigSlab(uint16_t size, uint16_t align);

void* bigSlabAlloc(Slab* slab);

void bigSlabFree(Slab* slab, void* ptr);

void bigSlabDelete(Slab* slab);

// help funcs

Slab* getSlabByPtr(void* ptr);

// main funcs

static inline Slab* newSlab(uint16_t size, uint16_t align) {
    return (size < BIG_OBJECT_SIZE ? newSmallSlab(size, align) : newBigSlab(size, align));
}

static inline void* slabAlloc(Slab* slab) {
    return (slab->size < BIG_OBJECT_SIZE ? smallSlabAlloc(slab) : bigSlabAlloc(slab));
}

static inline void slabFree(void* ptr) {
    Slab* slab = getSlabByPtr(ptr);
    slab->size < BIG_OBJECT_SIZE ? smallSlabFree(slab, ptr) : bigSlabFree(slab, ptr);
}

static inline void deleteSlab(Slab* slab) {
    slab->size < BIG_OBJECT_SIZE ? smallSlabDelete(slab) : bigSlabDelete(slab);
}

#endif