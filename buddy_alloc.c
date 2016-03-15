#include "utils.h"
#include "mem_info.h"
#include "buddy_alloc.h"
#include "boot_alloc.h"
#include "memory.h"

static PageDscrptr** orders;    // lists

static uint32_t pagesAmount;
PageDscrptr* pages;      // all pages


void addAtBegin(PageDscrptr* node) {
    node->next = orders[node->order];
    orders[node->order] = node;
}

void* buddyAlloc(uint8_t order) {
    // printf("T1\n");
    if (orders[order]) {
        uint64_t n = orders[order]->begin;
        orders[order]->isFree = 0;
        orders[order] = orders[order]->next;
        return (void*) (n * PAGE_SIZE);
    }

    // printf("T2\n");
    // printOrders();
    PageDscrptr* node;
    for (uint8_t i = order + 1; i < MAX_ORDER; ++i) {
        if (orders[i]) {
            node = orders[i];
            orders[i] = orders[i]->next;
            break;
        }
    }
    // printf("B::%llx\n", node->begin);
    // printf("T3\n");
    while (node->order > order) {
        // printf("T!\n");
        node->order--;
        PageDscrptr* node1 = pages + (node->begin + (1 << node->order));
        node1->begin = node->begin + (1 << node->order); 
        node1->order = node->order;
        node1->isFree = 1;
        node1->attr = node->attr;
        addAtBegin(node1);
    }
    // printOrders();

    node->isFree = 0;
    return (void*) ((uint64_t) (1 << node->order) * PAGE_SIZE);
}

void printOrders() {
    for (int i = 0; i < MAX_ORDER; ++i) {
        printf("%d:", i);
        PageDscrptr* ptr = orders[i];
        while (ptr) {
            printf("%llx -> ", ptr->begin);
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
    addAtBegin(node);
}


void initBuddyAllocator() {
    initBootAllocator();

    uint64_t physMemory = countPhysMemory();
    pagesAmount = physMemory / (PAGE_SIZE + sizeof(PageDscrptr));
   
    printf("PhysMemory: 0x%llx\n", physMemory); 
    printf("Page size: 0x%llx\n", PAGE_SIZE);
    printf("Amount of pages: 0x%llx\n", pagesAmount);
    printf("Memory for page descriptors: 0x%llx\n", sizeof(PageDscrptr) * pagesAmount);

    while (pages == NULL) {
        pages = (PageDscrptr*) boot_alloc(pagesAmount * sizeof(PageDscrptr));
        pagesAmount--;
    }
    printMemMap();

    printf("Amount of pages: 0x%llx\n", pagesAmount);
    orders = (PageDscrptr**) boot_alloc(MAX_ORDER * sizeof(PageDscrptr*));
    printf("Memory for lists: 0x%llx\n", MAX_ORDER * sizeof(PageDscrptr*));

    coverMemory();
}


void coverMemory() {
    uint32_t curBlockIdx = 0;
    MemMapStruct* dscrpt = BEGIN;
    do {
        if (dscrpt->type == 1) {
            ++curBlockIdx;
            uint64_t begin = dscrpt->base_addr;
            uint64_t end = begin + dscrpt->length;
            begin = (begin / PAGE_SIZE + begin % PAGE_SIZE) * PAGE_SIZE;
            coverBlock(begin, end, curBlockIdx);
        }
        dscrpt = nextMemMapStruct(dscrpt);
    } while(!MEMMAP_END(dscrpt));

    printf("Covered\n");
}


void coverBlock(uint64_t begin, uint64_t end, uint32_t curBlockIdx) {
    uint64_t idx = begin / PAGE_SIZE;
    while (isLess(idx, 0, end)) {
        uint64_t order = 0;
        uint64_t neighbour;
        while (isLess(idx, order + 1, end) && idx < (neighbour = idx ^ (1 << (order + 1)))) {
            ++order;
        }
        
        pages[idx].order = order;
        pages[idx].isFree = 1;
        pages[idx].begin = idx;
        pages[idx].attr = curBlockIdx;
        pages[idx].next = orders[order];
        addAtBegin(pages + idx);

        idx += (1 << order);
    }
}


int isLess(uint64_t idx, uint64_t order, uint64_t end) {
    return ((idx + (1 << order) - 1) * PAGE_SIZE < end);
}
