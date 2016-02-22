#include <stdint.h>

#include "interrupt.h"
#include "ioport.h"
#include "memory.h"

#include "UART.h"
#include "PIC.h"
#include "PIT.h"
#include "IDT.h"

#include "utils.h"

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
    __asm__ volatile("sti");

    while (1);
}