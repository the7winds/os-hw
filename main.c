#include <stdint.h>

#include "interrupt.h"
#include "ioport.h"
#include "memory.h"

#include "UART.h"
#include "PIC.h"
#include "PIT.h"
#include "IDT.h"

#include "utils.h"
#include "mem_info.h"
#include "buddy_alloc.h"
#include "fixed_alloc.h"
#include "paging.h"

struct idt_dscrpt idt[IDT_SIZE];

void test();

void main(void)
{
    initUART();
    initPIC();
    initPIT(0xFFFF);
    initIDT();

    extern void* pic_handler_start;
    uint64_t pic_handler = (uint64_t) &pic_handler_start;

    // add handler info to idt

    idt[0x20].offset_0_15 = pic_handler;
    idt[0x20].offset_16_31 = pic_handler >> 16;
    idt[0x20].offset_32_63 = pic_handler >> 32;
    idt[0x20].zero = 0x0;
    idt[0x20].type = (1 << 7) | 14;
    idt[0x20].seg_selector = KERNEL_CODE;
    idt[0x20].reserved = 0x0;

    // set up interrups flag
    // __asm__ volatile("sti");

    if (initMMAPInfo() == 0) {
        printf("intited initMMAPInfo\n");
        printMMAP();
        if (reserveKernelMemory() == 0) {
            printf("kernel memory is reserved\n");
            printMMAP();
            if (initBuddyAllocator() == 0) {
                printf("buddy allocator inited\n");
                setUpPaging();
                printOrders();
                printMMAP();
                if (initFixedAllocator() == 0) {
                    printf("fixed allocator inited!\n");
                    test();
                } else {
                    printf("can't init fixed allocator\n");
                }
            } else {
                printf("can't init buddy allocator\n");
            }
        } else {
            printf("can't reserve memory for kernel\n");
        }
    } else {
        printf("can't initMMAPInfo\n");
    }
    printf("----END----\n");
    
    while (1);
}

void test() {
    printf("buddy allocator test\n");

    char* ptr = buddyVAlloc(1);
    *ptr = 1;
    buddyVFree(ptr, 1);

    printf("fixed allocator test (small object)\n");

    FixedAllocator* fixedAllocator = newFixedAllocator(20, 4);

    ptr = fixedAllocate(fixedAllocator);
    *ptr = 1;
    fixedFree(ptr);

    deleteFixedAllocator(fixedAllocator);

    printf("fixed allocator test (big object)\n");

    fixedAllocator = newFixedAllocator(PAGE_SIZE / 4, 4);

    ptr = fixedAllocate(fixedAllocator);
    *ptr = 1;
    fixedFree(ptr);

    deleteFixedAllocator(fixedAllocator);
}