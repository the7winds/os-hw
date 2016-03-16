#ifndef __BOOT_ALLOC_H__
#define __BOOT_ALLOC_H__

#include "mem_info.h"

#define FIRST4G ((uint64_t) 1 << 32)

void* boot_alloc(uint64_t length);

void* search_free_memory(uint64_t length);

#endif