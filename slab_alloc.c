#include "slab_alloc.h"
#include "buddy_alloc.h"
#include "fixed_alloc.h"


// help funcs


FixedAllocator forAllocators;
FixedAllocator* forBigSlabStruct;
FixedAllocator* forBigSlabNode;

Slab* getSlabByPtr(void* ptr) {
    uint32_t idx = PA((uint64_t) ptr) / PAGE_SIZE;
    return pages[idx].slab;
}

PageDscrptr* getPageDscrptByVA(void* va) {
    uint64_t VA = (uint64_t) va;
    return pages + PA(VA) / PAGE_SIZE;
}

// small slab func

Slab* newSmallSlab(uint16_t size, uint16_t align) {
    void* page = buddyVAlloc(SMALL_ORDER);
    
    // place slab structure in the end of the page
    Slab* slab = (Slab*) ((uint64_t) page + (ORDER_SIZE(SMALL_ORDER) - sizeof(Slab)));

    // init slab structure
    slab->counter = 0;
    slab->size = size;
    slab->ctrl = NULL;
    slab->prev = slab->next = NULL;

    uint64_t blockSize = (size + sizeof(SlabNode));
    uint64_t shift = (blockSize / align + blockSize % align) * align;
    uint64_t dataBegin = ((uint64_t) page / align + (uint64_t) page % align) * align;

    // build list structure
    for (uint64_t ptr = dataBegin; ptr + shift <= (uint64_t) slab; ptr += shift) {
        slab->limit++;
        SlabNode* node = (SlabNode*) (ptr + size);
        node->data = (void*) ptr;
        node->next = slab->ctrl;
        slab->ctrl = (void*) node;
    }

    PageDscrptr* pageDscrptr = getPageDscrptByVA(page);
    pageDscrptr->slab = slab;

    return slab;
}


void* smallSlabAlloc(Slab* slab) {
    void* data = slab->ctrl->data;
    slab->ctrl = slab->ctrl->next;
    slab->counter++;
    return data;
}


void smallSlabFree(Slab* slab, void* ptr) {
    SlabNode* node = (SlabNode*) ((uint64_t) ptr + slab->size);
    node->next = slab->ctrl;
    slab->ctrl = node;
    slab->counter--;
}


void smallSlabDelete(Slab* slab) {
    void* page = (void*) ((uint64_t) slab - (ORDER_SIZE(SMALL_ORDER) - sizeof(Slab)));
    buddyVFree(page, SMALL_ORDER);
}


// Big Slab funcs

Slab* newBigSlab(uint16_t size, uint16_t align) {
    void* page = buddyVAlloc(BIG_ORDER);
    Slab* slab = (Slab*) fixedAllocate(forBigSlabStruct);

    uint64_t shift = (size / align + size % align) * align;
    uint64_t dataBegin = ((uint64_t) page / align + (uint64_t) page % align) * align;

    slab->size = size;
    slab->counter = 0;
    slab->limit = 0;
    slab->ctrl = NULL;
    slab->prev = slab->next = NULL;

    for (uint64_t ptr = dataBegin; ptr <= (uint64_t) page + ORDER_SIZE(BIG_ORDER); ptr += shift) {
        slab->limit++;
        SlabNode* node = (SlabNode*) fixedAllocate(forBigSlabNode);
        node->data = (void*) ptr;
        node->next = slab->ctrl;
        slab->ctrl = node;
    }

    // setting on both pages slab
    PageDscrptr* pageDscrptr = getPageDscrptByVA(page);
    pageDscrptr->slab = slab;
    pageDscrptr++;
    pageDscrptr->slab = slab;

    return slab;
}


void* bigSlabAlloc(Slab* slab) {
    SlabNode* node = slab->ctrl;
    slab->ctrl = node->next;
    slab->counter++;
    fixedFree(node);
    return node->data;
}


void bigSlabFree(Slab* slab, void* ptr) {
    SlabNode* node = (SlabNode*) fixedAllocate(forBigSlabNode);
    node->data = ptr;
    node->next = slab->ctrl;
    slab->ctrl = node;
    slab->counter--;
}


void bigSlabDelete(Slab* slab) {
    void* page = (void*) (((uint64_t) slab->ctrl->data / (ORDER_SIZE(BIG_ORDER) * PAGE_SIZE)) * PAGE_SIZE);
    buddyVFree(page, BIG_ORDER);
    
    while (slab->ctrl) {
        SlabNode* node = slab->ctrl;
        slab->ctrl = node->next;
        fixedFree(node);
    }

    fixedFree(slab);
}