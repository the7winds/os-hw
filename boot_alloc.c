#include "mem_info.h"
#include "memory.h"
#include "boot_alloc.h"

static uint8_t allocs;

void* boot_alloc(uint64_t length) {
    
    if (allocs < MAX_LENGTH - LENGTH) {
        allocs++;
        void* ptr = search_free_memory(length);
        if (reserveMemory(ptr, length) == 0) {
            return va((uint64_t)ptr);
        }
    }
    
    printf("allocations limit exceed\n");
    return NULL;
}

void* search_free_memory(uint64_t length) {
    for (uint8_t i = 0; i < LENGTH; ++i) {
        if (MMAP[i].type == 1 
        		&& length <= MMAP[i].length
        		&& MMAP[i].base_addr + length < FIRST4G) {

            return (void*) MMAP[i].base_addr;
        }
    }
    return 0;
}