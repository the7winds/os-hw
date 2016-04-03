#include "mem_info.h"
#include "utils.h"

extern uint32_t mboot_info;
extern char text_phys_begin[];
extern char bss_phys_end[];

uint8_t LENGTH;
MMAPStruct MMAP[MAX_LENGTH];
uint64_t MAX_PHYS_ADDR;


int initMMAPInfo() {
    uint32_t flags = *(uint32_t*) ((uint64_t) mboot_info);
    uint32_t* mmap_info = (uint32_t*) ((uint64_t) mboot_info + 44);

    if (flags & MMAP_FLAG) {
        void* nodePtr = (void*) ((uint64_t) mmap_info[1]);

        for (LENGTH = 0; LENGTH < MAX_LENGTH && !MMAP_end(nodePtr); ++LENGTH) {
            MMAP[LENGTH] = *getNodeByPtr(nodePtr);
            nodePtr = getNextPtr(nodePtr);
        }

        if (LENGTH == MAX_LENGTH && MMAP_end(nodePtr)) {
            printf("BAD MMAP\n");
            return 2;
        }

        MAX_PHYS_ADDR = MMAP[LENGTH - 1].base_addr + MMAP[LENGTH - 1].length;
        MMAP[0].type = 0;   // to make NULL incorrect, also it seems there're important information
    } else {
        printf("haven't 6th flag\n");
        return 1;
    }

    return 0;
}


void printMMAP() {
    printf("MEMORY MAP:\n");
    for (uint8_t i = 0; i < LENGTH; ++i) {
        uint64_t begin = MMAP[i].base_addr;
        uint64_t end = begin + MMAP[i].length - 1;
        printf("%llx - %llx, %s\n", begin, end, MMAP[i].type == 1 ? "Available" : "Reserved");
    } 
}


int reserveMemory(void* begin, uint64_t length) {
    uint8_t idx = 0;
    for (idx = 0; idx < LENGTH; ++idx) {
        if (MMAP[idx].type == 1
                && MMAP[idx].base_addr <= (uint64_t) begin
                && (uint64_t) begin + length <= (uint64_t) MMAP[idx].base_addr + MMAP[idx].length) {
            break;
        }
    }

    if (idx < LENGTH) {
        uint64_t l = 0;
        if ((uint64_t) begin > MMAP[idx].base_addr) {
            MMAP[LENGTH].base_addr = MMAP[idx].base_addr;
            MMAP[LENGTH].length = l = (uint64_t) begin - MMAP[idx].base_addr;
            MMAP[LENGTH++].type = 1;
        }
        
        if (MMAP[idx].length > l + length) {
            MMAP[LENGTH].base_addr = (uint64_t) begin + length;
            MMAP[LENGTH].length = MMAP[idx].length - l - length;
            MMAP[LENGTH++].type = 1;
        }

        MMAP[idx].base_addr = (uint64_t) begin;
        MMAP[idx].length = length;
        MMAP[idx].type = 0;
    } else {
        printf("can't reserve memory\n");
        return 1;
    }

    return 0;
}


int reserveKernelMemory() {
    uint64_t length = bss_phys_end - text_phys_begin + 1;
    return reserveMemory(text_phys_begin, length);
}
