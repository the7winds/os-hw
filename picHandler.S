#define UARTbegin    0x3f8
#define UARTDataR    UARTbegin + 0
#define UARTLnStsR   UARTbegin + 5


.global pic_handler_start

format:
    .asciz "Interrupt has been occurred: %d\n"

.text

pic_handler_start:
    // save registers info
    push %rax
    push %rbx
    push %rdx
    push %rcx
    push %rsp
    push %rbp
    push %rsi
    push %rdi
    pushfq

    movq $format, %rdi
    movq $33, %rsi
    call printf
    call sendEOI

    // restore registers
    popfq
    pop %rdi
    pop %rsi
    pop %rbp
    pop %rsp
    pop %rcx
    pop %rdx
    pop %rbx
    pop %rax

    iretq