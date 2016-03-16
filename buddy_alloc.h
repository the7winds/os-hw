#ifndef __BUDDY_ALLOC_H__
#define __BUDDY_ALLOC_H__

#include <stdint.h>
#include "memory.h"
#include "slab_alloc.h"

#define MAX_ORDER 20
#define ORDER_SIZE(o) (PAGE_SIZE * (1 << (o)))

#define SMALL_ORDER 0
#define BIG_ORDER 1

typedef struct PageDscrptr PageDscrptr;

struct PageDscrptr {
    PageDscrptr* next;
    uint8_t order;
    uint8_t isFree;
    uint32_t begin;     // page number
    uint32_t attr;      // number of available block in phys memory
    Slab* slab;
} __attribute__((packed));

extern PageDscrptr* pages;

void* buddyAlloc(uint8_t order);

static inline void* buddyVAlloc(uint8_t order) {
    return (void*) VA((uint64_t) buddyAlloc(order));
}

void buddyFree(void* ptr, uint8_t order);

static inline void buddyVFree(void* ptr, uint8_t order) {
    buddyFree((void*)PA((uint64_t) ptr), order);
}

void printOrders();

int initBuddyAllocator();

uint64_t getPageNumByPtr(void* ptr);

void coverMemory();

void coverBlock(uint64_t begin, uint64_t end, uint32_t curBlockIdx);

int isLess(uint64_t idx, uint64_t order, uint64_t end);


#endif