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

    //

    if (initMMAPInfo() == 0) {
        printf("intited initMMAPInfo\n");
        printMMAP();
        if (reserveKernelMemory() == 0) {
            printf("kernel memory is reserved\n");
            // printMMAP();
            if (initBuddyAllocator() == 0) {
                printf("buddy allocator inited\n");
                // printMMAP();
                // printOrders();
                /*while (1) {
                    uint64_t a = (uint64_t) buddyAlloc(0);
                    printf("addr: %llx - %s\n", a, (a >= ((uint64_t) 1 << 32) ? "BAD" : "OK"));
                    for (int i = 0; i < 10000000; ++i);
                }*/
                // printMMAP();
                // printOrders();
                setUpPaging();
                // initFixedAllocator();
            } else {
                printf("can't initBuddyAllocator\n");
            }
        } else {
            printf("can't reserveKernelMemory\n");
        }
    } else {
        printf("can't initMMAPInfo\n");
    }
    printf("----END----\n");
    
    while (1);
}
