#include "utils.h"
#include "paging.h"
#include "buddy_alloc.h"

extern uint64_t MAX_PHYS_ADDR;

    
void clearPage(void* page) {
    for (uint64_t* ptr = page; (uint64_t) ptr < (uint64_t) page + PAGE_SIZE; ++ptr) {
        *ptr = 0;
    }
}


void ifFeildEmptyCreate(pte_t* field) {
    if (pte_present(*field) == 0) {
        pte_t page = (pte_t) buddyAlloc(0);
        clearPage((void*) page);
        (*field) = page | PTE_PRESENT | PTE_WRITE;
    }
}


void addToTable(pte_t* pml4, uint64_t virt, uint64_t phys) {
    pte_t* pml4e = (pte_t*) (uint64_t) pml4 + pml4_i(virt);
   
    ifFeildEmptyCreate(pml4e);
    
    pte_t* pdpte = (pte_t*) (pte_phys(*pml4e) << 12) + pml3_i(virt);

    ifFeildEmptyCreate(pdpte);

    pte_t* pde = (pte_t*) (pte_phys(*pdpte) << 12) + pml2_i(virt);

    (*pde) = phys | PTE_PRESENT | PTE_WRITE | PTE_LARGE;
}


void setUpPaging() {
    pte_t* pml4 = (pte_t*) buddyAlloc(0);

    clearPage(pml4);

    printf("pml4: %llx\n", pml4);

    uint64_t phys = 0;
    for (; phys + PAGE2M_SIZE <= FIRST2G; phys += PAGE2M_SIZE) {
        addToTable(pml4, KERNEL_VIRT(phys), phys);
        addToTable(pml4, VA(phys), phys);
    }
    
    for (; phys + PAGE2M_SIZE <= MAX_PHYS_ADDR; phys += PAGE2M_SIZE) {
        addToTable(pml4, VA(phys), phys);
    }

    printf("cr3 is setting up...\n");
    
    store_pml4((phys_t)pml4);

    printf("Paging setted up\n");
}
