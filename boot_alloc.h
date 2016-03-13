#ifndef __BOOT_ALLOC_H__
#define __BOOT_ALLOC_H__

#include "mem_info.h"

void initBootAllocator();

void* boot_alloc(uint64_t length);

#endif