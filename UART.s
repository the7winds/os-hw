	.file	"UART.c"
	.text
	.type	out8, @function
out8:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %edx
	movl	%esi, %eax
	movw	%dx, -4(%rbp)
	movb	%al, -8(%rbp)
	movzbl	-8(%rbp), %eax
	movzwl	-4(%rbp), %edx
#APP
# 7 "ioport.h" 1
	outb %al, %dx
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	out8, .-out8
	.type	in8, @function
in8:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %eax
	movw	%ax, -20(%rbp)
	movzwl	-20(%rbp), %eax
	movl	%eax, %edx
#APP
# 13 "ioport.h" 1
	inb %dx, %al
# 0 "" 2
#NO_APP
	movb	%al, -1(%rbp)
	movzbl	-1(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	in8, .-in8
	.globl	UARTputchar
	.type	UARTputchar, @function
UARTputchar:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movl	%edi, %eax
	movb	%al, -20(%rbp)
.L7:
	movl	$1021, %edi
	call	in8
	movb	%al, -1(%rbp)
	movsbl	-1(%rbp), %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	.L5
	movzbl	-20(%rbp), %eax
	movzbl	%al, %eax
	movl	%eax, %esi
	movl	$1016, %edi
	call	out8
	jmp	.L8
.L5:
	jmp	.L7
.L8:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	UARTputchar, .-UARTputchar
	.globl	UARTputstr
	.type	UARTputstr, @function
UARTputstr:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movq	%rdi, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L10
.L11:
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	UARTputchar
	addl	$1, -4(%rbp)
.L10:
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	-24(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	jne	.L11
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	UARTputstr, .-UARTputstr
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.1) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
