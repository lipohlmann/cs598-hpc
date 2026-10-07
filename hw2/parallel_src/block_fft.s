	.file	"block_fft.c"
	.text
	.p2align 4
	.globl	block_fft_plan_free
	.type	block_fft_plan_free, @function
block_fft_plan_free:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movq	%rdi, %rbx
	subq	$8, %rsp
	movq	16(%rdi), %rdi
	call	free
	movq	24(%rbx), %rdi
	call	free
	movq	32(%rbx), %rdi
	call	free
	movq	40(%rbx), %rdi
	call	free
	vpxor	%xmm0, %xmm0, %xmm0
	vmovdqu	%ymm0, 16(%rbx)
	vzeroupper
	movq	-8(%rbp), %rbx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	block_fft_plan_free, .-block_fft_plan_free
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"block_fft: radix must be 4\n"
.LC1:
	.string	"block_fft: require n=4^k\n"
	.text
	.p2align 4
	.globl	block_fft_plan_init
	.type	block_fft_plan_init, @function
block_fft_plan_init:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	vmovd	%esi, %xmm7
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	vpinsrd	$1, %edx, %xmm7, %xmm0
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r10
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 10, -56
	movq	%rdi, %r10
	pushq	%rbx
	subq	$96, %rsp
	.cfi_offset 3, -64
	vmovq	%xmm0, (%rdi)
	movl	%esi, -132(%rbp)
	movl	%ecx, -124(%rbp)
	movl	%ecx, 8(%rdi)
	cmpl	$1, %esi
	jle	.L23
	movl	%esi, %edx
	xorl	%ebx, %ebx
	movl	$1, %eax
	.p2align 4
	.p2align 3
.L6:
	imull	%ecx, %eax
	incl	%ebx
	cmpl	%eax, %edx
	jg	.L6
.L5:
	cmpl	%eax, -132(%rbp)
	jne	.L37
	cmpl	$4, -124(%rbp)
	vpxor	%xmm0, %xmm0, %xmm0
	movl	%ebx, 12(%r10)
	vmovdqu	%ymm0, 16(%r10)
	jne	.L10
	movslq	-132(%rbp), %r9
	movq	%r10, -72(%rbp)
	movq	%r9, %r15
	leaq	0(,%r9,4), %rdi
	movq	%r9, -88(%rbp)
	vzeroupper
	call	malloc
	movq	-72(%rbp), %r10
	movq	%rax, -80(%rbp)
	movq	%rax, 16(%r10)
	leal	-1(%r15), %eax
	movslq	%eax, %r14
	imulq	$1431655766, %r14, %r14
	sarl	$31, %eax
	shrq	$32, %r14
	subl	%eax, %r14d
	movslq	%r14d, %r14
	salq	$4, %r14
	movq	%r14, %rdi
	call	malloc
	movq	-72(%rbp), %r10
	movq	%r14, %rdi
	movq	%rax, %r12
	movq	%rax, -112(%rbp)
	movq	%rax, 24(%r10)
	call	malloc
	movq	-72(%rbp), %r10
	movq	%r14, %rdi
	movq	%rax, %r13
	movq	%rax, 32(%r10)
	call	malloc
	movq	-72(%rbp), %r10
	movq	-80(%rbp), %r8
	testq	%r12, %r12
	movq	%rax, %r14
	movq	%rax, 40(%r10)
	sete	%al
	testq	%r8, %r8
	sete	%dl
	orb	%dl, %al
	jne	.L12
	testq	%r13, %r13
	sete	%al
	testq	%r14, %r14
	sete	%dl
	orb	%dl, %al
	jne	.L12
	xorl	%edi, %edi
	testl	%r15d, %r15d
	movq	-88(%rbp), %r9
	jle	.L19
	.p2align 4
	.p2align 3
.L18:
	movl	%edi, %eax
	testl	%ebx, %ebx
	je	.L25
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	.p2align 4
	.p2align 3
.L17:
	movl	%eax, %esi
	incl	%edx
	andl	$3, %esi
	sarl	$2, %eax
	leal	(%rsi,%rcx,4), %ecx
	cmpl	%ebx, %edx
	jne	.L17
	movl	%ecx, (%r8,%rdi,4)
	incq	%rdi
	cmpq	%r9, %rdi
	jne	.L18
.L19:
	cmpl	$3, -132(%rbp)
	movl	$0, -128(%rbp)
	jle	.L15
	.p2align 4
	.p2align 3
.L14:
	movl	-124(%rbp), %eax
	vxorpd	%xmm7, %xmm7, %xmm7
	movq	-112(%rbp), %rcx
	movl	%eax, %ebx
	vcvtsi2sdl	%eax, %xmm7, %xmm0
	movslq	-128(%rbp), %rax
	sarl	$2, %ebx
	salq	$4, %rax
	leaq	(%rcx,%rax), %rdx
	movq	.LC2(%rip), %rcx
	movq	$0x000000000, 8(%rdx)
	movq	%rcx, (%rdx)
	leaq	0(%r13,%rax), %rdx
	movq	%rcx, (%rdx)
	movq	$0x000000000, 8(%rdx)
	leaq	(%r14,%rax), %rdx
	movq	%rcx, (%rdx)
	movq	$0x000000000, 8(%rdx)
	cmpl	$1, %ebx
	je	.L22
	leaq	16(%rax), %r12
	movl	$1, %r15d
	vmovsd	.LC4(%rip), %xmm7
	vdivsd	%xmm0, %xmm7, %xmm7
	movq	%r12, %rax
	movl	%r15d, %r12d
	vmovsd	%xmm7, -120(%rbp)
	movq	%rax, %r15
	.p2align 4
	.p2align 3
.L21:
	leaq	-64(%rbp), %rsi
	leaq	-56(%rbp), %rdi
	vxorpd	%xmm7, %xmm7, %xmm7
	vcvtsi2sdl	%r12d, %xmm7, %xmm1
	vmulsd	-120(%rbp), %xmm1, %xmm1
	vmovsd	%xmm1, %xmm1, %xmm0
	vmovsd	%xmm1, -72(%rbp)
	call	sincos
	leaq	-64(%rbp), %rsi
	leaq	-56(%rbp), %rdi
	vmovsd	-64(%rbp), %xmm5
	vmovsd	-56(%rbp), %xmm4
	vmovsd	-72(%rbp), %xmm1
	vmovsd	%xmm5, -104(%rbp)
	vmovsd	%xmm4, -96(%rbp)
	vaddsd	%xmm1, %xmm1, %xmm0
	vmovsd	%xmm1, -88(%rbp)
	call	sincos
	leaq	-64(%rbp), %rsi
	leaq	-56(%rbp), %rdi
	vmovsd	-64(%rbp), %xmm3
	vmovsd	-56(%rbp), %xmm2
	vmovsd	%xmm3, -80(%rbp)
	vmovsd	%xmm2, -72(%rbp)
	vmovsd	-88(%rbp), %xmm1
	vmulsd	.LC5(%rip), %xmm1, %xmm0
	call	sincos
	movq	-112(%rbp), %rax
	vmovsd	-96(%rbp), %xmm4
	vmovsd	-104(%rbp), %xmm5
	vunpcklpd	%xmm4, %xmm5, %xmm0
	vmovsd	-72(%rbp), %xmm2
	vmovsd	-80(%rbp), %xmm3
	incl	%r12d
	vmovupd	%xmm0, (%rax,%r15)
	vunpcklpd	%xmm2, %xmm3, %xmm0
	vmovsd	-64(%rbp), %xmm6
	vmovupd	%xmm0, 0(%r13,%r15)
	vmovhpd	-56(%rbp), %xmm6, %xmm0
	vmovupd	%xmm0, (%r14,%r15)
	addq	$16, %r15
	cmpl	%r12d, %ebx
	jne	.L21
.L22:
	sall	$2, -124(%rbp)
	movl	-124(%rbp), %eax
	addl	%ebx, -128(%rbp)
	cmpl	%eax, -132(%rbp)
	jge	.L14
.L15:
	xorl	%eax, %eax
.L4:
	addq	$96, %rsp
	popq	%rbx
	popq	%r10
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4
	.p2align 3
.L25:
	.cfi_restore_state
	xorl	%ecx, %ecx
	movl	%ecx, (%r8,%rdi,4)
	incq	%rdi
	cmpq	%r9, %rdi
	jne	.L18
	jmp	.L19
.L10:
	movq	stderr(%rip), %rcx
	movl	$27, %edx
	movl	$1, %esi
	movl	$.LC0, %edi
	vzeroupper
	call	fwrite
	movl	$1, %eax
	jmp	.L4
.L23:
	xorl	%ebx, %ebx
	movl	$1, %eax
	jmp	.L5
.L37:
	cmpl	$4, -124(%rbp)
	vpxor	%xmm0, %xmm0, %xmm0
	movq	stderr(%rip), %rcx
	vmovdqu	%ymm0, 16(%r10)
	movl	$-1, 12(%r10)
	jne	.L10
	movl	$25, %edx
	movl	$1, %esi
	movl	$.LC1, %edi
	vzeroupper
	call	fwrite
	movl	$2, %eax
	jmp	.L4
.L12:
	movq	%r10, %rdi
	call	block_fft_plan_free
	movl	$3, %eax
	jmp	.L4
	.cfi_endproc
.LFE10:
	.size	block_fft_plan_init, .-block_fft_plan_init
	.p2align 4
	.globl	block_fft_forward
	.type	block_fft_forward, @function
block_fft_forward:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	movq	%rsi, %r15
	andq	$-32, %rsp
	subq	$168, %rsp
	movq	%rdi, -88(%rsp)
	movl	(%rdi), %esi
	movl	4(%rdi), %eax
	movl	%esi, 16(%rsp)
	movl	%eax, 108(%rsp)
	testl	%esi, %esi
	jle	.L85
	movl	%eax, %r13d
	movq	-88(%rsp), %rax
	movslq	16(%rsp), %r10
	xorl	%r8d, %r8d
	leal	-1(%r13), %r9d
	xorl	%edi, %edi
	movl	%r9d, 68(%rsp)
	leaq	16(%r15), %rbx
	salq	$4, %r9
	movq	16(%rax), %r12
	jmp	.L43
.L45:
	incq	%rdi
	addl	%r13d, %r8d
	cmpq	%r10, %rdi
	je	.L87
.L43:
	movl	(%r12,%rdi,4), %eax
	cmpl	%edi, %eax
	jle	.L45
	imull	%r13d, %eax
	movslq	%r8d, %rsi
	salq	$4, %rsi
	movslq	%eax, %rdx
	leaq	(%rsi,%r9), %r11
	salq	$4, %rdx
	testl	%r13d, %r13d
	jle	.L45
	leaq	(%r15,%rsi), %rax
	addq	%rbx, %r11
.L46:
	movq	%rax, %rcx
	vmovupd	(%rax), %xmm0
	addq	$16, %rax
	subq	%rsi, %rcx
	vmovdqu	(%rcx,%rdx), %xmm4
	vmovdqu	%xmm4, -16(%rax)
	vmovupd	%xmm0, (%rcx,%rdx)
	cmpq	%r11, %rax
	jne	.L46
	incq	%rdi
	addl	%r13d, %r8d
	cmpq	%r10, %rdi
	jne	.L43
.L87:
	cmpl	$3, 16(%rsp)
	jle	.L85
	movl	108(%rsp), %esi
	vmovdqa64	.LC6(%rip), %ymm28
	vmovdqa64	.LC7(%rip), %ymm20
	movl	$0, -48(%rsp)
	vmovdqa64	.LC8(%rip), %ymm19
	vmovdqa64	.LC9(%rip), %ymm31
	movl	$4, 20(%rsp)
	leal	(%rsi,%rsi), %eax
	cltq
	salq	$4, %rax
	movq	%rax, -32(%rsp)
	movl	68(%rsp), %eax
	movq	%rax, 40(%rsp)
	movl	%esi, %eax
	andl	$-4, %esi
	shrl	$2, %eax
	movl	%esi, 64(%rsp)
	decl	%eax
	movq	%rax, -96(%rsp)
.L64:
	movl	20(%rsp), %eax
	movl	108(%rsp), %edx
	movl	%eax, %ecx
	movl	%eax, %esi
	sarl	%esi
	sarl	$2, %ecx
	movl	%esi, 12(%rsp)
	movl	%esi, %ebx
	addl	%ecx, %esi
	movl	%ecx, 104(%rsp)
	movl	%esi, 8(%rsp)
	testl	%edx, %edx
	jle	.L47
	imull	%edx, %eax
	movl	$0, 28(%rsp)
	movl	$0, 48(%rsp)
	imull	%edx, %ebx
	imull	%edx, %esi
	movl	%eax, 4(%rsp)
	movl	%edx, %eax
	imull	%ecx, %eax
	movl	%ebx, (%rsp)
	movl	%esi, -36(%rsp)
	movl	%eax, 24(%rsp)
	movq	-88(%rsp), %rax
	movq	24(%rax), %rdi
	movq	32(%rax), %r14
	movq	40(%rax), %r9
	movslq	-48(%rsp), %rax
	movq	%rdi, -56(%rsp)
	movq	%r14, -64(%rsp)
	movq	%r9, -72(%rsp)
	movq	%rax, -80(%rsp)
	salq	$4, %rax
	addq	%rax, %rdi
	movq	%rdi, -8(%rsp)
	leaq	(%r14,%rax), %rdi
	addq	%r9, %rax
	movq	%rax, -24(%rsp)
	leal	1(%rcx), %eax
	movq	%rdi, -16(%rsp)
	imull	%edx, %eax
	movl	%eax, -40(%rsp)
	movslq	%edx, %rax
	movq	%rax, 88(%rsp)
	leal	-1(%rcx), %eax
	movl	%eax, -44(%rsp)
	movq	-96(%rsp), %rax
	incq	%rax
	salq	$6, %rax
	movq	%rax, 56(%rsp)
	.p2align 4
	.p2align 3
.L48:
	cmpl	$2, 104(%rsp)
	jle	.L88
	movslq	24(%rsp), %rdx
	movslq	28(%rsp), %rax
	movq	-24(%rsp), %r12
	movq	-16(%rsp), %r13
	movq	-8(%rsp), %r14
	salq	$4, %rdx
	movq	%rdx, 128(%rsp)
	movl	(%rsp), %edx
	movq	%rax, %rsi
	movq	%rax, %rbx
	addl	-40(%rsp), %esi
	salq	$4, %rbx
	addl	%eax, %edx
	movq	%rbx, 136(%rsp)
	movl	$1, %ebx
	movslq	%edx, %rdx
	salq	$4, %rdx
	movq	%rdx, 120(%rsp)
	movl	-36(%rsp), %edx
	addl	%eax, %edx
	movslq	%edx, %rdx
	salq	$4, %rdx
	movq	%rdx, 112(%rsp)
	movl	108(%rsp), %edx
	addl	%eax, %edx
	addq	40(%rsp), %rax
	movslq	%edx, %rdx
	salq	$4, %rdx
	movq	%rdx, 96(%rsp)
	movslq	%esi, %rdx
	movl	12(%rsp), %esi
	salq	$4, %rax
	leaq	16(%r15,%rax), %r11
	movl	48(%rsp), %eax
	salq	$4, %rdx
	movq	%rdx, 80(%rsp)
	addl	%eax, %esi
	movl	%esi, 72(%rsp)
	movl	8(%rsp), %esi
	addl	%esi, %eax
	movl	%eax, 52(%rsp)
.L50:
	movq	128(%rsp), %rsi
	movl	72(%rsp), %ecx
	movl	52(%rsp), %edx
	vmovsd	24(%r12), %xmm4
	movq	136(%rsp), %rax
	vmovsd	%xmm4, 144(%rsp)
	vmovsd	(%r14), %xmm15
	vmovsd	24(%r13), %xmm4
	vmovsd	8(%r14), %xmm21
	vmovsd	%xmm4, 152(%rsp)
	vmovsd	0(%r13), %xmm14
	vmovsd	24(%r14), %xmm4
	vmovsd	8(%r13), %xmm18
	vmovsd	(%r12), %xmm13
	vmovsd	8(%r12), %xmm17
	vmovsd	16(%r12), %xmm12
	vmovsd	16(%r13), %xmm11
	vmovsd	%xmm4, 160(%rsp)
	vmovsd	16(%r14), %xmm10
	leaq	(%r15,%rsi), %r10
	movq	120(%rsp), %rsi
	addl	%ebx, %ecx
	addl	%ebx, %edx
	imull	108(%rsp), %ecx
	addq	%r15, %rax
	imull	108(%rsp), %edx
	leaq	(%r15,%rsi), %r9
	movq	112(%rsp), %rsi
	movslq	%ecx, %rcx
	movslq	%edx, %rdx
	leaq	(%r15,%rsi), %r8
	movq	96(%rsp), %rsi
	salq	$4, %rcx
	salq	$4, %rdx
	leaq	(%r15,%rsi), %rdi
	movq	80(%rsp), %rsi
	addq	%r15, %rcx
	addq	%r15, %rdx
	addq	%r15, %rsi
	.p2align 4
	.p2align 3
.L49:
	vmovsd	(%r10), %xmm1
	vmovsd	(%r9), %xmm2
	vmovsd	8(%r10), %xmm6
	vmovsd	8(%r9), %xmm3
	vmovsd	8(%r8), %xmm4
	vmulsd	%xmm6, %xmm21, %xmm25
	vmulsd	%xmm3, %xmm18, %xmm26
	vmulsd	%xmm4, %xmm17, %xmm23
	vmulsd	%xmm1, %xmm21, %xmm0
	vmovsd	%xmm15, %xmm15, %xmm22
	vfmadd132sd	%xmm15, %xmm0, %xmm6
	vfmsub132sd	%xmm1, %xmm25, %xmm22
	vmulsd	%xmm2, %xmm18, %xmm0
	vmovsd	%xmm14, %xmm14, %xmm7
	vmovsd	%xmm13, %xmm13, %xmm24
	vmovsd	(%rax), %xmm5
	vfmsub132sd	%xmm2, %xmm26, %xmm7
	vfmadd132sd	%xmm14, %xmm0, %xmm3
	vmovsd	(%r8), %xmm0
	vmulsd	%xmm0, %xmm17, %xmm30
	vmovsd	8(%rax), %xmm8
	addq	$16, %rax
	addq	$16, %r10
	addq	$16, %r9
	vfmsub132sd	%xmm0, %xmm23, %xmm24
	vfmadd132sd	%xmm13, %xmm30, %xmm4
	vmovsd	144(%rsp), %xmm30
	addq	$16, %r8
	addq	$16, %rdi
	addq	$16, %rsi
	addq	$16, %rcx
	addq	$16, %rdx
	vfnmadd132sd	%xmm14, %xmm26, %xmm2
	vfnmadd132sd	%xmm15, %xmm25, %xmm1
	vaddsd	%xmm3, %xmm6, %xmm27
	vaddsd	%xmm3, %xmm8, %xmm16
	vaddsd	%xmm7, %xmm22, %xmm9
	vsubsd	%xmm3, %xmm8, %xmm3
	vfnmadd132sd	%xmm13, %xmm23, %xmm0
	vaddsd	%xmm24, %xmm5, %xmm29
	vsubsd	%xmm4, %xmm5, %xmm26
	vaddsd	%xmm29, %xmm9, %xmm9
	vmovsd	%xmm9, -16(%rax)
	vaddsd	%xmm4, %xmm8, %xmm9
	vaddsd	%xmm27, %xmm9, %xmm9
	vmovsd	%xmm9, -8(%rax)
	vaddsd	%xmm7, %xmm1, %xmm7
	vaddsd	%xmm4, %xmm2, %xmm9
	vaddsd	%xmm24, %xmm1, %xmm1
	vaddsd	%xmm6, %xmm2, %xmm2
	vaddsd	%xmm3, %xmm1, %xmm1
	vaddsd	%xmm26, %xmm2, %xmm2
	vmovsd	%xmm1, -8(%r10)
	vmovsd	%xmm2, -16(%r10)
	vaddsd	%xmm5, %xmm0, %xmm1
	vaddsd	%xmm4, %xmm6, %xmm4
	vaddsd	%xmm7, %xmm1, %xmm1
	vsubsd	%xmm4, %xmm16, %xmm4
	vmovsd	%xmm1, -16(%r9)
	vmovsd	%xmm4, -8(%r9)
	vsubsd	%xmm6, %xmm5, %xmm5
	vmovsd	160(%rsp), %xmm4
	vaddsd	%xmm9, %xmm5, %xmm5
	vaddsd	%xmm22, %xmm0, %xmm0
	vmovsd	%xmm5, -16(%r8)
	vaddsd	%xmm3, %xmm0, %xmm0
	vmovsd	%xmm0, -8(%r8)
	vmovsd	-16(%rcx), %xmm2
	vmovsd	-16(%rsi), %xmm1
	vmovsd	-8(%rsi), %xmm6
	vmovsd	-8(%rcx), %xmm3
	vmulsd	%xmm4, %xmm6, %xmm25
	vmulsd	%xmm4, %xmm1, %xmm0
	vmovsd	%xmm1, %xmm1, %xmm22
	vfmadd132sd	%xmm10, %xmm0, %xmm6
	vmovsd	152(%rsp), %xmm4
	vfmsub132sd	%xmm10, %xmm25, %xmm22
	vmulsd	%xmm4, %xmm3, %xmm26
	vmulsd	%xmm4, %xmm2, %xmm0
	vmovsd	%xmm2, %xmm2, %xmm7
	vmovsd	-8(%rdx), %xmm4
	vmulsd	%xmm30, %xmm4, %xmm23
	vfmadd132sd	%xmm11, %xmm0, %xmm3
	vfmsub132sd	%xmm11, %xmm26, %xmm7
	vmovsd	-16(%rdx), %xmm0
	vmovsd	%xmm0, %xmm0, %xmm24
	vmulsd	%xmm30, %xmm0, %xmm30
	vmovsd	-16(%rdi), %xmm5
	vmovsd	-8(%rdi), %xmm8
	vfmadd132sd	%xmm12, %xmm30, %xmm4
	vfnmadd132sd	%xmm11, %xmm26, %xmm2
	vfmsub132sd	%xmm12, %xmm23, %xmm24
	vfnmadd132sd	%xmm10, %xmm25, %xmm1
	vaddsd	%xmm22, %xmm7, %xmm9
	vaddsd	%xmm6, %xmm3, %xmm27
	vaddsd	%xmm8, %xmm3, %xmm16
	vsubsd	%xmm3, %xmm8, %xmm3
	vfnmadd132sd	%xmm12, %xmm23, %xmm0
	vsubsd	%xmm4, %xmm5, %xmm26
	vaddsd	%xmm5, %xmm24, %xmm29
	vaddsd	%xmm7, %xmm1, %xmm7
	vaddsd	%xmm29, %xmm9, %xmm9
	vaddsd	%xmm24, %xmm1, %xmm1
	vmovsd	%xmm9, -16(%rdi)
	vaddsd	%xmm3, %xmm1, %xmm1
	vaddsd	%xmm8, %xmm4, %xmm9
	vaddsd	%xmm27, %xmm9, %xmm9
	vmovsd	%xmm9, -8(%rdi)
	vmovsd	%xmm1, -8(%rsi)
	vaddsd	%xmm4, %xmm2, %xmm9
	vaddsd	%xmm5, %xmm0, %xmm1
	vaddsd	%xmm6, %xmm2, %xmm2
	vaddsd	%xmm7, %xmm1, %xmm1
	vaddsd	%xmm26, %xmm2, %xmm2
	vaddsd	%xmm6, %xmm4, %xmm4
	vmovsd	%xmm2, -16(%rsi)
	vsubsd	%xmm4, %xmm16, %xmm4
	vmovsd	%xmm1, -16(%rcx)
	vsubsd	%xmm6, %xmm5, %xmm5
	vmovsd	%xmm4, -8(%rcx)
	vaddsd	%xmm9, %xmm5, %xmm5
	vaddsd	%xmm22, %xmm0, %xmm0
	vmovsd	%xmm5, -16(%rdx)
	vaddsd	%xmm3, %xmm0, %xmm0
	vmovsd	%xmm0, -8(%rdx)
	cmpq	%r11, %rax
	jne	.L49
	movq	-32(%rsp), %rsi
	leal	1(%rbx), %eax
	addq	$32, %r14
	addl	$2, %ebx
	addq	$32, %r13
	addq	$32, %r12
	addq	%rsi, 136(%rsp)
	addq	%rsi, 128(%rsp)
	addq	%rsi, 120(%rsp)
	addq	%rsi, 112(%rsp)
	addq	%rsi, 96(%rsp)
	addq	%rsi, 80(%rsp)
	addq	%rsi, %r11
	cmpl	%ebx, -44(%rsp)
	jg	.L50
	movl	%eax, 112(%rsp)
.L63:
	movslq	112(%rsp), %rax
	movq	-56(%rsp), %rbx
	movq	%rax, %rsi
	addq	-80(%rsp), %rax
	salq	$4, %rax
	leaq	(%rbx,%rax), %r10
	movq	-64(%rsp), %rbx
	movq	%r10, %r13
	leaq	(%rbx,%rax), %r14
	addq	-72(%rsp), %rax
	movl	104(%rsp), %ebx
	movq	%rax, 96(%rsp)
	movl	48(%rsp), %eax
	addl	%esi, %eax
	movl	108(%rsp), %esi
	leal	(%rbx,%rax), %edx
	imull	%esi, %edx
	movl	%esi, %r9d
	imull	%eax, %r9d
	movslq	%edx, %rbx
	movq	%rbx, 136(%rsp)
	movl	12(%rsp), %ebx
	movslq	%r9d, %r9
	leal	(%rbx,%rax), %edx
	addl	8(%rsp), %eax
	imull	%esi, %edx
	movslq	%edx, %rbx
	imull	%esi, %eax
	subl	64(%rsp), %esi
	movq	%rbx, 128(%rsp)
	cltq
	movq	%rax, 120(%rsp)
	leaq	16(%r15), %rax
	movq	%rax, 32(%rsp)
	movl	%esi, 52(%rsp)
	.p2align 4
	.p2align 3
.L59:
	movq	128(%rsp), %rdx
	movq	120(%rsp), %rsi
	movq	96(%rsp), %rbx
	movq	%r9, %rax
	movq	136(%rsp), %rcx
	vmovsd	8(%r13), %xmm4
	vmovsd	8(%r14), %xmm6
	vmovsd	0(%r13), %xmm17
	vmovsd	%xmm4, 144(%rsp)
	vmovsd	(%r14), %xmm24
	vmovsd	%xmm6, 152(%rsp)
	salq	$4, %rax
	salq	$4, %rdx
	vmovsd	8(%rbx), %xmm2
	vmovsd	(%rbx), %xmm23
	vmovsd	%xmm2, 160(%rsp)
	salq	$4, %rsi
	leaq	64(%rdx), %rbx
	leaq	64(%rsi), %rdi
	leaq	64(%rax), %r12
	salq	$4, %rcx
	cmpq	%rbx, %rsi
	leaq	64(%rcx), %r11
	setge	%r8b
	cmpq	%rdi, %rdx
	setge	%r10b
	orl	%r10d, %r8d
	cmpq	%rcx, %rdi
	setle	%r10b
	cmpq	%r11, %rsi
	setge	80(%rsp)
	orb	80(%rsp), %r10b
	andl	%r10d, %r8d
	cmpq	%rdi, %rax
	setge	%dil
	cmpq	%r12, %rsi
	setge	%r10b
	orl	%r10d, %edi
	cmpq	%rcx, %rbx
	setle	%r10b
	cmpq	%r11, %rdx
	setge	80(%rsp)
	orb	80(%rsp), %r10b
	andl	%r10d, %edi
	testb	%dil, %r8b
	je	.L51
	cmpq	%rbx, %rax
	setge	%r8b
	cmpq	%r12, %rdx
	setge	%dil
	orl	%r8d, %edi
	cmpq	%r11, %rax
	setge	%r8b
	cmpq	%r12, %rcx
	setge	%r10b
	orl	%r10d, %r8d
	andl	%edi, %r8d
	cmpl	$1, 108(%rsp)
	setne	%dil
	testb	%dil, %r8b
	je	.L51
	cmpl	$2, 68(%rsp)
	jbe	.L65
	movq	56(%rsp), %rbx
	leaq	(%rax,%r15), %rdi
	vbroadcastsd	%xmm17, %ymm13
	leaq	(%rcx,%r15), %r11
	vbroadcastsd	%xmm4, %ymm22
	vbroadcastsd	%xmm24, %ymm12
	leaq	(%rdx,%r15), %r10
	vbroadcastsd	%xmm6, %ymm21
	vbroadcastsd	%xmm23, %ymm11
	leaq	(%rsi,%r15), %r8
	vbroadcastsd	%xmm2, %ymm18
	vmovsd	%xmm23, 80(%rsp)
	vmovsd	%xmm17, 72(%rsp)
	addq	%rdi, %rbx
	.p2align 4
	.p2align 3
.L53:
	vmovupd	32(%r11), %ymm3
	vmovupd	(%r11), %ymm4
	vmovapd	%ymm13, %ymm2
	vmovapd	%ymm13, %ymm5
	vmovapd	%ymm12, %ymm7
	vmovapd	%ymm12, %ymm6
	vmovupd	32(%r8), %ymm15
	vmovupd	(%rdi), %ymm23
	vmovupd	32(%rdi), %ymm17
	vmovapd	%ymm11, %ymm29
	vmovapd	%ymm11, %ymm30
	addq	$64, %rdi
	vmovapd	%ymm13, %ymm26
	vmovapd	%ymm12, %ymm27
	addq	$64, %r11
	addq	$64, %r10
	addq	$64, %r8
	vmulpd	%ymm22, %ymm3, %ymm0
	vmulpd	%ymm22, %ymm4, %ymm1
	vmovapd	%ymm23, %ymm14
	vmovapd	%ymm23, %ymm8
	vpermt2pd	%ymm17, %ymm20, %ymm14
	vpermt2pd	%ymm17, %ymm28, %ymm8
	vpermilpd	$5, %ymm0, %ymm0
	vpermilpd	$5, %ymm1, %ymm1
	vfmadd132pd	%ymm3, %ymm0, %ymm2
	vfmsub231pd	%ymm3, %ymm13, %ymm0
	vfmadd132pd	%ymm4, %ymm1, %ymm5
	vfmsub231pd	%ymm4, %ymm13, %ymm1
	vshufpd	$10, %ymm2, %ymm0, %ymm0
	vmovapd	%ymm4, %ymm2
	vpermt2pd	%ymm3, %ymm20, %ymm4
	vpermt2pd	%ymm3, %ymm28, %ymm2
	vmulpd	%ymm4, %ymm22, %ymm25
	vmulpd	%ymm2, %ymm22, %ymm3
	vfmadd132pd	%ymm2, %ymm14, %ymm26
	vshufpd	$10, %ymm5, %ymm1, %ymm1
	vmovupd	-32(%r10), %ymm5
	vfnmadd132pd	%ymm13, %ymm25, %ymm2
	vfmadd132pd	%ymm13, %ymm3, %ymm4
	vmovupd	-64(%r10), %ymm3
	vmulpd	%ymm21, %ymm5, %ymm9
	vmulpd	%ymm21, %ymm3, %ymm10
	vpermilpd	$5, %ymm9, %ymm9
	vfmadd132pd	%ymm5, %ymm9, %ymm6
	vfmsub231pd	%ymm5, %ymm12, %ymm9
	vpermilpd	$5, %ymm10, %ymm10
	vfmadd132pd	%ymm3, %ymm10, %ymm7
	vfmsub231pd	%ymm3, %ymm12, %ymm10
	vshufpd	$10, %ymm6, %ymm9, %ymm9
	vaddpd	%ymm0, %ymm9, %ymm9
	vshufpd	$10, %ymm7, %ymm10, %ymm10
	vaddpd	%ymm1, %ymm10, %ymm10
	vmovapd	%ymm3, %ymm1
	vpermt2pd	%ymm5, %ymm20, %ymm3
	vpermt2pd	%ymm5, %ymm28, %ymm1
	vmulpd	%ymm3, %ymm21, %ymm7
	vmulpd	%ymm1, %ymm21, %ymm0
	vmulpd	%ymm18, %ymm15, %ymm5
	vfmadd132pd	%ymm1, %ymm8, %ymm27
	vfnmadd132pd	%ymm12, %ymm7, %ymm1
	vsubpd	%ymm7, %ymm2, %ymm7
	vfmadd132pd	%ymm12, %ymm0, %ymm3
	vmovupd	-64(%r8), %ymm0
	vpermilpd	$5, %ymm5, %ymm5
	vfmadd132pd	%ymm15, %ymm5, %ymm29
	vfmsub231pd	%ymm15, %ymm11, %ymm5
	vaddpd	%ymm27, %ymm7, %ymm7
	vaddpd	%ymm3, %ymm14, %ymm16
	vmulpd	%ymm18, %ymm0, %ymm6
	vshufpd	$10, %ymm29, %ymm5, %ymm5
	vaddpd	%ymm17, %ymm5, %ymm5
	vpermilpd	$5, %ymm6, %ymm6
	vfmadd132pd	%ymm0, %ymm6, %ymm30
	vfmsub231pd	%ymm0, %ymm11, %ymm6
	vaddpd	%ymm9, %ymm5, %ymm5
	vmovapd	%ymm0, %ymm9
	vpermt2pd	%ymm15, %ymm28, %ymm9
	vpermt2pd	%ymm15, %ymm20, %ymm0
	vmulpd	%ymm9, %ymm18, %ymm15
	vfmadd231pd	%ymm9, %ymm11, %ymm14
	vmovupd	%ymm5, -32(%rdi)
	vshufpd	$10, %ymm30, %ymm6, %ymm6
	vaddpd	%ymm23, %ymm6, %ymm6
	vaddpd	%ymm10, %ymm6, %ymm6
	vmulpd	%ymm0, %ymm18, %ymm10
	vfmadd132pd	%ymm11, %ymm15, %ymm0
	vfnmadd132pd	%ymm11, %ymm10, %ymm9
	vsubpd	%ymm10, %ymm2, %ymm2
	vmovupd	%ymm6, -64(%rdi)
	vaddpd	%ymm1, %ymm0, %ymm5
	vsubpd	%ymm0, %ymm8, %ymm6
	vaddpd	%ymm1, %ymm4, %ymm1
	vaddpd	%ymm0, %ymm4, %ymm0
	vaddpd	%ymm14, %ymm2, %ymm2
	vsubpd	%ymm4, %ymm8, %ymm8
	vaddpd	%ymm7, %ymm9, %ymm7
	vsubpd	%ymm25, %ymm9, %ymm9
	vaddpd	%ymm6, %ymm1, %ymm1
	vsubpd	%ymm0, %ymm16, %ymm16
	vsubpd	%ymm3, %ymm2, %ymm2
	vaddpd	%ymm5, %ymm8, %ymm8
	vaddpd	%ymm26, %ymm9, %ymm9
	vmovapd	%ymm7, %ymm0
	vmovapd	%ymm1, %ymm6
	vpermt2pd	%ymm16, %ymm19, %ymm0
	vpermt2pd	%ymm16, %ymm31, %ymm7
	vpermt2pd	%ymm2, %ymm19, %ymm6
	vpermt2pd	%ymm2, %ymm31, %ymm1
	vsubpd	%ymm3, %ymm9, %ymm9
	vmovupd	%ymm6, -64(%r11)
	vmovupd	%ymm1, -32(%r11)
	vmovupd	%ymm0, -64(%r10)
	vmovapd	%ymm8, %ymm0
	vpermt2pd	%ymm9, %ymm19, %ymm0
	vpermt2pd	%ymm9, %ymm31, %ymm8
	vmovupd	%ymm7, -32(%r10)
	vmovupd	%ymm0, -64(%r8)
	vmovupd	%ymm8, -32(%r8)
	cmpq	%rbx, %rdi
	jne	.L53
	movl	64(%rsp), %ebx
	vmovsd	80(%rsp), %xmm23
	vmovsd	72(%rsp), %xmm17
	cmpl	%ebx, 108(%rsp)
	je	.L58
	movl	52(%rsp), %edi
	movl	%edi, %r12d
	cmpl	$1, %edi
	je	.L66
	movl	%ebx, %r8d
	movl	%ebx, %edi
.L52:
	leaq	(%r9,%r8), %rbx
	movq	136(%rsp), %r11
	movq	128(%rsp), %r10
	vmovddup	144(%rsp), %xmm1
	vmovddup	%xmm17, %xmm15
	vmovddup	152(%rsp), %xmm0
	vmovddup	160(%rsp), %xmm11
	vmovddup	%xmm24, %xmm21
	salq	$4, %rbx
	vmovddup	%xmm23, %xmm13
	addq	%r15, %rbx
	vmovupd	(%rbx), %xmm4
	addq	%r8, %r11
	addq	%r8, %r10
	addq	120(%rsp), %r8
	salq	$4, %r11
	addq	%r15, %r11
	salq	$4, %r10
	vmovhpd	16(%rbx), %xmm4, %xmm6
	vmovupd	16(%rbx), %xmm4
	vmovlpd	8(%rbx), %xmm4, %xmm10
	vmovupd	(%r11), %xmm4
	addq	%r15, %r10
	salq	$4, %r8
	addq	%r15, %r8
	vmovhpd	16(%r11), %xmm4, %xmm9
	vmovupd	16(%r11), %xmm4
	vmovlpd	8(%r11), %xmm4, %xmm7
	vmovupd	(%r10), %xmm4
	vmovapd	%xmm9, %xmm8
	vmulpd	%xmm1, %xmm7, %xmm18
	vmulpd	%xmm1, %xmm9, %xmm1
	vfmadd132pd	%xmm15, %xmm1, %xmm7
	vfmsub132pd	%xmm15, %xmm18, %xmm8
	vmovhpd	16(%r10), %xmm4, %xmm1
	vmovupd	16(%r10), %xmm4
	vmovlpd	8(%r10), %xmm4, %xmm3
	vmovupd	(%r8), %xmm4
	vmovapd	%xmm1, %xmm16
	vfnmadd132pd	%xmm15, %xmm18, %xmm9
	vmovhpd	16(%r8), %xmm4, %xmm5
	vmovupd	16(%r8), %xmm4
	vmovlpd	8(%r8), %xmm4, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm22
	vmovapd	%xmm5, %xmm4
	vmulpd	%xmm0, %xmm1, %xmm0
	vfmsub132pd	%xmm21, %xmm22, %xmm16
	vfnmadd132pd	%xmm21, %xmm22, %xmm1
	vfmadd132pd	%xmm21, %xmm0, %xmm3
	vmulpd	%xmm11, %xmm2, %xmm14
	vmulpd	%xmm11, %xmm5, %xmm11
	vaddpd	%xmm16, %xmm8, %xmm26
	vaddpd	%xmm9, %xmm16, %xmm16
	vfmsub132pd	%xmm13, %xmm14, %xmm4
	vaddpd	%xmm3, %xmm7, %xmm25
	vaddpd	%xmm3, %xmm10, %xmm12
	vfnmadd132pd	%xmm13, %xmm14, %xmm5
	vfmadd132pd	%xmm13, %xmm11, %xmm2
	vaddpd	%xmm4, %xmm6, %xmm0
	vaddpd	%xmm2, %xmm10, %xmm11
	vsubpd	%xmm3, %xmm10, %xmm10
	vaddpd	%xmm0, %xmm26, %xmm0
	vaddpd	%xmm9, %xmm4, %xmm3
	vaddpd	%xmm25, %xmm11, %xmm11
	vaddpd	%xmm10, %xmm3, %xmm3
	vunpcklpd	%xmm11, %xmm0, %xmm25
	vunpckhpd	%xmm11, %xmm0, %xmm0
	vaddpd	%xmm1, %xmm2, %xmm11
	vaddpd	%xmm1, %xmm7, %xmm1
	vmovupd	%xmm0, 16(%rbx)
	vsubpd	%xmm2, %xmm6, %xmm0
	vaddpd	%xmm2, %xmm7, %xmm2
	vmovupd	%xmm25, (%rbx)
	vaddpd	%xmm0, %xmm1, %xmm0
	vsubpd	%xmm2, %xmm12, %xmm12
	vunpcklpd	%xmm3, %xmm0, %xmm1
	vunpckhpd	%xmm3, %xmm0, %xmm0
	vaddpd	%xmm5, %xmm6, %xmm3
	vmovupd	%xmm0, 16(%r11)
	vmovupd	%xmm1, (%r11)
	vaddpd	%xmm5, %xmm8, %xmm1
	vaddpd	%xmm16, %xmm3, %xmm3
	vaddpd	%xmm10, %xmm1, %xmm1
	vunpcklpd	%xmm12, %xmm3, %xmm0
	vunpckhpd	%xmm12, %xmm3, %xmm3
	vmovupd	%xmm0, (%r10)
	vsubpd	%xmm7, %xmm6, %xmm0
	vmovupd	%xmm3, 16(%r10)
	vaddpd	%xmm11, %xmm0, %xmm0
	vunpcklpd	%xmm1, %xmm0, %xmm2
	vunpckhpd	%xmm1, %xmm0, %xmm0
	vmovupd	%xmm2, (%r8)
	vmovupd	%xmm0, 16(%r8)
	movl	%r12d, %r8d
	andl	$-2, %r8d
	addl	%r8d, %edi
	cmpl	%r12d, %r8d
	je	.L58
.L55:
	movslq	%edi, %rdi
	vmovsd	144(%rsp), %xmm7
	vmovsd	160(%rsp), %xmm27
	vmovsd	%xmm17, %xmm17, %xmm6
	salq	$4, %rdi
	vmovsd	%xmm24, %xmm24, %xmm10
	addq	%rdi, %rax
	addq	%rdi, %rcx
	addq	%rdi, %rdx
	addq	%rsi, %rdi
	addq	%r15, %rcx
	addq	%r15, %rdx
	addq	%r15, %rdi
	addq	%r15, %rax
	vmovsd	(%rcx), %xmm8
	vmovsd	(%rdi), %xmm3
	vmovsd	8(%rcx), %xmm5
	vmovsd	8(%rdx), %xmm9
	vmulsd	%xmm7, %xmm5, %xmm14
	vmulsd	%xmm7, %xmm8, %xmm0
	vfmsub132sd	%xmm8, %xmm14, %xmm6
	vfmadd132sd	%xmm17, %xmm0, %xmm5
	vmovsd	152(%rsp), %xmm7
	vmovsd	(%rdx), %xmm0
	vmulsd	%xmm7, %xmm9, %xmm15
	vmulsd	%xmm7, %xmm0, %xmm1
	vfmsub132sd	%xmm0, %xmm15, %xmm10
	vfmadd132sd	%xmm24, %xmm1, %xmm9
	vmulsd	%xmm27, %xmm3, %xmm21
	vmovsd	8(%rdi), %xmm1
	vmovsd	%xmm23, %xmm23, %xmm7
	vmulsd	%xmm27, %xmm1, %xmm13
	vmovsd	(%rax), %xmm4
	vmovsd	8(%rax), %xmm2
	vfmadd132sd	%xmm23, %xmm21, %xmm1
	vfmsub132sd	%xmm3, %xmm13, %xmm7
	vfnmadd132sd	%xmm24, %xmm15, %xmm0
	vfnmadd132sd	%xmm17, %xmm14, %xmm8
	vaddsd	%xmm10, %xmm6, %xmm12
	vaddsd	%xmm9, %xmm2, %xmm11
	vaddsd	%xmm9, %xmm5, %xmm16
	vfnmadd132sd	%xmm23, %xmm13, %xmm3
	vaddsd	%xmm7, %xmm4, %xmm18
	vsubsd	%xmm1, %xmm4, %xmm15
	vaddsd	%xmm18, %xmm12, %xmm12
	vmovsd	%xmm12, (%rax)
	vaddsd	%xmm1, %xmm2, %xmm12
	vsubsd	%xmm9, %xmm2, %xmm2
	vaddsd	%xmm16, %xmm12, %xmm12
	vaddsd	%xmm8, %xmm10, %xmm10
	vmovsd	%xmm12, 8(%rax)
	vaddsd	%xmm8, %xmm7, %xmm7
	vaddsd	%xmm0, %xmm1, %xmm12
	vaddsd	%xmm2, %xmm7, %xmm7
	vaddsd	%xmm0, %xmm5, %xmm0
	vmovsd	%xmm7, 8(%rcx)
	vaddsd	%xmm15, %xmm0, %xmm0
	vaddsd	%xmm1, %xmm5, %xmm1
	vmovsd	%xmm0, (%rcx)
	vsubsd	%xmm1, %xmm11, %xmm1
	vaddsd	%xmm3, %xmm4, %xmm0
	vmovsd	%xmm1, 8(%rdx)
	vaddsd	%xmm10, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx)
	vsubsd	%xmm5, %xmm4, %xmm0
	vaddsd	%xmm12, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdi)
	vaddsd	%xmm3, %xmm6, %xmm0
	vaddsd	%xmm2, %xmm0, %xmm0
	vmovsd	%xmm0, 8(%rdi)
.L58:
	movq	88(%rsp), %rbx
	incl	112(%rsp)
	movl	112(%rsp), %eax
	addq	88(%rsp), %r9
	addq	$16, %r13
	addq	$16, 96(%rsp)
	addq	%rbx, 136(%rsp)
	addq	%rbx, 128(%rsp)
	addq	$16, %r14
	addq	%rbx, 120(%rsp)
	cmpl	%eax, 104(%rsp)
	jg	.L59
	movl	20(%rsp), %esi
	movl	4(%rsp), %ebx
	addl	%esi, 48(%rsp)
	movl	48(%rsp), %eax
	addl	%ebx, 28(%rsp)
	addl	%ebx, 24(%rsp)
	cmpl	%eax, 16(%rsp)
	jg	.L48
.L47:
	movl	104(%rsp), %esi
	sall	$2, 20(%rsp)
	movl	20(%rsp), %eax
	addl	%esi, -48(%rsp)
	cmpl	%eax, 16(%rsp)
	jge	.L64
	vzeroupper
.L85:
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4
	.p2align 3
.L51:
	.cfi_restore_state
	movq	40(%rsp), %rbx
	addq	%r15, %rax
	addq	%r15, %rcx
	addq	%r15, %rdx
	addq	%r15, %rsi
	vmovsd	160(%rsp), %xmm9
	vmovsd	152(%rsp), %xmm10
	vmovsd	144(%rsp), %xmm11
	leaq	(%rbx,%r9), %rdi
	salq	$4, %rdi
	addq	32(%rsp), %rdi
	.p2align 4
	.p2align 3
.L57:
	vmovsd	(%rcx), %xmm8
	vmovsd	(%rsi), %xmm3
	vmovsd	8(%rcx), %xmm5
	vmovsd	8(%rdx), %xmm12
	vmulsd	%xmm5, %xmm11, %xmm18
	vmulsd	%xmm12, %xmm10, %xmm21
	vmulsd	%xmm8, %xmm11, %xmm0
	vmulsd	%xmm3, %xmm9, %xmm26
	vfmadd132sd	%xmm17, %xmm0, %xmm5
	vmovsd	%xmm17, %xmm17, %xmm6
	vmovsd	(%rdx), %xmm0
	vfmsub132sd	%xmm8, %xmm18, %xmm6
	vmulsd	%xmm0, %xmm10, %xmm1
	vmovsd	%xmm24, %xmm24, %xmm13
	vmovsd	%xmm23, %xmm23, %xmm7
	vmovsd	(%rax), %xmm4
	vfmsub132sd	%xmm0, %xmm21, %xmm13
	vfmadd132sd	%xmm24, %xmm1, %xmm12
	vmovsd	8(%rsi), %xmm1
	vmulsd	%xmm1, %xmm9, %xmm16
	vmovsd	8(%rax), %xmm2
	addq	$16, %rax
	addq	$16, %rcx
	addq	$16, %rdx
	vfmadd132sd	%xmm23, %xmm26, %xmm1
	vfmsub132sd	%xmm3, %xmm16, %xmm7
	addq	$16, %rsi
	vfnmadd132sd	%xmm24, %xmm21, %xmm0
	vfnmadd132sd	%xmm17, %xmm18, %xmm8
	vaddsd	%xmm13, %xmm6, %xmm14
	vaddsd	%xmm12, %xmm5, %xmm22
	vaddsd	%xmm12, %xmm2, %xmm15
	vfnmadd132sd	%xmm23, %xmm16, %xmm3
	vaddsd	%xmm7, %xmm4, %xmm25
	vaddsd	%xmm25, %xmm14, %xmm14
	vmovsd	%xmm14, -16(%rax)
	vaddsd	%xmm1, %xmm2, %xmm14
	vaddsd	%xmm22, %xmm14, %xmm14
	vsubsd	%xmm1, %xmm4, %xmm21
	vmovsd	%xmm14, -8(%rax)
	vsubsd	%xmm12, %xmm2, %xmm2
	vaddsd	%xmm0, %xmm1, %xmm14
	vaddsd	%xmm8, %xmm13, %xmm13
	vaddsd	%xmm0, %xmm5, %xmm0
	vaddsd	%xmm8, %xmm7, %xmm7
	vaddsd	%xmm21, %xmm0, %xmm0
	vaddsd	%xmm2, %xmm7, %xmm7
	vmovsd	%xmm0, -16(%rcx)
	vmovsd	%xmm7, -8(%rcx)
	vaddsd	%xmm3, %xmm4, %xmm0
	vaddsd	%xmm1, %xmm5, %xmm1
	vaddsd	%xmm13, %xmm0, %xmm0
	vsubsd	%xmm1, %xmm15, %xmm1
	vmovsd	%xmm0, -16(%rdx)
	vmovsd	%xmm1, -8(%rdx)
	vsubsd	%xmm5, %xmm4, %xmm0
	vaddsd	%xmm14, %xmm0, %xmm0
	vmovsd	%xmm0, -16(%rsi)
	vaddsd	%xmm3, %xmm6, %xmm0
	vaddsd	%xmm2, %xmm0, %xmm0
	vmovsd	%xmm0, -8(%rsi)
	cmpq	%rdi, %rax
	jne	.L57
	jmp	.L58
.L65:
	movl	108(%rsp), %r12d
	xorl	%r8d, %r8d
	xorl	%edi, %edi
	jmp	.L52
.L66:
	movl	%ebx, %edi
	jmp	.L55
.L88:
	movl	$0, 112(%rsp)
	jmp	.L63
	.cfi_endproc
.LFE13:
	.size	block_fft_forward, .-block_fft_forward
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC2:
	.long	0
	.long	1072693248
	.align 8
.LC4:
	.long	1413754136
	.long	-1072094725
	.align 8
.LC5:
	.long	0
	.long	1074266112
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC6:
	.quad	0
	.quad	2
	.quad	4
	.quad	6
	.align 32
.LC7:
	.quad	1
	.quad	3
	.quad	5
	.quad	7
	.align 32
.LC8:
	.quad	0
	.quad	4
	.quad	1
	.quad	5
	.align 32
.LC9:
	.quad	2
	.quad	6
	.quad	3
	.quad	7
	.ident	"GCC: (GNU) 11.5.0 20240719 (Red Hat 11.5.0-14)"
	.section	.note.GNU-stack,"",@progbits
