#ifndef __UART_H__
#define __UART_H__

#include "ioport.h"

#define UARTbegin    0x3f8
#define UARTDataR    UARTbegin + 0
#define UARTIntrEnR  UARTbegin + 1
#define UARTLnCntrlR UARTbegin + 3
#define UARTLnStsR   UARTbegin + 5      	// if 5th bit is 1 then we can write next data else wait

static inline void initUART()
{
    // setting speed: 600 BPS
    out8(UARTLnCntrlR, 0x8B);               // divisor latch
    out8(UARTDataR,    0x00);               // lower part of divisor
    out8(UARTLnCntrlR, 0xC0);               // higher part of divisor

    // setting 8bit frame, check oddness + no interrupts
    out8(UARTLnCntrlR, 0x0B);                 
    out8(UARTIntrEnR, 0);                   // don't gen interruptions

    // test output
    char *s = "UART test\n";
    for (int i = 0; s[i] != 0; ++i)
    {
        while (1) 
        {
            char status = in8(UARTLnStsR);
            
            if (status & (1 << 5)) 
            {
                out8(UARTDataR, s[i]);
                break;
            }
        }
    }
}

#endif /* __UART_H__ */