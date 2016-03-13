#include "mem_info.h"
#include "utils.h"
#include "memory.h"

#define NEXT_NODE_SIZE 20

extern uint32_t mboot_info;
extern char text_phys_begin[];
extern char bss_phys_end[];

uint32_t LENGTH;
MemMapStruct* BEGIN;
MemMapStruct* END;
MemMapStruct* LAST;
uint64_t MAX_PHYS_ADDR;


uint32_t getFreeSize() {
    return (LENGTH - ((uint64_t) END - (uint64_t) BEGIN)) / sizeof(MemMapStruct);
}


void initMemMapInfo() {
    uint64_t mboot_info_ptr_int = mboot_info;

    uint32_t flags = *((uint32_t*) mboot_info_ptr_int);

    if (flags & (1 << 6)) {
        LENGTH = *((uint32_t*) (mboot_info_ptr_int + 44));
        LAST = END = BEGIN = (MemMapStruct*) ((uint64_t) *((uint32_t*) (mboot_info_ptr_int + 48)));
        MAX_PHYS_ADDR = 0;
        do {
            END = nextMemMapStruct(LAST = END);
        } while (!MEMMAP_END(END));
        MAX_PHYS_ADDR = LAST->base_addr + LAST->length;
    } else {
        printf("haven't 6th flag\n");
    }
}


MemMapStruct* nextMemMapStruct(MemMapStruct *dscrpt) {
    return (MemMapStruct*) ((uint64_t) dscrpt + sizeof(dscrpt->size) + dscrpt->size);
}


void printMemMap() {
    printf("MEMORY MAP:\n");
    MemMapStruct* dscrpt = BEGIN;
    do {
        uint64_t begin = dscrpt->base_addr;
        uint64_t end = begin + dscrpt->length - 1;
        printf("%llx - %llx, %s\n", begin, end, dscrpt->type == 1 ? "Available" : "Reserved");
        dscrpt = nextMemMapStruct(dscrpt);
    } while (!MEMMAP_END(dscrpt));
}


MemMapStruct* getNewNode() {
    MemMapStruct* tmp = END;
    tmp->size = NEXT_NODE_SIZE;
    END = nextMemMapStruct(tmp);

    return tmp;
}


void reserveMemory(void* begin, uint64_t length) {
    MemMapStruct* dscrpt = NULL;
    MemMapStruct* i = BEGIN; 
    do {
        if (i->type == 1
                && i->base_addr <= (uint64_t) begin
                && (uint64_t) begin + length <= (uint64_t) i->base_addr + i->length) {
            dscrpt = i;
            break;
        }
        i = nextMemMapStruct(i);
    } while(!MEMMAP_END(i));

    if (dscrpt) {
        MemMapStruct* node;
        if ((uint64_t) begin > dscrpt->base_addr) {
            node = getNewNode();
            node->base_addr = dscrpt->base_addr;
            node->length = (uint64_t) begin - dscrpt->base_addr;
            node->type = 1;
        }

        uint64_t l = ((uint64_t) begin > dscrpt->base_addr ? (uint64_t) begin > dscrpt->base_addr : 0);
        if (dscrpt->length > l + length) {
            node = getNewNode();
            node->base_addr = (uint64_t) begin + length;
            node->length = dscrpt->length - l - length;
            node->type = 1;
        }

        dscrpt->base_addr = (uint64_t) begin;
        dscrpt->length = length;
        dscrpt->type = 0;
    } else {
        printf("can't reserve memory\n");
    }
}


void reserveKernelMemory() {
    uint64_t length = bss_phys_end - text_phys_begin + 1;
    reserveMemory(text_phys_begin, length);
}

#define AVAILABLE 0x100000000

void* search_free_memory(uint64_t length) {
    MemMapStruct* i = BEGIN;
    // to not return NULL
    i = nextMemMapStruct(i);
    do {
        if (i->type == 1 && length <= i->length && i->base_addr + length < AVAILABLE) {
            return (void*) i->base_addr;
        }
        i = nextMemMapStruct(i);
    } while (!MEMMAP_END(i));
    return NULL;
}


uint64_t countPhysMemory() {
    uint64_t res = 0;
    MemMapStruct* dscrptr = BEGIN;
    do {
        res += dscrptr->length;
        dscrptr = nextMemMapStruct(dscrptr);
    } while(!MEMMAP_END(dscrptr));

    return res;
}