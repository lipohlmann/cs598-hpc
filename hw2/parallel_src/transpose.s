	.file	"transpose.c"
	.text
	.p2align 4
	.globl	transpose_naive
	.type	transpose_naive, @function
transpose_naive:
.LFB0:
	.cfi_startproc
	movl	%esi, %r9d
	testl	%esi, %esi
	jle	.L6
	testl	%edi, %edi
	jle	.L6
	leal	-1(%rdi), %eax
	movslq	%edi, %r11
	movslq	%esi, %rsi
	movq	$-8, %r10
	salq	$3, %rax
	salq	$3, %r11
	salq	$3, %rsi
	movq	%rcx, %r8
	xorl	%edi, %edi
	leaq	8(%rdx,%rax), %rcx
	subq	%rax, %r10
	.p2align 4
	.p2align 3
.L3:
	leaq	(%r10,%rcx), %rax
	movq	%r8, %rdx
	.p2align 4
	.p2align 3
.L4:
	vmovsd	(%rax), %xmm0
	addq	$8, %rax
	vmovsd	%xmm0, (%rdx)
	addq	%rsi, %rdx
	cmpq	%rcx, %rax
	jne	.L4
	incl	%edi
	addq	$8, %r8
	addq	%r11, %rcx
	cmpl	%edi, %r9d
	jne	.L3
.L6:
	ret
	.cfi_endproc
.LFE0:
	.size	transpose_naive, .-transpose_naive
	.p2align 4
	.globl	transpose_blocked
	.type	transpose_blocked, @function
transpose_blocked:
.LFB1:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	%esi, -4(%rsp)
	movq	%rcx, -24(%rsp)
	testl	%esi, %esi
	jle	.L20
	movl	%edi, %ebp
	testl	%edi, %edi
	jle	.L20
	leal	0(,%rsi,8), %r14d
	movslq	%ebp, %r11
	movslq	%esi, %rsi
	movq	%rdx, %rax
	leal	0(,%rdi,8), %edi
	addq	$8, %rax
	xorl	%r13d, %r13d
	movl	$0, -8(%rsp)
	salq	$3, %r11
	movq	%rax, -16(%rsp)
	vmovd	%edi, %xmm3
	salq	$3, %rsi
.L15:
	movl	%r13d, -28(%rsp)
	movl	-4(%rsp), %eax
	leal	8(%r13), %r10d
	cmpl	%eax, %r10d
	cmovg	%eax, %r10d
	cmpl	%r13d, %r10d
	jle	.L10
	movslq	-8(%rsp), %r15
	xorl	%r12d, %r12d
	xorl	%ebx, %ebx
	.p2align 4
	.p2align 3
.L14:
	leal	8(%rbx), %eax
	cmpl	%ebp, %eax
	cmovg	%ebp, %eax
	cmpl	%ebx, %eax
	jle	.L11
	movq	-24(%rsp), %rdi
	movslq	%r12d, %rdx
	subl	%ebx, %eax
	addq	%r13, %rdx
	leal	-1(%rax), %r9d
	leaq	(%r15,%rbx), %rax
	addq	%r9, %rax
	notq	%r9
	salq	$3, %r9
	leaq	(%rdi,%rdx,8), %r8
	movq	-16(%rsp), %rdi
	leaq	(%rdi,%rax,8), %rcx
	movl	-28(%rsp), %edi
	.p2align 4
	.p2align 3
.L12:
	leaq	(%r9,%rcx), %rax
	movq	%r8, %rdx
	.p2align 4
	.p2align 3
.L13:
	vmovsd	(%rax), %xmm0
	addq	$8, %rax
	vmovsd	%xmm0, (%rdx)
	addq	%rsi, %rdx
	cmpq	%rcx, %rax
	jne	.L13
	incl	%edi
	addq	$8, %r8
	addq	%r11, %rcx
	cmpl	%edi, %r10d
	jne	.L12
.L11:
	addq	$8, %rbx
	addl	%r14d, %r12d
	cmpl	%ebx, %ebp
	jg	.L14
.L10:
	vmovd	%xmm3, %ebx
	addq	$8, %r13
	addl	%ebx, -8(%rsp)
	cmpl	%r13d, -4(%rsp)
	jg	.L15
.L20:
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1:
	.size	transpose_blocked, .-transpose_blocked
	.p2align 4
	.globl	transpose_real
	.type	transpose_real, @function
transpose_real:
.LFB2:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	%esi, -12(%rsp)
	movq	%rdx, -8(%rsp)
	movq	%rcx, -24(%rsp)
	testl	%esi, %esi
	jle	.L34
	movl	%edi, %r9d
	testl	%edi, %edi
	jle	.L34
	leal	0(,%rdi,8), %ebx
	movslq	%edi, %r12
	movslq	%esi, %r14
	xorl	%r13d, %r13d
	vmovd	%ebx, %xmm3
	leal	0(,%rsi,8), %ebx
	movl	$0, -16(%rsp)
	salq	$3, %r12
	vmovd	%ebx, %xmm2
	salq	$3, %r14
.L29:
	movl	%r13d, -28(%rsp)
	movl	-12(%rsp), %eax
	leal	8(%r13), %ebp
	cmpl	%eax, %ebp
	cmovg	%eax, %ebp
	cmpl	%r13d, %ebp
	jle	.L24
	movslq	-16(%rsp), %rax
	movq	-8(%rsp), %rbx
	xorl	%r10d, %r10d
	xorl	%edx, %edx
	vmovd	%xmm2, %r15d
	movq	%r13, -40(%rsp)
	leaq	(%rbx,%rax,8), %r11
	.p2align 4
	.p2align 3
.L28:
	leal	8(%rdx), %eax
	cmpl	%r9d, %eax
	cmovg	%r9d, %eax
	cmpl	%edx, %eax
	jle	.L25
	movq	-24(%rsp), %rbx
	movslq	%r10d, %rcx
	addq	-40(%rsp), %rcx
	movl	-28(%rsp), %esi
	subl	%edx, %eax
	leaq	(%rbx,%rcx,8), %rdi
	leal	-1(%rax), %ecx
	movq	%r11, %rax
	leaq	8(,%rcx,8), %r13
	.p2align 4
	.p2align 3
.L26:
	leaq	0(%r13,%rax), %rbx
	movq	%rdi, %r8
	movq	%rax, %rcx
	.p2align 4
	.p2align 3
.L27:
	vmovsd	(%rcx), %xmm1
	addq	$8, %rcx
	vmovsd	%xmm1, (%r8)
	addq	%r14, %r8
	cmpq	%rbx, %rcx
	jne	.L27
	incl	%esi
	addq	%r12, %rax
	addq	$8, %rdi
	cmpl	%esi, %ebp
	jne	.L26
.L25:
	addq	$8, %rdx
	addq	$64, %r11
	addl	%r15d, %r10d
	cmpl	%edx, %r9d
	jg	.L28
	movq	-40(%rsp), %r13
.L24:
	vmovd	%xmm3, %ebx
	addq	$8, %r13
	addl	%ebx, -16(%rsp)
	cmpl	%r13d, -12(%rsp)
	jg	.L29
.L34:
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE2:
	.size	transpose_real, .-transpose_real
	.ident	"GCC: (GNU) 11.5.0 20240719 (Red Hat 11.5.0-14)"
	.section	.note.GNU-stack,"",@progbits
