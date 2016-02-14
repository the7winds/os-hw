#include <stdint.h>
#include "interrupt.h"
#include "ioport.h"

#include "UART.h"
#include "PIC.h"
#include "PIT.h"

void main(void)
{
    initUART();
    initPIC();
    initPIT(0xFFFF);

    while (1);
}
