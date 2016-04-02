#include "utils.h"
#include "mem_info.h"
#include "buddy_alloc.h"
#include "boot_alloc.h"
#include "lock.h"

static PageDscrptr** orders;    // lists

static uint32_t pagesAmount;
PageDscrptr* pages;             // all pages

static Lock buddyLock = false;

void* buddyAlloc(uint8_t order) {
    if (orders[order]) {
        uint64_t n = orders[order]->begin;
        orders[order]->isFree = 0;
        orders[order] = orders[order]->next;
        return (void*) (n * PAGE_SIZE);
    }

    PageDscrptr* node;
    for (uint8_t i = order + 1; i < MAX_ORDER; ++i) {
        if (orders[i]) {    // remove head
            node = orders[i];
            orders[i] = orders[i]->next;
            break;
        }
    }
    
    while (node->order > order) {
        node->order--;
        uint32_t neighbour = (node->begin ^ (1 << node->order));
        pages[neighbour].begin = neighbour;
        pages[neighbour].order = node->order;
        pages[neighbour].isFree = 1;
        pages[neighbour].attr = node->attr;
        pages[neighbour].next = orders[pages[neighbour].order]; // adding to list
        orders[pages[neighbour].order] = pages + neighbour;
    }

    node->isFree = 0;

    return (void*) ((uint64_t) (node->begin) * PAGE_SIZE);
}

void printOrders() {
    for (int i = 0; i < MAX_ORDER; ++i) {
        printf("%d:", i);
        PageDscrptr* ptr = orders[i];
        while (ptr) {
            printf("%llx -> ", (uint64_t) ptr->begin * PAGE_SIZE);
            ptr = ptr->next;
        }
        printf("NULL\n");
    }
}

int canMerge(PageDscrptr* node) {
    PageDscrptr* neighbour = pages + (node->begin ^ (1 << node->order));
    return (neighbour->isFree 
                && neighbour->order == node->order 
                && neighbour->attr == node->attr);
}

PageDscrptr* mergeNodes(PageDscrptr* node) {
    PageDscrptr* neighbour = pages + (node->begin ^ (1 << node->order));
    node->begin = (neighbour->begin < node->begin 
                                    ? neighbour->begin 
                                    : node->begin);
    node->order++;
    return node;
}


void buddyFree(void* ptr, uint8_t order) {
    uint64_t ptrInt = (uint64_t) ptr;
    PageDscrptr* node = pages + (ptrInt / PAGE_SIZE);
    node->isFree = 1;

    while (order < MAX_ORDER && canMerge(node)) {
        node = mergeNodes(node);
    }
 
    node->isFree = 1;

    node->next = orders[node->order];
    orders[node->order] = node;
}


int initBuddyAllocator() {
    pagesAmount = MAX_PHYS_ADDR / (PAGE_SIZE + sizeof(PageDscrptr));
   
    printf("Max phys addr: 0x%llx\n", MAX_PHYS_ADDR); 
    printf("Page size: 0x%llx\n", PAGE_SIZE);
    printf("Amount of pages: 0x%llx\n", pagesAmount);
    printf("Memory for pages dscrptrs: 0x%llx\n", pagesAmount * sizeof(PageDscrptr));

    pages = (PageDscrptr*) boot_alloc(pagesAmount * sizeof(PageDscrptr));
    if (pages == NULL) {
        return 1;
    }

    printf("resered memory for pages dscrptrs\n");

    orders = (PageDscrptr**) boot_alloc(MAX_ORDER * sizeof(PageDscrptr*));
    if (orders == NULL) {
        return 2;
    }

    printf("resered memory for lists\n");

    printf("initing lists...\n");

    coverMemory();

    return 0;
}


void coverMemory() {
    for (int i = 0; i < LENGTH; ++i) {
        if (MMAP[i].type == 1) {
            uint64_t begin = MMAP[i].base_addr;
            uint64_t end = begin + MMAP[i].length;  // byte after last
            begin = (begin / PAGE_SIZE + begin % PAGE_SIZE) * PAGE_SIZE;
            coverBlock(begin, end, i);
        }
    }
}


void coverBlock(uint64_t begin, uint64_t end, uint32_t blockIdx) {
    for (uint64_t order = 0, idx = begin / PAGE_SIZE; isLess(idx, 0, end); idx += (1 << order)) {
        while (isLess(idx, order + 1, end) && idx < (idx ^ (1 << order))) {
            ++order;
        }
        pages[idx].order = order;
        pages[idx].isFree = 1;
        pages[idx].begin = idx;
        pages[idx].attr = blockIdx;
        pages[idx].next = orders[order];
        orders[order] = pages + idx;
    }
}


int isLess(uint64_t idx, uint64_t order, uint64_t end) {
    return ((idx + (1 << order)) * PAGE_SIZE <= end);
}


void* buddyVAlloc(uint8_t order) {
    lock(&buddyLock);
    void* ptr = (void*) VA((uint64_t) buddyAlloc(order));
    unlock(&buddyLock);
    return ptr;
}


void buddyVFree(void* ptr, uint8_t order) {
    lock(&buddyLock);
    buddyFree((void*)PA((uint64_t) ptr), order);
    unlock(&buddyLock);
}