#ifndef __PIC_H__
#define __PIC_H__

#include "ioport.h"

#define MasterCmdStsR      0x20         // port to access to Command & Status Register Master
#define MasterIntrMskDataR 0x21         // port to access to Interrupt Mask Register & Data RegisterMaster

#define SlaveCmdStsR      0xA0          // port to access to Command & Status Register Slave
#define SlaveIntrMskDataR 0xA1          // port to access to Interrupt Mask Register & Data Register Slave

#define EOI               0x20

static inline void initPIC()
{
    out8(MasterCmdStsR,      0x11);         // tells that command consists of 4 words
    out8(MasterIntrMskDataR, 0x20);         // discribes mapping: IRQ + 0x20
    out8(MasterIntrMskDataR, 1 << 2);       // slave is connected to 2
    out8(MasterIntrMskDataR, 1);

    out8(SlaveCmdStsR, 	    0x11);          // tells that command consists of 4 words
    out8(SlaveIntrMskDataR, 0x28);          // discribes mapping: IRQ + 0x28
    out8(SlaveIntrMskDataR, 2);             // slave is connected to 2
    out8(SlaveIntrMskDataR, 1);
}

#endif /* __PIC_H__ */