#ifndef __MEM_INFO__
#define __MEM_INFO__

#include <stdint.h>
#include "utils.h"

#define MEMMAP_END(ptr) ((ptr)->size < 20 || (ptr) < BEGIN || (ptr) >= BEGIN + LENGTH)

void initMemMapInfo();

struct MemMapStruct* nextMemMapStruct(struct MemMapStruct *dscrpt);

struct MemMapStruct {
    uint32_t size; 
    uint64_t base_addr;
    uint64_t length;
    uint8_t type;
} __attribute__((packed));

typedef struct MemMapStruct MemMapStruct;

extern uint32_t LENGTH;
extern MemMapStruct* BEGIN;
extern MemMapStruct* END;


void printMemMap();

void reserveMemory(void* begin, uint64_t length);

void reserveKernelMemory();

void* search_free_memory(uint64_t length);

uint32_t getFreeSize();

uint64_t countPhysMemory();

#endif /* __MEM_INFO__ */