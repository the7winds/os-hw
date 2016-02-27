#ifndef __PIT_H__
#define __PIT_H__

#include "ioport.h"

#define PITCntrl 0x43
#define PITData  0x40

static inline void initPIT(uint16_t divisor)
{
    out8(PITCntrl, 0x34);
    out8(PITData, divisor);
    out8(PITData, divisor >> 8);
}

#endif /* __PIT_H__ */