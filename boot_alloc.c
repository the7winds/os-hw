#include "mem_info.h"
#include "boot_alloc.h"

static int counter;
static int LIMIT;

void initBootAllocator() {
    LIMIT = getFreeSize();
    printf("LIMIT: %d\n", LIMIT);
}

void* boot_alloc(uint64_t length) {
    if (counter < LIMIT) {

        void* ptr = search_free_memory(length);
        
        if (ptr == NULL) {
        	printf("Can't find enough memory: %llx\n", length);
        	return NULL;
        }
        
        ++counter;
        reserveMemory(ptr, length);    
        return ptr;
    }
    printf("Allocations limit exceed\n");
    return NULL;
}