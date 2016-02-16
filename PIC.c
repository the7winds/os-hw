#include "ioport.h"
#include "PIC.h"
#include "UART.h"

void sendEOI()
{
    out8(MasterCmdStsR, EOI);
}

void HelloPrint()
{
    UARTputstr("Hello!\n");
}