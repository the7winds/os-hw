#ifndef __MEM_INFO__
#define __MEM_INFO__

#include <stdint.h>
#include "utils.h"

struct MMAPStruct {
    uint64_t base_addr;
    uint64_t length;
    uint8_t type;
} __attribute__((packed));

typedef struct MMAPStruct MMAPStruct;

#define MAX_LENGTH 100
#define SIZE_OF_SIZE 4
#define MMAP_FLAG (1 << 6)

extern uint8_t LENGTH;
extern MMAPStruct MMAP[MAX_LENGTH];
extern uint64_t MAX_PHYS_ADDR;

int initMMAPInfo();

static inline int MMAP_end(void* ptr) {
	return (*(uint32_t*) ptr) < 20;
}

static inline void* getNextPtr(void *ptr) {
    uint64_t shift = *((uint32_t*) ptr) + SIZE_OF_SIZE;
    return (void*) ((uint64_t) ptr + shift);
}


static inline MMAPStruct* getNodeByPtr(void* nodePtr) {
    return (MMAPStruct*) ((uint64_t) nodePtr + SIZE_OF_SIZE);
}

void printMMAP();

int reserveKernelMemory();

int reserveMemory(void* begin, uint64_t length);

#endif /* __MEM_INFO__ */