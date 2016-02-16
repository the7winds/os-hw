#include "UART.h"

void UARTputchar(char c)
{
	while (1) 
    {
        char status = in8(UARTLnStsR);
        
        if (status & (1 << 5)) 
        {
            out8(UARTDataR, c);
            break;
        }
    }
}

void UARTputstr(char* str)
{
	for (int i = 0; str[i] != 0; ++i)
	{
		UARTputchar(str[i]);
	}
}

void HelloPrint()
{
    UARTputstr("Hello!\n");
}