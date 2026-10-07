	.file	"block_fst.c"
	.text
	.p2align 4
	.globl	block_fst_plan_free
	.type	block_fst_plan_free, @function
block_fst_plan_free:
.LFB10:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	leaq	16(%rdi), %rdi
	call	block_fft_plan_free
	movq	64(%rbx), %rdi
	call	free
	movq	72(%rbx), %rdi
	movq	$0, 64(%rbx)
	call	free
	movq	80(%rbx), %rdi
	movq	$0, 72(%rbx)
	call	free
	movq	$0, 80(%rbx)
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE10:
	.size	block_fst_plan_free, .-block_fst_plan_free
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"block_fst: require m,n >= 1\n"
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC1:
	.string	"block_fst: require L=2*(n+1)=4^k; try n=31,127,511,2047,...\n"
	.text
	.p2align 4
	.globl	block_fst_plan_init
	.type	block_fst_plan_init, @function
block_fst_plan_init:
.LFB9:
	.cfi_startproc
	vmovd	%esi, %xmm1
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	vpinsrd	$1, %edx, %xmm1, %xmm0
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	leal	2(%rdx,%rdx), %ebp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$8, %rsp
	.cfi_def_cfa_offset 64
	vmovq	%xmm0, (%rdi)
	movl	%ebp, 8(%rdi)
	movq	$0, 64(%rdi)
	movq	$0, 72(%rdi)
	movq	$0, 80(%rdi)
	testl	%esi, %esi
	jle	.L5
	movl	%edx, %ebx
	testl	%edx, %edx
	jle	.L5
	movq	%rdi, %r12
	movl	%esi, %r13d
	movl	$1, %eax
	cmpl	$1, %ebp
	jle	.L7
	.p2align 4
	.p2align 3
.L6:
	sall	$2, %eax
	cmpl	%eax, %ebp
	jg	.L6
	jne	.L7
	movslq	%r13d, %r15
	movslq	%ebp, %rdi
	imulq	%r15, %rdi
	salq	$4, %rdi
	call	malloc
	movslq	%ebx, %rdi
	movq	%rdi, %rbx
	movq	%rax, %r14
	movq	%rax, 64(%r12)
	imulq	%r15, %rbx
	leaq	0(,%rbx,8), %rdi
	call	malloc
	movq	%rax, 80(%r12)
	testq	%r14, %r14
	je	.L13
	testq	%rax, %rax
	je	.L13
	leaq	16(%r12), %r14
	movl	$4, %ecx
	movl	%r13d, %edx
	movl	%ebp, %esi
	movq	%r14, %rdi
	call	block_fft_plan_init
	testl	%eax, %eax
	jne	.L20
.L4:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
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
	.p2align 4
	.p2align 3
.L7:
	.cfi_restore_state
	movq	stderr(%rip), %rcx
	movl	$60, %edx
	movl	$1, %esi
	movl	$.LC1, %edi
	call	fwrite
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	$1, %eax
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
	.p2align 4
	.p2align 3
.L5:
	.cfi_restore_state
	movq	stderr(%rip), %rcx
	movl	$28, %edx
	movl	$1, %esi
	movl	$.LC0, %edi
	call	fwrite
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	$1, %eax
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
.L20:
	.cfi_restore_state
	movq	%r14, %rdi
	call	block_fft_plan_free
	movq	64(%r12), %rdi
	call	free
	movq	72(%r12), %rdi
	movq	$0, 64(%r12)
	call	free
	movq	80(%r12), %rdi
	movq	$0, 72(%r12)
	call	free
	movl	$3, %eax
	movq	$0, 80(%r12)
	jmp	.L4
.L13:
	movq	%r12, %rdi
	call	block_fst_plan_free
	movl	$2, %eax
	jmp	.L4
	.cfi_endproc
.LFE9:
	.size	block_fst_plan_init, .-block_fst_plan_init
	.p2align 4
	.globl	block_fst_apply
	.type	block_fst_apply, @function
block_fst_apply:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rdi, %rax
	vxorps	%xmm1, %xmm1, %xmm1
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
	movq	%rsi, %rbx
	andq	$-32, %rsp
	subq	$192, %rsp
	movl	(%rdi), %r12d
	movl	8(%rax), %ecx
	movl	4(%rdi), %esi
	movq	%rdi, 136(%rsp)
	vmovsd	.LC2(%rip), %xmm0
	imull	%r12d, %ecx
	leal	1(%rsi), %edi
	movl	%esi, 188(%rsp)
	movq	64(%rax), %rsi
	vcvtsi2sdl	%edi, %xmm1, %xmm1
	movl	%edi, 56(%rsp)
	vdivsd	%xmm1, %xmm0, %xmm5
	vsqrtsd	%xmm5, %xmm5, %xmm5
	vmulsd	.LC3(%rip), %xmm5, %xmm5
	testl	%ecx, %ecx
	jle	.L28
	cmpl	$1, %ecx
	je	.L54
	movl	%ecx, %edx
	movq	%rsi, %rax
	vxorpd	%xmm0, %xmm0, %xmm0
	shrl	%edx
	decl	%edx
	salq	$5, %rdx
	leaq	32(%rsi,%rdx), %rdx
	.p2align 4
	.p2align 3
.L26:
	vmovupd	%ymm0, (%rax)
	addq	$32, %rax
	cmpq	%rdx, %rax
	jne	.L26
	movl	%ecx, %eax
	andl	$-2, %eax
	andl	$1, %ecx
	je	.L28
.L25:
	cltq
	salq	$4, %rax
	addq	%rsi, %rax
	movq	$0x000000000, (%rax)
	movq	$0x000000000, 8(%rax)
.L28:
	movq	136(%rsp), %rax
	movl	188(%rsp), %edx
	addq	$16, %rax
	movq	%rax, 128(%rsp)
	testl	%edx, %edx
	jle	.L30
	testl	%r12d, %r12d
	jle	.L30
	movl	188(%rsp), %r8d
	cmpl	$2, %r8d
	jle	.L55
	leal	3(%r8), %edx
	leal	(%r12,%r12), %eax
	movslq	%r12d, %r13
	leal	-3(%r8), %r11d
	imull	%r12d, %edx
	movslq	%eax, %rcx
	andl	$-2, %r11d
	vbroadcastsd	.LC8(%rip), %ymm2
	movq	%rcx, %r15
	leaq	(%rbx,%r13,8), %r10
	movq	%rbx, %r14
	movq	%r13, 160(%rsp)
	salq	$3, %rcx
	movq	%rbx, 64(%rsp)
	vmovq	.LC5(%rip), %xmm7
	movl	$1, 176(%rsp)
	vmovapd	%xmm7, %xmm6
	movq	%rsi, 120(%rsp)
	movslq	%edx, %rdi
	movq	%rcx, 88(%rsp)
	addq	%rsi, %rcx
	subl	%r12d, %edx
	salq	$4, %rdi
	movq	%rcx, 168(%rsp)
	leal	(%rax,%r12), %ecx
	movq	%rdi, 152(%rsp)
	movslq	%edx, %rdi
	negl	%eax
	subl	%ecx, %edx
	movl	%eax, 112(%rsp)
	leal	-2(%r8), %eax
	movslq	%edx, %rdx
	imull	%r12d, %eax
	leaq	(%rbx,%rdx,8), %r9
	movq	%r13, %rdx
	negq	%rdx
	salq	$4, %rdx
	movq	%rdx, 80(%rsp)
	movslq	%eax, %rdx
	leal	3(%r11), %eax
	movq	152(%rsp), %r11
	movl	%eax, 116(%rsp)
	movl	%r12d, %eax
	movl	%edx, 144(%rsp)
	movq	%r13, 152(%rsp)
	shrl	$3, %eax
	decl	%eax
	incq	%rax
	salq	$6, %rax
	movq	%rax, 48(%rsp)
	movl	%r12d, %eax
	andl	$-8, %eax
	movl	%eax, 60(%rsp)
	movq	%rax, 40(%rsp)
	leal	-1(%r12), %eax
	movl	%eax, 184(%rsp)
	movl	%r12d, %eax
	movq	%rax, 32(%rsp)
	salq	$3, %rax
	movq	%rax, 24(%rsp)
	leal	-1(%r8), %eax
	imull	%r12d, %eax
	salq	$4, %rdi
	cltq
	movq	%rdi, 96(%rsp)
	movq	96(%rsp), %rbx
	subq	%rax, %rdx
	leaq	0(,%rdx,8), %rax
	movq	%rax, 16(%rsp)
	leaq	(%r13,%r13), %rax
	movq	80(%rsp), %r13
	vmovq	%rax, %xmm8
	movl	$16, %eax
	salq	$4, %r15
	subq	%rsi, %rax
	movq	%r15, 104(%rsp)
	movq	%rax, 72(%rsp)
.L39:
	movq	160(%rsp), %rax
	vmovq	%xmm8, %rsi
	leaq	16(%r15), %rdx
	movq	72(%rsp), %rdi
	subq	152(%rsp), %rax
	leaq	16(%rbx), %rcx
	addq	168(%rsp), %rdi
	addq	%rsi, %rax
	movq	%rax, 96(%rsp)
	movq	%r11, %rax
	subq	%rdx, %rax
	cmpq	$96, %rax
	movq	%r15, %rax
	seta	%dl
	subq	%rcx, %rax
	cmpq	$96, %rax
	seta	%al
	andl	%eax, %edx
	movq	%r11, %rax
	subq	%rcx, %rax
	cmpq	$96, %rax
	seta	%al
	cmpl	$6, 184(%rsp)
	seta	%cl
	andl	%ecx, %eax
	testb	%al, %dl
	je	.L34
	movq	%r15, %rax
	movq	%r11, %rdx
	subq	%rdi, %rax
	cmpq	$96, %rax
	seta	%al
	subq	%rdi, %rdx
	cmpq	$96, %rdx
	seta	%dl
	andl	%edx, %eax
	movq	%rbx, %rdx
	subq	%rdi, %rdx
	cmpq	$96, %rdx
	seta	%dl
	testb	%dl, %al
	je	.L34
	movq	120(%rsp), %rcx
	movslq	144(%rsp), %rax
	movq	64(%rsp), %rsi
	movq	%r13, 80(%rsp)
	movq	168(%rsp), %rdx
	movq	%rbx, %r13
	leaq	(%rbx,%rcx), %r8
	leaq	(%r15,%rcx), %rdi
	movq	%r11, %rbx
	addq	%r11, %rcx
	movq	48(%rsp), %r11
	leaq	(%rsi,%rax,8), %rsi
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L47:
	vmovupd	(%r14,%rax), %xmm0
	vmovupd	(%r14,%rax), %ymm3
	vxorpd	(%r9,%rax), %ymm2, %ymm1
	subq	$-128, %rdx
	subq	$-128, %r8
	subq	$-128, %rdi
	subq	$-128, %rcx
	vmovlpd	%xmm0, -128(%rdx)
	vmovhpd	%xmm0, -112(%rdx)
	vextractf64x2	$0x1, %ymm3, %xmm0
	vmovupd	32(%r14,%rax), %ymm3
	vmovlpd	%xmm0, -96(%rdx)
	vmovhpd	%xmm0, -80(%rdx)
	vmovupd	32(%r14,%rax), %xmm0
	vmovlpd	%xmm0, -64(%rdx)
	vmovhpd	%xmm0, -48(%rdx)
	vextractf64x2	$0x1, %ymm3, %xmm0
	vmovupd	(%r10,%rax), %ymm3
	vmovlpd	%xmm0, -32(%rdx)
	vmovhpd	%xmm0, -16(%rdx)
	vxorpd	32(%rax,%r9), %ymm2, %ymm0
	vmovlpd	%xmm1, -128(%r8)
	vmovhpd	%xmm1, -112(%r8)
	vextractf64x2	$0x1, %ymm1, %xmm1
	vmovlpd	%xmm1, -96(%r8)
	vmovhpd	%xmm1, -80(%r8)
	vmovlpd	%xmm0, -64(%r8)
	vmovhpd	%xmm0, -48(%r8)
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmovlpd	%xmm0, -32(%r8)
	vmovhpd	%xmm0, -16(%r8)
	vmovupd	(%r10,%rax), %xmm0
	vmovlpd	%xmm0, -128(%rdi)
	vmovhpd	%xmm0, -112(%rdi)
	vextractf64x2	$0x1, %ymm3, %xmm0
	vmovupd	32(%r10,%rax), %ymm3
	vmovlpd	%xmm0, -96(%rdi)
	vmovhpd	%xmm0, -80(%rdi)
	vmovupd	32(%r10,%rax), %xmm0
	vmovlpd	%xmm0, -64(%rdi)
	vmovhpd	%xmm0, -48(%rdi)
	vextractf64x2	$0x1, %ymm3, %xmm0
	vmovlpd	%xmm0, -32(%rdi)
	vxorpd	(%rsi,%rax), %ymm2, %ymm1
	vmovhpd	%xmm0, -16(%rdi)
	vxorpd	32(%rsi,%rax), %ymm2, %ymm0
	addq	$64, %rax
	vmovlpd	%xmm1, -128(%rcx)
	vmovhpd	%xmm1, -112(%rcx)
	vextractf64x2	$0x1, %ymm1, %xmm1
	vmovlpd	%xmm0, -64(%rcx)
	vmovhpd	%xmm0, -48(%rcx)
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmovlpd	%xmm1, -96(%rcx)
	vmovhpd	%xmm1, -80(%rcx)
	vmovlpd	%xmm0, -32(%rcx)
	vmovhpd	%xmm0, -16(%rcx)
	cmpq	%r11, %rax
	jne	.L47
	movq	%rbx, %r11
	movq	%r13, %rbx
	movq	80(%rsp), %r13
	cmpl	%r12d, 60(%rsp)
	je	.L38
	movq	160(%rsp), %rdx
	movq	40(%rsp), %rax
	addq	%rax, %rdx
	salq	$4, %rdx
	addq	120(%rsp), %rdx
	.p2align 4
	.p2align 3
.L36:
	leaq	(%rdx,%r13), %rcx
	vmovsd	(%r14,%rax,8), %xmm0
	vmovsd	%xmm0, (%rdx)
	vmovsd	(%r9,%rax,8), %xmm0
	vxorpd	%xmm6, %xmm0, %xmm0
	addq	$16, %rdx
	vmovsd	%xmm0, (%rcx,%rbx)
	vmovsd	(%r10,%rax,8), %xmm0
	vmovsd	%xmm0, (%rcx,%r15)
	vmovsd	(%rsi,%rax,8), %xmm0
	incq	%rax
	vxorpd	%xmm6, %xmm0, %xmm0
	vmovsd	%xmm0, (%rcx,%r11)
	cmpl	%eax, %r12d
	jg	.L36
.L38:
	movq	96(%rsp), %rdi
	movq	104(%rsp), %rax
	addl	$2, 176(%rsp)
	movl	176(%rsp), %esi
	addq	152(%rsp), %rdi
	addq	%rax, 168(%rsp)
	addq	%rax, %r11
	addq	%rax, %r15
	addq	%rax, %rbx
	subq	%rax, %r13
	movq	%rdi, 160(%rsp)
	movq	88(%rsp), %rdi
	addq	%rdi, %r14
	subq	%rdi, %r9
	addq	%rdi, %r10
	movl	112(%rsp), %edi
	addl	%edi, 144(%rsp)
	cmpl	116(%rsp), %esi
	jne	.L39
	movq	120(%rsp), %rsi
	movq	64(%rsp), %rbx
	movq	152(%rsp), %r13
.L31:
	movl	116(%rsp), %r11d
	movl	188(%rsp), %r9d
	movl	56(%rsp), %r8d
	vmovq	.LC5(%rip), %xmm9
	vbroadcastsd	.LC8(%rip), %ymm4
	vmovapd	%xmm9, %xmm10
	movl	%r12d, 176(%rsp)
	leal	-1(%r11), %eax
	subl	%r11d, %r9d
	addl	%r11d, %r8d
	imull	%r12d, %eax
	imull	%r12d, %r9d
	imull	%r12d, %r8d
	movslq	%eax, %r10
	addl	%r12d, %eax
	movslq	%eax, %rdi
	movl	%r12d, %eax
	movslq	%r9d, %r9
	negl	%eax
	movslq	%r8d, %r8
	cltq
	vmovq	%rax, %xmm11
	movl	%r12d, %eax
	shrl	$2, %eax
	leal	-1(%rax), %ecx
	movl	%r12d, %eax
	andl	$-4, %eax
	incq	%rcx
	leal	1(%rax), %edx
	vmovd	%eax, %xmm6
	movl	%eax, 168(%rsp)
	addl	$2, %eax
	vmovd	%eax, %xmm2
	movl	%eax, 152(%rsp)
	movq	32(%rsp), %rax
	vmovd	%edx, %xmm3
	movl	%edx, 160(%rsp)
	salq	$5, %rcx
	salq	$3, %rax
	movq	%rax, 120(%rsp)
	.p2align 4
	.p2align 3
.L45:
	movq	%r8, %rax
	movq	%rdi, %rdx
	salq	$4, %rax
	salq	$4, %rdx
	movq	%rax, %r15
	leaq	16(%rdx), %r12
	subq	%r12, %r15
	cmpq	$32, %r15
	jbe	.L40
	cmpl	$4, 184(%rsp)
	jbe	.L40
	leaq	(%rsi,%rax), %r15
	leaq	(%rbx,%r10,8), %r14
	addq	%rsi, %rdx
	leaq	(%rbx,%r9,8), %r12
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L41:
	vmovupd	(%r14,%rax), %xmm0
	vmovupd	(%r14,%rax), %ymm7
	vmovlpd	%xmm0, (%rdx,%rax,2)
	vmovhpd	%xmm0, 16(%rdx,%rax,2)
	vextractf64x2	$0x1, %ymm7, %xmm0
	vmovlpd	%xmm0, 32(%rdx,%rax,2)
	vmovhpd	%xmm0, 48(%rdx,%rax,2)
	vxorpd	(%r12,%rax), %ymm4, %ymm0
	vmovlpd	%xmm0, (%r15,%rax,2)
	vmovhpd	%xmm0, 16(%r15,%rax,2)
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmovlpd	%xmm0, 32(%r15,%rax,2)
	vmovhpd	%xmm0, 48(%r15,%rax,2)
	addq	$32, %rax
	cmpq	%rax, %rcx
	jne	.L41
	movl	176(%rsp), %r15d
	movl	168(%rsp), %eax
	cmpl	%eax, %r15d
	je	.L44
	vmovq	%xmm6, %rax
	vmovq	%xmm6, %rdx
	addq	%rdi, %rax
	addq	%r10, %rdx
	salq	$4, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vmovq	%xmm6, %rdx
	vmovsd	%xmm0, (%rsi,%rax)
	vmovq	%xmm6, %rax
	addq	%r9, %rdx
	addq	%r8, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vxorpd	%xmm10, %xmm0, %xmm0
	salq	$4, %rax
	vmovsd	%xmm0, (%rsi,%rax)
	movl	160(%rsp), %eax
	cmpl	%eax, %r15d
	jle	.L44
	vmovq	%xmm3, %rax
	vmovq	%xmm3, %rdx
	addq	%rdi, %rax
	addq	%r10, %rdx
	salq	$4, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vmovq	%xmm3, %rdx
	vmovsd	%xmm0, (%rsi,%rax)
	vmovq	%xmm3, %rax
	addq	%r9, %rdx
	addq	%r8, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vxorpd	%xmm10, %xmm0, %xmm0
	salq	$4, %rax
	vmovsd	%xmm0, (%rsi,%rax)
	movl	152(%rsp), %eax
	cmpl	%eax, %r15d
	jle	.L44
	vmovq	%xmm2, %rax
	vmovq	%xmm2, %rdx
	addq	%rdi, %rax
	addq	%r10, %rdx
	salq	$4, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vmovq	%xmm2, %rdx
	vmovsd	%xmm0, (%rsi,%rax)
	vmovq	%xmm2, %rax
	addq	%r9, %rdx
	addq	%r8, %rax
	vmovsd	(%rbx,%rdx,8), %xmm0
	vxorpd	%xmm10, %xmm0, %xmm0
	salq	$4, %rax
	vmovsd	%xmm0, (%rsi,%rax)
.L44:
	vmovq	%xmm11, %rax
	incl	%r11d
	addq	%r13, %r10
	addq	%r13, %rdi
	addq	%rax, %r9
	addq	%r13, %r8
	cmpl	%r11d, 188(%rsp)
	jge	.L45
	movl	176(%rsp), %r12d
	movq	128(%rsp), %rdi
	vmovsd	%xmm5, 176(%rsp)
	vzeroupper
	call	block_fft_forward
	movq	136(%rsp), %rax
	movl	184(%rsp), %esi
	movq	%r13, %r9
	vmovsd	176(%rsp), %xmm5
	vmovdqa	.LC7(%rip), %ymm1
	movl	$1, %r8d
	salq	$4, %r9
	vbroadcastsd	%xmm5, %ymm2
	movq	64(%rax), %r14
	movl	%esi, %eax
	andl	$-4, %esi
	movl	%esi, %r10d
	xorl	%esi, %esi
	shrl	$2, %eax
	leal	-1(%rax), %edi
	movq	%rsi, %rcx
	addq	%r13, %rsi
	incq	%rdi
	leaq	8(%r14,%r9), %rdx
	movq	%r14, %r11
	salq	$5, %rdi
	cmpl	$3, 184(%rsp)
	jbe	.L56
	.p2align 4
	.p2align 3
.L84:
	leaq	(%rbx,%rcx,8), %r14
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L50:
	vmovupd	(%rdx,%rax,2), %ymm0
	vpermt2pd	32(%rdx,%rax,2), %ymm1, %ymm0
	vmulpd	%ymm2, %ymm0, %ymm0
	vmovupd	%ymm0, (%r14,%rax)
	addq	$32, %rax
	cmpq	%rax, %rdi
	jne	.L50
	movl	%r10d, %eax
.L49:
	movslq	%eax, %r14
	leaq	(%r14,%rcx), %r15
	addq	%rsi, %r14
	salq	$4, %r14
	vmulsd	8(%r11,%r14), %xmm5, %xmm4
	leal	1(%rax), %r14d
	vmovsd	%xmm4, (%rbx,%r15,8)
	cmpl	%r14d, %r12d
	jle	.L53
	movslq	%r14d, %r14
	leaq	(%rcx,%r14), %r15
	addq	%rsi, %r14
	salq	$4, %r14
	vmulsd	8(%r11,%r14), %xmm5, %xmm4
	leal	2(%rax), %r14d
	vmovsd	%xmm4, (%rbx,%r15,8)
	cmpl	%r14d, %r12d
	jle	.L53
	movslq	%r14d, %r14
	addl	$3, %eax
	leaq	(%r14,%rcx), %r15
	addq	%rsi, %r14
	salq	$4, %r14
	vmulsd	8(%r11,%r14), %xmm5, %xmm4
	vmovsd	%xmm4, (%rbx,%r15,8)
	cmpl	%r12d, %eax
	jge	.L53
	cltq
	addq	%rax, %rcx
	addq	%rsi, %rax
	salq	$4, %rax
	vmulsd	8(%r11,%rax), %xmm5, %xmm0
	vmovsd	%xmm0, (%rbx,%rcx,8)
.L53:
	leal	1(%r8), %eax
	addq	%r9, %rdx
	cmpl	%r8d, 188(%rsp)
	je	.L83
	movq	%rsi, %rcx
	addq	%r13, %rsi
	cmpl	$3, 184(%rsp)
	movl	%eax, %r8d
	ja	.L84
.L56:
	xorl	%eax, %eax
	jmp	.L49
.L30:
	movq	%rax, %rdi
	vzeroupper
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	jmp	block_fft_forward
	.p2align 4
	.p2align 3
.L40:
	.cfi_restore_state
	leaq	(%rbx,%r10,8), %r12
	leaq	(%rsi,%rdx), %r14
	movq	%r13, 144(%rsp)
	leaq	(%rbx,%r9,8), %rdx
	movq	%rbx, %r13
	movq	%rdi, %rbx
	movq	%rcx, %rdi
	movq	120(%rsp), %rcx
	addq	%rsi, %rax
	xorl	%r15d, %r15d
	.p2align 4
	.p2align 3
.L43:
	vmovsd	(%r12,%r15), %xmm0
	vmovsd	%xmm0, (%r14,%r15,2)
	vmovsd	(%rdx,%r15), %xmm0
	vxorpd	%xmm9, %xmm0, %xmm0
	vmovsd	%xmm0, (%rax,%r15,2)
	addq	$8, %r15
	cmpq	%rcx, %r15
	jne	.L43
	movq	%rdi, %rcx
	movq	%rbx, %rdi
	movq	%r13, %rbx
	movq	144(%rsp), %r13
	jmp	.L44
	.p2align 4
	.p2align 3
.L83:
	vzeroupper
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
.L34:
	.cfi_restore_state
	movq	120(%rsp), %rsi
	movq	16(%rsp), %rax
	movq	%r13, 160(%rsp)
	movq	%rbx, %r13
	leaq	(%r9,%rax), %r8
	leaq	(%rsi,%rbx), %rdi
	leaq	(%rsi,%r11), %rdx
	leaq	(%rsi,%r15), %rcx
	movq	%r11, %rbx
	movq	24(%rsp), %rsi
	movq	%r10, %r11
	movq	%r9, %r10
	movq	168(%rsp), %r9
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L37:
	vmovsd	(%r14,%rax), %xmm0
	vmovsd	%xmm0, (%r9,%rax,2)
	vmovsd	(%r10,%rax), %xmm0
	vxorpd	%xmm7, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdi,%rax,2)
	vmovsd	(%r11,%rax), %xmm0
	vmovsd	%xmm0, (%rcx,%rax,2)
	vmovsd	(%r8,%rax), %xmm0
	vxorpd	%xmm7, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%rax,2)
	addq	$8, %rax
	cmpq	%rsi, %rax
	jne	.L37
	movq	%r10, %r9
	movq	%r11, %r10
	movq	%rbx, %r11
	movq	%r13, %rbx
	movq	160(%rsp), %r13
	jmp	.L38
.L54:
	xorl	%eax, %eax
	jmp	.L25
.L55:
	leal	-1(%r12), %eax
	movslq	%r12d, %r13
	movl	$1, 116(%rsp)
	movl	%eax, 184(%rsp)
	movl	%r12d, %eax
	movq	%rax, 32(%rsp)
	jmp	.L31
	.cfi_endproc
.LFE11:
	.size	block_fst_apply, .-block_fst_apply
	.p2align 4
	.globl	block_fst_direct_ortho
	.type	block_fst_direct_ortho, @function
block_fst_direct_ortho:
.LFB12:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-64, %rsp
	vxorpd	%xmm4, %xmm4, %xmm4
	pushq	-8(%r10)
	pushq	%rbp
	movq	%rsp, %rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	pushq	%rbx
	subq	$320, %rsp
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movq	%rdx, -240(%rbp)
	leal	1(%rsi), %edx
	vmovsd	.LC2(%rip), %xmm1
	vcvtsi2sdl	%edx, %xmm4, %xmm0
	movl	%edi, -308(%rbp)
	vdivsd	%xmm0, %xmm1, %xmm1
	movl	%esi, -276(%rbp)
	vsqrtsd	%xmm1, %xmm1, %xmm4
	vmovsd	%xmm4, -344(%rbp)
	testl	%esi, %esi
	jle	.L103
	vmovsd	.LC10(%rip), %xmm1
	vdivsd	%xmm0, %xmm1, %xmm7
	vmovsd	%xmm7, -368(%rbp)
	testl	%edi, %edi
	jle	.L103
	vbroadcastsd	%xmm7, %ymm0
	movq	%rcx, %rax
	movslq	%edi, %rdx
	movl	%esi, %ecx
	vmulpd	.LC13(%rip){1to4}, %ymm0, %ymm4
	movq	%rax, -320(%rbp)
	leal	-1(%rsi), %eax
	leaq	0(,%rdx,8), %r15
	movq	%rdx, -288(%rbp)
	movl	%eax, -312(%rbp)
	movq	%rdx, %rax
	vmovddup	%xmm7, %xmm0
	salq	$6, %rdx
	movl	$0, -224(%rbp)
	vmovapd	%ymm4, -272(%rbp)
	vmulpd	.LC13(%rip){1to2}, %xmm0, %xmm4
	movq	%rdx, -216(%rbp)
	movq	%rax, %rdx
	movl	%esi, %edi
	leaq	(%rax,%rax,2), %rax
	shrl	$3, %ecx
	vmovapd	%xmm4, -336(%rbp)
	salq	$5, %rdx
	movl	%ecx, -220(%rbp)
	andl	$-8, %edi
	leaq	0(,%rax,8), %r14
	movq	%rdx, -360(%rbp)
	movl	%edi, -348(%rbp)
	.p2align 4
	.p2align 3
.L88:
	movq	-240(%rbp), %rax
	vmovsd	-368(%rbp), %xmm4
	incl	-224(%rbp)
	vmulsd	.LC13(%rip), %xmm4, %xmm4
	movq	$0, -232(%rbp)
	vmovsd	%xmm4, -304(%rbp)
	movq	%rax, -296(%rbp)
	.p2align 4
	.p2align 3
.L96:
	movl	-232(%rbp), %eax
	cmpl	$6, -312(%rbp)
	movl	%eax, -280(%rbp)
	jbe	.L97
	vpbroadcastd	-224(%rbp), %ymm3
	movq	-360(%rbp), %rbx
	movq	-296(%rbp), %r12
	xorl	%r13d, %r13d
	movq	%r14, %rax
	movl	%r13d, %r14d
	vxorpd	%xmm4, %xmm4, %xmm4
	movq	%rax, %r13
	addq	%r12, %rbx
	vmovdqa	%ymm3, -208(%rbp)
	vmovdqa	.LC9(%rip), %ymm3
	.p2align 4
	.p2align 3
.L92:
	vmovdqa	%ymm3, %ymm0
	vpaddd	.LC15(%rip), %ymm0, %ymm0
	vpmulld	-208(%rbp), %ymm0, %ymm0
	vmovapd	-272(%rbp), %ymm7
	vmovsd	(%r12,%r15,2), %xmm1
	vmovsd	(%r12), %xmm2
	vmovhpd	(%r12,%r13), %xmm1, %xmm1
	vpaddd	.LC14(%rip), %ymm3, %ymm3
	vmovhpd	(%r12,%r15), %xmm2, %xmm2
	vmovapd	%ymm4, -176(%rbp)
	incl	%r14d
	vinsertf128	$0x1, %xmm1, %ymm2, %ymm5
	vmovsd	(%rbx,%r15,2), %xmm1
	vmovsd	(%rbx), %xmm2
	vmovhpd	(%rbx,%r13), %xmm1, %xmm1
	vmovhpd	(%rbx,%r15), %xmm2, %xmm2
	vmovapd	%ymm5, -80(%rbp)
	vinsertf128	$0x1, %xmm1, %ymm2, %ymm6
	vmovapd	%ymm6, -112(%rbp)
	vmovdqa	%ymm3, -144(%rbp)
	vcvtdq2pd	%xmm0, %ymm1
	vextracti128	$0x1, %ymm0, %xmm0
	vcvtdq2pd	%xmm0, %ymm0
	vmulpd	%ymm7, %ymm1, %ymm1
	vmulpd	%ymm7, %ymm0, %ymm0
	vinsertf64x4	$0x1, %ymm0, %zmm1, %zmm0
	call	_ZGVeN8v_sin
	movq	-216(%rbp), %rax
	vmovapd	-176(%rbp), %ymm4
	vmovdqa	-144(%rbp), %ymm3
	vmovapd	%ymm0, %ymm1
	vextractf64x4	$0x1, %zmm0, %ymm0
	vmulpd	-112(%rbp), %ymm0, %ymm0
	vfmadd231pd	-80(%rbp), %ymm1, %ymm0
	addq	%rax, %r12
	addq	%rax, %rbx
	cmpl	%r14d, -220(%rbp)
	vaddpd	%ymm0, %ymm4, %ymm4
	jne	.L92
	movq	%r13, %r14
	movl	-348(%rbp), %r13d
	movl	-276(%rbp), %edx
	vextractf64x2	$0x1, %ymm4, %xmm0
	vaddpd	%xmm4, %xmm0, %xmm0
	vunpckhpd	%xmm0, %xmm0, %xmm1
	movl	%r13d, %eax
	vaddpd	%xmm0, %xmm1, %xmm1
	cmpl	%edx, %eax
	je	.L89
	movl	%edx, %r12d
.L95:
	subl	%eax, %r12d
	leal	-1(%r12), %esi
	cmpl	$2, %esi
	jbe	.L90
	imulq	-288(%rbp), %rax
	movq	-240(%rbp), %rcx
	vpbroadcastd	-224(%rbp), %xmm2
	vmovsd	%xmm1, -144(%rbp)
	vmovapd	-336(%rbp), %xmm6
	addq	-232(%rbp), %rax
	leaq	(%rcx,%rax,8), %rax
	leaq	(%rax,%r15), %rsi
	vmovsd	(%rax), %xmm0
	vmovhpd	(%rax,%r15), %xmm0, %xmm7
	vmovsd	(%rsi,%r15), %xmm0
	vmovhpd	(%rsi,%r15,2), %xmm0, %xmm5
	vpbroadcastd	%r13d, %xmm0
	vpaddd	.LC11(%rip), %xmm0, %xmm0
	vmovapd	%xmm7, -80(%rbp)
	vmovapd	%xmm5, -112(%rbp)
	vpmulld	%xmm2, %xmm0, %xmm0
	vcvtdq2pd	%xmm0, %xmm2
	vpshufd	$238, %xmm0, %xmm0
	vcvtdq2pd	%xmm0, %xmm0
	vmulpd	%xmm6, %xmm2, %xmm2
	vmulpd	%xmm6, %xmm0, %xmm0
	vinsertf128	$0x1, %xmm0, %ymm2, %ymm0
	call	_ZGVdN4v_sin
	movl	%r12d, %eax
	vmovsd	-144(%rbp), %xmm1
	vmovapd	%xmm0, %xmm2
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmulpd	-112(%rbp), %xmm0, %xmm0
	andl	$-4, %eax
	vfmadd132pd	-80(%rbp), %xmm0, %xmm2
	vunpckhpd	%xmm2, %xmm2, %xmm0
	addl	%eax, %r13d
	vaddpd	%xmm2, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm1, %xmm1
	cmpl	%eax, %r12d
	je	.L89
.L90:
	movl	-308(%rbp), %edi
	movl	-224(%rbp), %ebx
	leal	1(%r13), %r12d
	vxorpd	%xmm3, %xmm3, %xmm3
	vmovsd	%xmm1, -112(%rbp)
	imull	%r13d, %edi
	imull	%r12d, %ebx
	movl	%edi, -80(%rbp)
	vcvtsi2sdl	%ebx, %xmm3, %xmm0
	vmulsd	-304(%rbp), %xmm0, %xmm0
	vzeroupper
	call	sin
	movl	-280(%rbp), %ecx
	movl	-80(%rbp), %edi
	vmovsd	-112(%rbp), %xmm1
	movq	-240(%rbp), %r8
	leal	(%rdi,%rcx), %esi
	movslq	%esi, %rsi
	vfmadd231sd	(%r8,%rsi,8), %xmm0, %xmm1
	cmpl	-276(%rbp), %r12d
	jge	.L89
	movl	-308(%rbp), %eax
	addl	-224(%rbp), %ebx
	vmovsd	%xmm1, -80(%rbp)
	vxorpd	%xmm3, %xmm3, %xmm3
	leal	(%rdi,%rax), %r12d
	vcvtsi2sdl	%ebx, %xmm3, %xmm0
	vmulsd	-304(%rbp), %xmm0, %xmm0
	call	sin
	movl	-280(%rbp), %ecx
	movq	-240(%rbp), %r8
	vmovsd	-80(%rbp), %xmm1
	leal	(%rcx,%r12), %esi
	leal	2(%r13), %ecx
	movslq	%esi, %rsi
	vfmadd231sd	(%r8,%rsi,8), %xmm0, %xmm1
	cmpl	%ecx, -276(%rbp)
	jle	.L89
	movl	-224(%rbp), %edx
	vmovsd	%xmm1, -80(%rbp)
	vxorpd	%xmm3, %xmm3, %xmm3
	leal	(%rbx,%rdx), %eax
	vcvtsi2sdl	%eax, %xmm3, %xmm0
	vmulsd	-304(%rbp), %xmm0, %xmm0
	call	sin
	addl	-308(%rbp), %r12d
	addl	-280(%rbp), %r12d
	movq	-240(%rbp), %r8
	vmovsd	-80(%rbp), %xmm1
	movslq	%r12d, %r12
	vfmadd231sd	(%r8,%r12,8), %xmm0, %xmm1
.L89:
	movq	-232(%rbp), %rax
	movq	-320(%rbp), %rdi
	vmulsd	-344(%rbp), %xmm1, %xmm1
	addq	$8, -296(%rbp)
	vmovsd	%xmm1, (%rdi,%rax,8)
	incq	%rax
	movq	%rax, -232(%rbp)
	cmpq	%rax, -288(%rbp)
	jne	.L96
	movl	-224(%rbp), %edi
	addq	%r15, -320(%rbp)
	cmpl	%edi, -276(%rbp)
	jne	.L88
	vzeroupper
.L103:
	addq	$320, %rsp
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.p2align 4
	.p2align 3
.L97:
	.cfi_restore_state
	movl	-276(%rbp), %r12d
	xorl	%eax, %eax
	xorl	%r13d, %r13d
	vxorpd	%xmm1, %xmm1, %xmm1
	jmp	.L95
	.cfi_endproc
.LFE12:
	.size	block_fst_direct_ortho, .-block_fst_direct_ortho
	.p2align 4
	.globl	block_fst_build_matrix
	.type	block_fst_build_matrix, @function
block_fst_build_matrix:
.LFB13:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-64, %rsp
	pushq	-8(%r10)
	pushq	%rbp
	movq	%rsp, %rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x58,0x6
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	.cfi_escape 0x10,0xd,0x2,0x76,0x68
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	pushq	%rbx
	subq	$256, %rsp
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	cmpq	$0, 72(%rdi)
	movl	4(%rdi), %r13d
	je	.L121
.L106:
	xorl	%eax, %eax
.L105:
	addq	$256, %rsp
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.p2align 4
	.p2align 3
.L121:
	.cfi_restore_state
	movq	%rdi, %rbx
	movslq	%r13d, %rdi
	movq	%rdi, -224(%rbp)
	movq	%rdi, %r15
	imulq	%rdi, %rdi
	salq	$3, %rdi
	call	malloc
	movq	%rax, %rsi
	movq	%rax, -192(%rbp)
	movq	%rax, 72(%rbx)
	testq	%rax, %rax
	je	.L115
	testl	%r13d, %r13d
	jle	.L106
	leal	1(%r13), %eax
	vxorpd	%xmm3, %xmm3, %xmm3
	vmovsd	.LC2(%rip), %xmm0
	xorl	%r12d, %r12d
	vcvtsi2sdl	%eax, %xmm3, %xmm1
	leaq	0(,%r15,8), %rax
	vdivsd	%xmm1, %xmm0, %xmm0
	vsqrtsd	%xmm0, %xmm0, %xmm3
	vmovsd	.LC10(%rip), %xmm0
	vdivsd	%xmm1, %xmm0, %xmm0
	vbroadcastsd	%xmm0, %ymm1
	vmovsd	%xmm3, -184(%rbp)
	movq	%rax, -256(%rbp)
	vmulpd	.LC13(%rip){1to4}, %ymm1, %ymm3
	leal	-1(%r13), %eax
	xorl	%r15d, %r15d
	movl	%eax, -244(%rbp)
	movl	%r13d, %eax
	movq	%rsi, -208(%rbp)
	movq	$0, -200(%rbp)
	shrl	$3, %eax
	vmovapd	%ymm3, -176(%rbp)
	vmulsd	.LC13(%rip), %xmm0, %xmm3
	vmovddup	%xmm0, %xmm0
	vmovsd	%xmm3, -216(%rbp)
	vmulpd	.LC13(%rip){1to2}, %xmm0, %xmm3
	decl	%eax
	movq	%rax, -264(%rbp)
	movl	%r13d, %eax
	andl	$-8, %eax
	vmovapd	%xmm3, -240(%rbp)
	movl	%eax, -248(%rbp)
	.p2align 4
	.p2align 3
.L108:
	incl	%r12d
	cmpl	$6, -244(%rbp)
	jbe	.L116
	movq	-264(%rbp), %rax
	vpbroadcastd	%r12d, %ymm3
	movq	-208(%rbp), %r14
	vmovdqa	.LC9(%rip), %ymm2
	vmovdqa	%ymm3, -112(%rbp)
	vbroadcastsd	-184(%rbp), %ymm3
	leaq	1(%rax), %rbx
	salq	$6, %rbx
	vmovapd	%ymm3, -144(%rbp)
	addq	%r14, %rbx
	.p2align 4
	.p2align 3
.L112:
	vmovdqa	%ymm2, %ymm0
	vpaddd	.LC15(%rip), %ymm0, %ymm0
	vpmulld	-112(%rbp), %ymm0, %ymm0
	vmovapd	-176(%rbp), %ymm4
	vpaddd	.LC14(%rip), %ymm2, %ymm2
	addq	$64, %r14
	vmovdqa	%ymm2, -80(%rbp)
	vcvtdq2pd	%xmm0, %ymm1
	vextracti128	$0x1, %ymm0, %xmm0
	vcvtdq2pd	%xmm0, %ymm0
	vmulpd	%ymm4, %ymm1, %ymm1
	vmulpd	%ymm4, %ymm0, %ymm0
	vinsertf64x4	$0x1, %ymm0, %zmm1, %zmm0
	call	_ZGVeN8v_sin
	vmovapd	-144(%rbp), %ymm5
	vmovdqa	-80(%rbp), %ymm2
	vmulpd	%ymm5, %ymm0, %ymm1
	vextractf64x4	$0x1, %zmm0, %ymm0
	vmulpd	%ymm5, %ymm0, %ymm0
	vmovupd	%ymm1, -64(%r14)
	vmovupd	%ymm0, -32(%r14)
	cmpq	%rbx, %r14
	jne	.L112
	movl	-248(%rbp), %ebx
	movl	%ebx, %eax
	cmpl	%r13d, %eax
	je	.L109
.L114:
	movl	%r13d, %r14d
	subl	%eax, %r14d
	leal	-1(%r14), %ecx
	cmpl	$2, %ecx
	jbe	.L110
	vpbroadcastd	%ebx, %xmm0
	vpaddd	.LC11(%rip), %xmm0, %xmm0
	vpbroadcastd	%r12d, %xmm1
	vmovapd	-240(%rbp), %xmm7
	movq	-192(%rbp), %rsi
	addq	-200(%rbp), %rax
	vpmulld	%xmm1, %xmm0, %xmm0
	leaq	(%rsi,%rax,8), %rax
	movq	%rax, -80(%rbp)
	vcvtdq2pd	%xmm0, %xmm1
	vpshufd	$238, %xmm0, %xmm0
	vcvtdq2pd	%xmm0, %xmm0
	vmulpd	%xmm7, %xmm1, %xmm1
	vmulpd	%xmm7, %xmm0, %xmm0
	vinsertf128	$0x1, %xmm0, %ymm1, %ymm0
	call	_ZGVdN4v_sin
	movq	-80(%rbp), %rax
	vmovddup	-184(%rbp), %xmm1
	vmulpd	%xmm1, %xmm0, %xmm2
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmulpd	%xmm1, %xmm0, %xmm0
	vmovupd	%xmm2, (%rax)
	vmovupd	%xmm0, 16(%rax)
	movl	%r14d, %eax
	andl	$-4, %eax
	addl	%eax, %ebx
	cmpl	%eax, %r14d
	je	.L109
.L110:
	leal	1(%rbx), %r14d
	vxorpd	%xmm3, %xmm3, %xmm3
	movl	%r14d, %eax
	imull	%r12d, %eax
	vcvtsi2sdl	%eax, %xmm3, %xmm0
	movl	%eax, -80(%rbp)
	vmulsd	-216(%rbp), %xmm0, %xmm0
	vzeroupper
	call	sin
	movq	-192(%rbp), %rax
	leal	(%r15,%rbx), %edx
	vmulsd	-184(%rbp), %xmm0, %xmm0
	movslq	%edx, %rdx
	vmovsd	%xmm0, (%rax,%rdx,8)
	cmpl	%r13d, %r14d
	jge	.L109
	movl	-80(%rbp), %eax
	vxorpd	%xmm3, %xmm3, %xmm3
	addl	$2, %ebx
	leal	(%rax,%r12), %edx
	vcvtsi2sdl	%edx, %xmm3, %xmm0
	movl	%edx, -80(%rbp)
	vmulsd	-216(%rbp), %xmm0, %xmm0
	call	sin
	leal	(%r14,%r15), %eax
	movq	-192(%rbp), %r14
	vmulsd	-184(%rbp), %xmm0, %xmm0
	cltq
	vmovsd	%xmm0, (%r14,%rax,8)
	cmpl	%ebx, %r13d
	jle	.L109
	movl	-80(%rbp), %edx
	addl	%r15d, %ebx
	vxorpd	%xmm3, %xmm3, %xmm3
	movslq	%ebx, %rbx
	leal	(%rdx,%r12), %eax
	vcvtsi2sdl	%eax, %xmm3, %xmm0
	vmulsd	-216(%rbp), %xmm0, %xmm0
	call	sin
	vmulsd	-184(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, (%r14,%rbx,8)
.L109:
	movq	-256(%rbp), %rcx
	movq	-224(%rbp), %rdi
	addl	%r13d, %r15d
	addq	%rcx, -208(%rbp)
	addq	%rdi, -200(%rbp)
	cmpl	%r12d, %r13d
	jne	.L108
	vzeroupper
	jmp	.L106
	.p2align 4
	.p2align 3
.L116:
	xorl	%eax, %eax
	xorl	%ebx, %ebx
	jmp	.L114
.L115:
	movl	$1, %eax
	jmp	.L105
	.cfi_endproc
.LFE13:
	.size	block_fst_build_matrix, .-block_fst_build_matrix
	.section	.rodata.str1.8
	.align 8
.LC19:
	.string	"block_fst_apply_slow: could not build S\n"
	.text
	.p2align 4
	.globl	block_fst_apply_slow
	.type	block_fst_apply_slow, @function
block_fst_apply_slow:
.LFB14:
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
	movq	%rdi, %r13
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rsi, %rbp
	subq	$88, %rsp
	.cfi_def_cfa_offset 144
	movl	(%rdi), %r12d
	movl	4(%rdi), %ebx
	call	block_fst_build_matrix
	testl	%eax, %eax
	jne	.L123
	movq	80(%r13), %rax
	movq	%rax, 72(%rsp)
	testl	%ebx, %ebx
	jle	.L143
	testl	%r12d, %r12d
	jle	.L143
	movq	%rax, %rdi
	movslq	%r12d, %rax
	movq	72(%r13), %r15
	xorl	%r11d, %r11d
	leaq	0(,%rax,8), %r14
	movq	%rax, 16(%rsp)
	movq	%rdi, 8(%rsp)
	movslq	%ebx, %rdi
	salq	$4, %rax
	movq	%rdi, 64(%rsp)
	movl	$0, 44(%rsp)
	movq	$0, 48(%rsp)
	movq	%rax, %rsi
	movl	%ebx, %eax
	shrl	%eax
	salq	$3, %rdi
	decl	%eax
	vmovq	%rdi, %xmm4
	leal	-1(%rbx), %edi
	incq	%rax
	movl	%edi, 28(%rsp)
	movl	%ebx, %edi
	salq	$4, %rax
	andl	$-2, %edi
	movq	%rax, 56(%rsp)
	movl	%edi, 40(%rsp)
	.p2align 4
	.p2align 3
.L126:
	movq	48(%rsp), %rax
	movq	56(%rsp), %rcx
	movq	%rbp, %r9
	xorl	%edi, %edi
	addq	%r15, %rax
	addq	%rax, %rcx
	movq	%rax, 32(%rsp)
	.p2align 4
	.p2align 3
.L132:
	cmpl	$2, 28(%rsp)
	movl	%edi, %r10d
	jbe	.L133
	movq	32(%rsp), %rdx
	movq	%r9, %rax
	vxorpd	%xmm1, %xmm1, %xmm1
	.p2align 4
	.p2align 3
.L128:
	vmovsd	(%rax), %xmm0
	vmovhpd	(%rax,%r14), %xmm0, %xmm0
	vmulpd	(%rdx), %xmm0, %xmm0
	addq	$16, %rdx
	addq	%rsi, %rax
	vaddpd	%xmm0, %xmm1, %xmm1
	cmpq	%rdx, %rcx
	jne	.L128
	movl	40(%rsp), %eax
	vunpckhpd	%xmm1, %xmm1, %xmm0
	vaddpd	%xmm1, %xmm0, %xmm0
	cmpl	%ebx, %eax
	je	.L127
.L131:
	movl	%r12d, %edx
	leal	(%r11,%rax), %r8d
	imull	%eax, %edx
	movslq	%r8d, %r13
	vmovsd	(%r15,%r13,8), %xmm2
	leal	(%rdx,%r10), %r8d
	movslq	%r8d, %r8
	vfmadd231sd	0(%rbp,%r8,8), %xmm2, %xmm0
	leal	1(%rax), %r8d
	cmpl	%r8d, %ebx
	jle	.L127
	addl	%r11d, %r8d
	addl	%r12d, %edx
	addl	$2, %eax
	movslq	%r8d, %r13
	leal	(%r10,%rdx), %r8d
	movslq	%r8d, %r8
	vmovsd	(%r15,%r13,8), %xmm3
	vfmadd231sd	0(%rbp,%r8,8), %xmm3, %xmm0
	cmpl	%eax, %ebx
	jle	.L127
	addl	%r12d, %edx
	addl	%r11d, %eax
	addl	%r10d, %edx
	cltq
	movslq	%edx, %rdx
	vmovsd	0(%rbp,%rdx,8), %xmm5
	vfmadd231sd	(%r15,%rax,8), %xmm5, %xmm0
.L127:
	movq	8(%rsp), %rax
	addq	$8, %r9
	vmovsd	%xmm0, (%rax,%rdi,8)
	incq	%rdi
	cmpq	%rdi, 16(%rsp)
	jne	.L132
	incl	44(%rsp)
	movl	44(%rsp), %eax
	vmovq	%xmm4, %rcx
	addl	%ebx, %r11d
	addq	%r14, 8(%rsp)
	addq	%rcx, 48(%rsp)
	cmpl	%eax, %ebx
	jne	.L126
.L124:
	movq	16(%rsp), %rdx
	movq	72(%rsp), %rsi
	movq	%rbp, %rdi
	imulq	64(%rsp), %rdx
	addq	$88, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
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
	salq	$3, %rdx
	jmp	memcpy
	.p2align 4
	.p2align 3
.L133:
	.cfi_restore_state
	xorl	%eax, %eax
	vxorpd	%xmm0, %xmm0, %xmm0
	jmp	.L131
.L123:
	movq	stderr(%rip), %rcx
	addq	$88, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	$40, %edx
	movl	$1, %esi
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	movl	$.LC19, %edi
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	jmp	fwrite
.L143:
	.cfi_restore_state
	movslq	%r12d, %rax
	movq	%rax, 16(%rsp)
	movslq	%ebx, %rax
	movq	%rax, 64(%rsp)
	jmp	.L124
	.cfi_endproc
.LFE14:
	.size	block_fst_apply_slow, .-block_fst_apply_slow
	.section	.rodata.str1.8
	.align 8
.LC20:
	.string	"block_fst_apply_matmul: could not build S\n"
	.text
	.p2align 4
	.globl	block_fst_apply_matmul
	.type	block_fst_apply_matmul, @function
block_fst_apply_matmul:
.LFB15:
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
	movq	%rdi, %r13
	movq	%rsi, %r12
	andq	$-32, %rsp
	subq	$64, %rsp
	movl	4(%rdi), %eax
	movl	(%rdi), %ebx
	movl	%eax, %r14d
	movl	%eax, 44(%rsp)
	call	block_fst_build_matrix
	testl	%eax, %eax
	jne	.L145
	movl	%r14d, %eax
	movq	80(%r13), %r8
	imull	%ebx, %eax
	testl	%eax, %eax
	jle	.L149
	cltq
	movq	%r8, %rdi
	xorl	%esi, %esi
	leaq	0(,%rax,8), %rdx
	call	memset
	movq	%rax, %r8
.L149:
	movl	44(%rsp), %eax
	testl	%eax, %eax
	jle	.L172
	movq	72(%r13), %rdx
	testl	%ebx, %ebx
	jle	.L172
	movslq	44(%rsp), %rax
	movl	%ebx, %ecx
	movl	%ebx, %r13d
	movl	$0, 48(%rsp)
	andl	$-4, %r13d
	shrl	$2, %ecx
	leal	-1(%rcx), %r9d
	movq	$-8, %rcx
	movq	%rax, %rdi
	movq	%rax, 24(%rsp)
	incq	%r9
	salq	$3, %rax
	vmovq	%rax, %xmm4
	leal	-1(%rdi), %eax
	movslq	%ebx, %rdi
	movq	%rdi, 56(%rsp)
	salq	$3, %rdi
	salq	$3, %rax
	vmovq	%rdi, %xmm3
	salq	$5, %r9
	leal	-1(%rbx), %edi
	leaq	8(%rdx,%rax), %r14
	subq	%rax, %rcx
	movq	%r8, %rdx
	movl	%edi, 52(%rsp)
	xorl	%edi, %edi
	movq	%rcx, 32(%rsp)
	.p2align 4
	.p2align 3
.L159:
	movq	32(%rsp), %rax
	xorl	%esi, %esi
	leaq	(%rax,%r14), %r11
	.p2align 4
	.p2align 3
.L154:
	cmpl	$2, 52(%rsp)
	vmovsd	(%r11), %xmm0
	jbe	.L160
	leaq	(%r12,%rsi,8), %rcx
	vbroadcastsd	%xmm0, %ymm2
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L152:
	vmovupd	(%rcx,%rax), %ymm1
	vfmadd213pd	(%rdx,%rax), %ymm2, %ymm1
	vmovupd	%ymm1, (%rdx,%rax)
	addq	$32, %rax
	cmpq	%rax, %r9
	jne	.L152
	cmpl	%r13d, %ebx
	je	.L156
	movl	%r13d, %ecx
	movl	%r13d, %eax
.L151:
	movl	%ebx, %r10d
	subl	%ecx, %r10d
	cmpl	$1, %r10d
	je	.L157
	leaq	(%rdi,%rcx), %r15
	addq	%rsi, %rcx
	vmovddup	%xmm0, %xmm1
	leaq	(%r8,%r15,8), %r15
	vmovupd	(%r15), %xmm6
	vfmadd132pd	(%r12,%rcx,8), %xmm6, %xmm1
	movl	%r10d, %ecx
	vmovupd	%xmm1, (%r15)
	andl	$-2, %ecx
	addl	%ecx, %eax
	cmpl	%ecx, %r10d
	je	.L156
.L157:
	cltq
	leaq	(%rax,%rdi), %rcx
	addq	%rsi, %rax
	leaq	(%r8,%rcx,8), %rcx
	vmovsd	(%rcx), %xmm5
	vfmadd132sd	(%r12,%rax,8), %xmm5, %xmm0
	vmovsd	%xmm0, (%rcx)
.L156:
	addq	$8, %r11
	addq	56(%rsp), %rsi
	cmpq	%r11, %r14
	jne	.L154
	vmovq	%xmm4, %rsi
	incl	48(%rsp)
	movl	48(%rsp), %eax
	addq	56(%rsp), %rdi
	addq	%rsi, %r14
	vmovq	%xmm3, %rsi
	addq	%rsi, %rdx
	cmpl	%eax, 44(%rsp)
	jne	.L159
	vzeroupper
.L148:
	movq	56(%rsp), %rdx
	movq	%r12, %rdi
	movq	%r8, %rsi
	imulq	24(%rsp), %rdx
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	salq	$3, %rdx
	jmp	memcpy
.L160:
	.cfi_restore_state
	xorl	%eax, %eax
	xorl	%ecx, %ecx
	jmp	.L151
.L145:
	movq	stderr(%rip), %rcx
	leaq	-40(%rbp), %rsp
	movl	$42, %edx
	movl	$1, %esi
	popq	%rbx
	popq	%r12
	popq	%r13
	movl	$.LC20, %edi
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	jmp	fwrite
.L172:
	.cfi_restore_state
	movslq	%ebx, %rax
	movq	%rax, 56(%rsp)
	movslq	44(%rsp), %rax
	movq	%rax, 24(%rsp)
	jmp	.L148
	.cfi_endproc
.LFE15:
	.size	block_fst_apply_matmul, .-block_fst_apply_matmul
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC2:
	.long	0
	.long	1073741824
	.align 8
.LC3:
	.long	0
	.long	-1075838976
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC5:
	.long	0
	.long	-2147483648
	.long	0
	.long	0
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC7:
	.quad	0
	.quad	2
	.quad	4
	.quad	6
	.set	.LC8,.LC5
	.align 32
.LC9:
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.section	.rodata.cst8
	.align 8
.LC10:
	.long	0
	.long	1072693248
	.section	.rodata.cst16
	.align 16
.LC11:
	.long	1
	.long	2
	.long	3
	.long	4
	.section	.rodata.cst8
	.align 8
.LC13:
	.long	1413754136
	.long	1074340347
	.section	.rodata.cst32
	.align 32
.LC14:
	.long	8
	.long	8
	.long	8
	.long	8
	.long	8
	.long	8
	.long	8
	.long	8
	.align 32
.LC15:
	.long	1
	.long	1
	.long	1
	.long	1
	.long	1
	.long	1
	.long	1
	.long	1
	.ident	"GCC: (GNU) 11.5.0 20240719 (Red Hat 11.5.0-14)"
	.section	.note.GNU-stack,"",@progbits
