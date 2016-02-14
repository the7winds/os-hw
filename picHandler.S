#define UARTbegin    0x3f8
#define UARTDataR    UARTbegin + 0
#define UARTLnStsR   UARTbegin + 5

#define MasterCmdStsR  0x20

.global pic_handler_start

.text

pic_handler_start:
    // save registers info
    push %rax
    push %rbx
    push %rdx

    movb '!', %al
    push %ax
    call UARTputchar
    pop %ax

    // send EOI to master on IRQ0 - 0b00010000
    movw MasterCmdStsR, %dx
    movb 0x10, %al
    outb %al, %dx  

    // restore registers
    pop %rdx
    pop %rbx
    pop %rax

    iretq