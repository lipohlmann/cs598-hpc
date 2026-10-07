	.file	"poisson.c"
	.text
	.p2align 4
	.type	dist_transpose, @function
dist_transpose:
.LFB11:
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
	movq	%rdi, %rax
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$120, %rsp
	.cfi_def_cfa_offset 176
	movq	296(%rax), %r12
	movq	%rdx, 16(%rsp)
	movslq	44(%rax), %rdx
	movq	176(%rsp), %rbx
	movq	184(%rsp), %rdi
	movq	%rsi, 48(%rsp)
	movl	%ecx, 88(%rsp)
	movslq	%ecx, %rcx
	movl	192(%rsp), %r13d
	movq	%r8, 56(%rsp)
	movq	%r9, 64(%rsp)
	movq	%rcx, 32(%rsp)
	movq	%rdx, %r14
	movq	%rbx, 24(%rsp)
	movl	40(%rax), %ebx
	movl	80(%rax), %eax
	movq	%rdi, (%rsp)
	salq	$2, %rdx
	movl	(%r8,%rdx), %ebp
	leaq	(%rdi,%rdx), %rsi
	addq	%r9, %rdx
	movq	%rsi, 72(%rsp)
	movq	%rdx, 80(%rsp)
	movslq	%ebp, %rsi
	movq	%rsi, 8(%rsp)
	testl	%eax, %eax
	jne	.L2
	cmpl	$1, %ebx
	jle	.L3
	leal	-1(%r14), %eax
	leal	-1(%rbx,%r14), %r15d
	movl	%eax, 92(%rsp)
	.p2align 4
	.p2align 3
.L4:
	movl	%r15d, %eax
	movq	24(%rsp), %rcx
	decl	%r15d
	cltd
	idivl	%ebx
	movslq	%edx, %rax
	movl	(%rcx,%rax,4), %edx
	movq	(%rsp), %rcx
	movq	%rax, %rdi
	movslq	(%rcx,%rax,4), %rax
	imull	%ebp, %edx
	movl	%r13d, %ecx
	imulq	8(%rsp), %rax
	sall	$3, %edx
	leaq	(%r12,%rax,8), %rsi
	call	irecv
	cmpl	%r15d, %r14d
	jne	.L4
	leal	1(%r14), %r15d
	leal	(%rbx,%r14), %eax
	movl	%r14d, 96(%rsp)
	movl	%ebp, 100(%rsp)
	movq	%r12, 104(%rsp)
	movq	48(%rsp), %rbp
	movl	%r15d, %r12d
	movq	56(%rsp), %r14
	movl	%r13d, %r15d
	movq	64(%rsp), %r13
	movl	%eax, 40(%rsp)
	.p2align 4
	.p2align 3
.L5:
	movl	%r12d, %eax
	movl	%r15d, %ecx
	incl	%r12d
	cltd
	idivl	%ebx
	movslq	%edx, %rax
	movl	88(%rsp), %edx
	imull	(%r14,%rax,4), %edx
	movq	%rax, %rdi
	movslq	0(%r13,%rax,4), %rax
	imulq	32(%rsp), %rax
	sall	$3, %edx
	leaq	0(%rbp,%rax,8), %rsi
	call	isend
	cmpl	%r12d, 40(%rsp)
	jne	.L5
	movq	72(%rsp), %rax
	movq	16(%rsp), %rcx
	movq	48(%rsp), %rdi
	movl	100(%rsp), %ebp
	movl	96(%rsp), %r14d
	movq	104(%rsp), %r12
	movslq	(%rax), %rax
	imulq	8(%rsp), %rax
	movl	%ebp, %esi
	leaq	(%rcx,%rax,8), %rcx
	movq	80(%rsp), %rax
	movslq	(%rax), %rax
	imulq	32(%rsp), %rax
	leaq	(%rdi,%rax,8), %rdx
	movl	88(%rsp), %edi
	call	transpose_real
	call	msgwait
	movl	92(%rsp), %r13d
	movq	24(%rsp), %r15
	addl	%ebx, %r13d
	.p2align 4
	.p2align 3
.L7:
	movl	%r13d, %eax
	movq	16(%rsp), %rdi
	movl	%ebp, %esi
	decl	%r13d
	cltd
	idivl	%ebx
	movq	(%rsp), %rax
	movslq	%edx, %rcx
	movslq	(%rax,%rcx,4), %rax
	imulq	8(%rsp), %rax
	salq	$3, %rax
	leaq	(%rdi,%rax), %r9
	movl	(%r15,%rcx,4), %edi
	leaq	(%r12,%rax), %rdx
	movq	%r9, %rcx
	call	transpose_real
	cmpl	%r14d, %r13d
	jne	.L7
.L15:
	addq	$120, %rsp
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
.L2:
	.cfi_restore_state
	movq	72(%rsp), %rax
	movq	16(%rsp), %rdi
	movq	%rcx, %rsi
	salq	$3, %rsi
	movq	%rsi, 40(%rsp)
	movslq	(%rax), %rax
	imulq	8(%rsp), %rax
	leaq	(%rdi,%rax,8), %rcx
	movq	80(%rsp), %rax
	movl	88(%rsp), %edi
	movslq	(%rax), %rdx
	imulq	%rsi, %rdx
	addq	48(%rsp), %rdx
	movl	%ebp, %esi
	call	transpose_real
	cmpl	$1, %ebx
	jle	.L15
	movslq	%ebp, %rcx
	leal	(%r14,%rbx), %eax
	leal	1(%r14), %r15d
	movl	%ebx, 92(%rsp)
	salq	$3, %rcx
	movl	%eax, 72(%rsp)
	movq	%r12, 32(%rsp)
	leal	(%rbx,%r14,2), %eax
	movq	%rcx, 8(%rsp)
	movl	%eax, 80(%rsp)
	.p2align 4
	.p2align 3
.L9:
	movl	92(%rsp), %ebx
	movl	%r15d, %eax
	movl	%r13d, %ecx
	cltd
	idivl	%ebx
	movl	80(%rsp), %eax
	subl	%r15d, %eax
	incl	%r15d
	movl	%edx, %r12d
	cltd
	idivl	%ebx
	movq	24(%rsp), %rbx
	movslq	%edx, %rax
	movq	%rax, %rdi
	salq	$2, %rax
	leaq	(%rbx,%rax), %r14
	movq	(%rsp), %rbx
	movl	(%r14), %edx
	addq	%rax, %rbx
	movslq	(%rbx), %rsi
	imull	%ebp, %edx
	imulq	8(%rsp), %rsi
	sall	$3, %edx
	addq	32(%rsp), %rsi
	call	irecv
	movq	56(%rsp), %rsi
	movslq	%r12d, %rcx
	movl	88(%rsp), %edx
	movl	%r12d, %edi
	imull	(%rsi,%rcx,4), %edx
	movq	64(%rsp), %rsi
	movslq	(%rsi,%rcx,4), %rsi
	movl	%r13d, %ecx
	imulq	40(%rsp), %rsi
	sall	$3, %edx
	addq	48(%rsp), %rsi
	call	isend
	call	msgwait
	movslq	(%rbx), %rdx
	movq	16(%rsp), %rax
	imulq	8(%rsp), %rdx
	movl	%ebp, %esi
	movl	(%r14), %edi
	leaq	(%rax,%rdx), %rcx
	addq	32(%rsp), %rdx
	call	transpose_real
	cmpl	%r15d, 72(%rsp)
	jne	.L9
	addq	$120, %rsp
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
.L3:
	.cfi_restore_state
	movq	72(%rsp), %rax
	movq	16(%rsp), %rbx
	movl	88(%rsp), %edi
	movl	%ebp, %esi
	movslq	(%rax), %rax
	imulq	8(%rsp), %rax
	leaq	(%rbx,%rax,8), %rcx
	movq	80(%rsp), %rax
	movq	48(%rsp), %rbx
	movslq	(%rax), %rax
	imulq	32(%rsp), %rax
	leaq	(%rbx,%rax,8), %rdx
	call	transpose_real
	addq	$120, %rsp
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
	jmp	msgwait
	.cfi_endproc
.LFE11:
	.size	dist_transpose, .-dist_transpose
	.p2align 4
	.globl	poisson_plan_free
	.type	poisson_plan_free, @function
poisson_plan_free:
.LFB10:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	leaq	88(%rdi), %rdi
	call	block_fst_plan_free
	leaq	176(%rbx), %rdi
	call	block_fst_plan_free
	movq	48(%rbx), %rdi
	call	free
	movq	56(%rbx), %rdi
	movq	$0, 48(%rbx)
	call	free
	movq	64(%rbx), %rdi
	movq	$0, 56(%rbx)
	call	free
	movq	72(%rbx), %rdi
	movq	$0, 64(%rbx)
	call	free
	movq	264(%rbx), %rdi
	movq	$0, 72(%rbx)
	call	free
	movq	272(%rbx), %rdi
	movq	$0, 264(%rbx)
	call	free
	movq	280(%rbx), %rdi
	movq	$0, 272(%rbx)
	call	free
	movq	288(%rbx), %rdi
	movq	$0, 280(%rbx)
	call	free
	movq	296(%rbx), %rdi
	movq	$0, 288(%rbx)
	call	free
	movq	$0, 296(%rbx)
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE10:
	.size	poisson_plan_free, .-poisson_plan_free
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC2:
	.string	"poisson: need P <= min(nx,ny); P=%d nx=%d ny=%d\n"
	.text
	.p2align 4
	.globl	poisson_plan_init
	.type	poisson_plan_init, @function
poisson_plan_init:
.LFB9:
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
	movl	%edx, %r13d
	subq	$320, %rsp
	.cfi_escape 0x10,0x3,0x2,0x76,0x50
	movl	$336, %edx
	movl	%esi, -80(%rbp)
	xorl	%esi, %esi
	movq	%rdi, %r12
	vmovsd	%xmm0, -144(%rbp)
	vmovsd	%xmm1, -112(%rbp)
	call	memset
	vmovd	-80(%rbp), %xmm5
	vxorpd	%xmm6, %xmm6, %xmm6
	vmovsd	-144(%rbp), %xmm0
	vmovsd	-112(%rbp), %xmm1
	movl	$1, 84(%r12)
	vmovd	%xmm5, %ebx
	vpinsrd	$1, %r13d, %xmm5, %xmm2
	leal	1(%rbx), %eax
	vmovq	%xmm2, (%r12)
	vcvtsi2sdl	%eax, %xmm6, %xmm2
	leal	1(%r13), %eax
	vmovsd	%xmm2, %xmm2, %xmm7
	vmovsd	%xmm2, -176(%rbp)
	vcvtsi2sdl	%eax, %xmm6, %xmm2
	vmovsd	%xmm2, %xmm2, %xmm5
	vmovsd	%xmm2, -208(%rbp)
	vdivsd	%xmm5, %xmm1, %xmm3
	vdivsd	%xmm7, %xmm0, %xmm2
	vunpcklpd	%xmm1, %xmm0, %xmm1
	vunpcklpd	%xmm3, %xmm2, %xmm2
	vinsertf128	$0x1, %xmm2, %ymm1, %ymm0
	vmovupd	%ymm0, 8(%r12)
	vzeroupper
	call	num_ranks
	movl	%eax, %r15d
	movl	%eax, 40(%r12)
	call	msg_rank
	cmpl	%r13d, %ebx
	movl	%eax, %r8d
	movl	%eax, 44(%r12)
	movl	%ebx, %eax
	cmovg	%r13d, %eax
	cmpl	%eax, %r15d
	jle	.L21
	movl	$1, %r14d
	testl	%r8d, %r8d
	je	.L83
.L20:
	addq	$320, %rsp
	movl	%r14d, %eax
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
.L21:
	.cfi_restore_state
	movslq	%r15d, %rdi
	movl	%r8d, -240(%rbp)
	salq	$2, %rdi
	movq	%rdi, -112(%rbp)
	call	malloc
	movq	-112(%rbp), %rdi
	movq	%rax, %r14
	movq	%rax, 48(%r12)
	call	malloc
	movq	-112(%rbp), %rdi
	movq	%rax, 56(%r12)
	movq	%rax, -144(%rbp)
	call	malloc
	movq	-112(%rbp), %rdi
	movq	%rax, %rbx
	movq	%rax, 64(%r12)
	call	malloc
	testq	%rbx, %rbx
	movq	%rax, %rcx
	movq	%rax, 72(%r12)
	sete	%al
	testq	%rcx, %rcx
	sete	%dl
	orb	%dl, %al
	jne	.L33
	movq	-144(%rbp), %rsi
	testq	%r14, %r14
	movl	-240(%rbp), %r8d
	sete	%al
	testq	%rsi, %rsi
	sete	%dl
	orb	%dl, %al
	jne	.L33
	movl	-80(%rbp), %eax
	movl	$0, (%rsi)
	cltd
	idivl	%r15d
	testl	%r15d, %r15d
	jle	.L84
	xorl	%edi, %edi
	testl	%edx, %edx
	setg	%dil
	xorl	%r11d, %r11d
	addl	%eax, %edi
	movl	%edi, (%r14)
	cmpl	$1, %r15d
	je	.L28
	leal	-2(%r15), %edi
	leal	-1(%r15), %r11d
	cmpl	$6, %edi
	jbe	.L65
	movl	%r11d, %r9d
	vmovdqa	.LC1(%rip), %ymm2
	vpbroadcastd	.LC14(%rip), %ymm1
	vpbroadcastd	%edx, %ymm4
	shrl	$3, %r9d
	vpbroadcastd	%eax, %ymm3
	decl	%r9d
	leaq	4(%r14), %rdi
	salq	$5, %r9
	leaq	36(%r14,%r9), %r9
	.p2align 4
	.p2align 3
.L56:
	vmovdqa	%ymm2, %ymm0
	addq	$32, %rdi
	vpaddd	%ymm1, %ymm2, %ymm2
	vpcmpgtd	%ymm0, %ymm4, %ymm0
	vpsubd	%ymm0, %ymm3, %ymm0
	vmovdqu	%ymm0, -32(%rdi)
	cmpq	%rdi, %r9
	jne	.L56
	movl	%r11d, %r9d
	andl	$-8, %r9d
	leal	1(%r9), %edi
	movl	%edi, -112(%rbp)
	cmpl	%r9d, %r11d
	je	.L57
.L55:
	movl	%r15d, %r10d
	subl	%r9d, %r10d
	leal	-1(%r10), %edi
	subl	$2, %r10d
	cmpl	$2, %r10d
	jbe	.L58
	vpbroadcastd	-112(%rbp), %xmm2
	vpaddd	.LC13(%rip), %xmm2, %xmm2
	vpbroadcastd	%edx, %xmm1
	vpbroadcastd	%eax, %xmm0
	incl	%r9d
	vpcmpgtd	%xmm2, %xmm1, %xmm1
	vpsubd	%xmm1, %xmm0, %xmm0
	vmovdqu	%xmm0, (%r14,%r9,4)
	movl	%edi, %r9d
	andl	$-4, %r9d
	addl	%r9d, -112(%rbp)
	cmpl	%r9d, %edi
	je	.L57
.L58:
	movslq	-112(%rbp), %r9
	xorl	%r10d, %r10d
	movq	%r9, %rdi
	salq	$2, %r9
	cmpl	%edi, %edx
	setg	%r10b
	addl	%eax, %r10d
	movl	%r10d, (%r14,%r9)
	leal	1(%rdi), %r10d
	cmpl	%r10d, %r15d
	jle	.L57
	cmpl	%r10d, %edx
	setg	%r10b
	addl	$2, %edi
	movzbl	%r10b, %r10d
	addl	%eax, %r10d
	movl	%r10d, 4(%r14,%r9)
	cmpl	%edi, %r15d
	jle	.L57
	cmpl	%edi, %edx
	setg	%dl
	movzbl	%dl, %edx
	addl	%eax, %edx
	movl	%edx, 8(%r14,%r9)
.L57:
	movl	%r11d, %edi
	xorl	%eax, %eax
	xorl	%edx, %edx
	.p2align 4
	.p2align 3
.L27:
	addl	(%r14,%rax,4), %edx
	movl	%edx, 4(%rsi,%rax,4)
	incq	%rax
	cmpq	%rdi, %rax
	jne	.L27
.L28:
	movl	%r13d, %eax
	movl	$0, (%rcx)
	cltd
	idivl	%r15d
	cmpl	$6, %r11d
	jbe	.L64
	movl	%r15d, %edi
	vmovdqa	.LC0(%rip), %ymm2
	vpbroadcastd	.LC14(%rip), %ymm1
	vpbroadcastd	%edx, %ymm4
	shrl	$3, %edi
	vpbroadcastd	%eax, %ymm3
	salq	$5, %rdi
	movq	%rbx, %rsi
	addq	%rbx, %rdi
	.p2align 4
	.p2align 3
.L51:
	vmovdqa	%ymm2, %ymm0
	addq	$32, %rsi
	vpaddd	%ymm1, %ymm2, %ymm2
	vpcmpgtd	%ymm0, %ymm4, %ymm0
	vpsubd	%ymm0, %ymm3, %ymm0
	vmovdqu	%ymm0, -32(%rsi)
	cmpq	%rsi, %rdi
	jne	.L51
	movl	%r15d, %edi
	andl	$-8, %edi
	movl	%edi, %esi
	cmpl	%edi, %r15d
	je	.L52
.L50:
	movl	%r15d, %r9d
	subl	%edi, %r9d
	leal	-1(%r9), %r10d
	cmpl	$2, %r10d
	jbe	.L53
	vpbroadcastd	%esi, %xmm2
	vpaddd	.LC13(%rip), %xmm2, %xmm2
	vpbroadcastd	%edx, %xmm1
	vpbroadcastd	%eax, %xmm0
	vpcmpgtd	%xmm2, %xmm1, %xmm1
	vpsubd	%xmm1, %xmm0, %xmm0
	vmovdqu	%xmm0, (%rbx,%rdi,4)
	movl	%r9d, %edi
	andl	$-4, %edi
	addl	%edi, %esi
	cmpl	%edi, %r9d
	je	.L52
.L53:
	movslq	%esi, %rdi
	xorl	%r9d, %r9d
	salq	$2, %rdi
	cmpl	%esi, %edx
	setg	%r9b
	addl	%eax, %r9d
	movl	%r9d, (%rbx,%rdi)
	leal	1(%rsi), %r9d
	cmpl	%r15d, %r9d
	jge	.L52
	cmpl	%edx, %r9d
	setl	%r9b
	addl	$2, %esi
	movzbl	%r9b, %r9d
	addl	%eax, %r9d
	movl	%r9d, 4(%rbx,%rdi)
	cmpl	%esi, %r15d
	jle	.L52
	cmpl	%esi, %edx
	setg	%dl
	movzbl	%dl, %edx
	addl	%eax, %edx
	movl	%edx, 8(%rbx,%rdi)
.L52:
	movl	%r15d, %r15d
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L30:
	testq	%rax, %rax
	je	.L29
	movl	-4(%rbx,%rax,4), %edx
	addl	-4(%rcx,%rax,4), %edx
	movl	%edx, (%rcx,%rax,4)
.L29:
	incq	%rax
	cmpq	%rax, %r15
	jne	.L30
	vzeroupper
.L49:
	movslq	%r8d, %rbx
	leaq	176(%r12), %r15
	movl	%r13d, %edx
	salq	$2, %rbx
	movl	(%r14,%rbx), %esi
	movq	%r15, %rdi
	call	block_fst_plan_init
	leaq	88(%r12), %rdi
	testl	%eax, %eax
	jne	.L82
	movq	64(%r12), %rax
	movl	-80(%rbp), %edx
	movq	%rdi, -112(%rbp)
	movl	(%rax,%rbx), %esi
	call	block_fst_plan_init
	movq	-112(%rbp), %rdi
	testl	%eax, %eax
	movl	%eax, %r14d
	jne	.L82
	movq	48(%r12), %rax
	movslq	%r13d, %rdx
	movslq	-80(%rbp), %rdi
	movq	%rdx, -144(%rbp)
	movslq	(%rax,%rbx), %rax
	imulq	%rdx, %rax
	movq	%rax, -112(%rbp)
	movq	64(%r12), %rax
	movslq	(%rax,%rbx), %rbx
	imulq	%rdi, %rbx
	salq	$3, %rdi
	call	malloc
	movq	-144(%rbp), %rdx
	movq	%rax, %r15
	movq	%rax, 264(%r12)
	leaq	0(,%rdx,8), %rdi
	call	malloc
	movq	%rax, -280(%rbp)
	movq	%rax, 272(%r12)
	movq	-112(%rbp), %rax
	leaq	0(,%rax,8), %rdi
	call	malloc
	leaq	0(,%rbx,8), %rdi
	movq	%rax, 280(%r12)
	movq	%rax, -240(%rbp)
	call	malloc
	movq	%rax, 288(%r12)
	movq	%rax, -144(%rbp)
	movq	-112(%rbp), %rax
	cmpq	%rbx, %rax
	movq	%rax, %rdi
	cmovb	%rbx, %rdi
	salq	$3, %rdi
	call	malloc
	testq	%r15, %r15
	movq	-144(%rbp), %rsi
	movq	-240(%rbp), %rcx
	movq	%rax, %rdx
	movq	%rax, 296(%r12)
	sete	%al
	cmpq	$0, -280(%rbp)
	sete	%dil
	orl	%edi, %eax
	testq	%rsi, %rsi
	sete	%sil
	testq	%rcx, %rcx
	sete	%cl
	orl	%esi, %ecx
	orb	%cl, %al
	jne	.L33
	testq	%rdx, %rdx
	je	.L33
	movl	-80(%rbp), %eax
	testl	%eax, %eax
	jle	.L40
	movl	-80(%rbp), %ecx
	vmovsd	.LC3(%rip), %xmm1
	vmovsd	24(%r12), %xmm0
	vdivsd	-176(%rbp), %xmm1, %xmm5
	vmulsd	%xmm0, %xmm0, %xmm0
	vmovsd	%xmm5, -288(%rbp)
	vdivsd	%xmm0, %xmm1, %xmm7
	vmulsd	.LC4(%rip), %xmm7, %xmm3
	vmovsd	%xmm7, -304(%rbp)
	vmovsd	%xmm3, -296(%rbp)
	leal	-1(%rcx), %eax
	cmpl	$6, %eax
	jbe	.L62
	vbroadcastsd	%xmm7, %ymm0
	vbroadcastsd	%xmm5, %ymm2
	vpbroadcastd	.LC14(%rip), %ymm1
	vpbroadcastd	.LC15(%rip), %ymm5
	vmulpd	.LC4(%rip){1to4}, %ymm0, %ymm4
	vbroadcastsd	.LC12(%rip), %ymm0
	vmovdqa	.LC0(%rip), %ymm3
	movq	%r12, -312(%rbp)
	shrl	$3, %ecx
	movl	%ecx, %edx
	vmovapd	%ymm4, -176(%rbp)
	movq	%r15, %r12
	salq	$6, %rdx
	vmovdqa	%ymm5, -112(%rbp)
	vmulpd	%ymm0, %ymm2, %ymm6
	leaq	(%rdx,%r15), %rax
	vmovdqa	%ymm1, -272(%rbp)
	movq	%rax, %rbx
	vmovapd	%ymm6, -240(%rbp)
	.p2align 4
	.p2align 3
.L38:
	vmovdqa	%ymm3, %ymm0
	vpaddd	-112(%rbp), %ymm0, %ymm0
	vmovapd	-240(%rbp), %ymm6
	vpaddd	-272(%rbp), %ymm3, %ymm3
	addq	$64, %r12
	vcvtdq2pd	%xmm0, %ymm2
	vextracti128	$0x1, %ymm0, %xmm0
	vcvtdq2pd	%xmm0, %ymm0
	vmovdqa	%ymm3, -144(%rbp)
	vmulpd	%ymm6, %ymm2, %ymm2
	vmulpd	%ymm6, %ymm0, %ymm0
	vinsertf64x4	$0x1, %ymm0, %zmm2, %zmm0
	call	_ZGVeN8v_sin
	vmovapd	-176(%rbp), %ymm7
	vmovdqa	-144(%rbp), %ymm3
	vmovapd	%ymm0, %ymm2
	vextractf64x4	$0x1, %zmm0, %ymm0
	vmulpd	%ymm2, %ymm2, %ymm2
	vmulpd	%ymm0, %ymm0, %ymm0
	vmulpd	%ymm7, %ymm2, %ymm2
	vmulpd	%ymm7, %ymm0, %ymm0
	vmovupd	%ymm2, -64(%r12)
	vmovupd	%ymm0, -32(%r12)
	cmpq	%rbx, %r12
	jne	.L38
	movl	-80(%rbp), %ecx
	movq	-312(%rbp), %r12
	movl	%ecx, %edx
	andl	$-8, %edx
	movl	%edx, %eax
	cmpl	%edx, %ecx
	je	.L40
.L37:
	movl	-80(%rbp), %ecx
	subl	%edx, %ecx
	leal	-1(%rcx), %esi
	movl	%ecx, -240(%rbp)
	cmpl	$2, %esi
	jbe	.L42
	vpbroadcastd	%eax, %xmm1
	vpaddd	.LC10(%rip), %xmm1, %xmm1
	vmovddup	-288(%rbp), %xmm3
	vmulpd	.LC12(%rip){1to2}, %xmm3, %xmm3
	vmovddup	-304(%rbp), %xmm2
	vmulpd	.LC4(%rip){1to2}, %xmm2, %xmm2
	leaq	(%r15,%rdx,8), %rdx
	movl	%eax, -112(%rbp)
	movq	%rdx, -144(%rbp)
	vmovapd	%xmm2, -176(%rbp)
	vcvtdq2pd	%xmm1, %xmm0
	vpshufd	$238, %xmm1, %xmm1
	vcvtdq2pd	%xmm1, %xmm1
	vmulpd	%xmm3, %xmm0, %xmm0
	vmulpd	%xmm3, %xmm1, %xmm1
	vinsertf128	$0x1, %xmm1, %ymm0, %ymm0
	call	_ZGVdN4v_sin
	vmovapd	-176(%rbp), %xmm2
	movq	-144(%rbp), %rdx
	movl	-240(%rbp), %ecx
	vmovapd	%xmm0, %xmm1
	vextractf64x2	$0x1, %ymm0, %xmm0
	movl	-112(%rbp), %eax
	vmulpd	%xmm1, %xmm1, %xmm1
	vmulpd	%xmm0, %xmm0, %xmm0
	vmulpd	%xmm2, %xmm1, %xmm1
	vmulpd	%xmm2, %xmm0, %xmm0
	vmovupd	%xmm1, (%rdx)
	vmovupd	%xmm0, 16(%rdx)
	movl	%ecx, %edx
	andl	$-4, %edx
	addl	%edx, %eax
	cmpl	%edx, %ecx
	je	.L40
.L42:
	leal	1(%rax), %edx
	vmovsd	-288(%rbp), %xmm3
	vxorpd	%xmm5, %xmm5, %xmm5
	vmulsd	.LC12(%rip), %xmm3, %xmm7
	vcvtsi2sdl	%edx, %xmm5, %xmm0
	movl	%eax, -176(%rbp)
	movl	%edx, -144(%rbp)
	vmovsd	%xmm7, -112(%rbp)
	vmulsd	%xmm7, %xmm0, %xmm0
	vzeroupper
	call	sin
	movslq	-176(%rbp), %rcx
	movl	-144(%rbp), %edx
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-296(%rbp), %xmm0, %xmm0
	leaq	0(,%rcx,8), %rbx
	vmovsd	%xmm0, (%r15,%rbx)
	cmpl	%edx, -80(%rbp)
	jle	.L40
	leal	2(%rcx), %edx
	vxorpd	%xmm5, %xmm5, %xmm5
	movl	%edx, -144(%rbp)
	vcvtsi2sdl	%edx, %xmm5, %xmm0
	vmulsd	-112(%rbp), %xmm0, %xmm0
	call	sin
	movl	-144(%rbp), %edx
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-296(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, 8(%r15,%rbx)
	cmpl	-80(%rbp), %edx
	jge	.L40
	movl	-176(%rbp), %eax
	vxorpd	%xmm5, %xmm5, %xmm5
	addl	$3, %eax
	vcvtsi2sdl	%eax, %xmm5, %xmm0
	vmulsd	-112(%rbp), %xmm0, %xmm0
	call	sin
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-296(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, 16(%r15,%rbx)
.L40:
	testl	%r13d, %r13d
	jle	.L80
	leal	-1(%r13), %eax
	vmovsd	.LC3(%rip), %xmm1
	vmovsd	32(%r12), %xmm0
	vdivsd	-208(%rbp), %xmm1, %xmm6
	vmulsd	%xmm0, %xmm0, %xmm0
	vmovsd	%xmm6, -240(%rbp)
	vdivsd	%xmm0, %xmm1, %xmm4
	vmulsd	.LC4(%rip), %xmm4, %xmm7
	vmovsd	%xmm4, -288(%rbp)
	vmovsd	%xmm7, -272(%rbp)
	cmpl	$6, %eax
	jbe	.L63
	vbroadcastsd	%xmm4, %ymm0
	vpbroadcastd	.LC14(%rip), %ymm1
	vpbroadcastd	.LC15(%rip), %ymm4
	movq	-280(%rbp), %rax
	vmulpd	.LC4(%rip){1to4}, %ymm0, %ymm7
	vbroadcastsd	.LC12(%rip), %ymm0
	movl	%r13d, %r15d
	vmovdqa	.LC0(%rip), %ymm3
	shrl	$3, %r15d
	vbroadcastsd	%xmm6, %ymm2
	salq	$6, %r15
	vmovapd	%ymm7, -176(%rbp)
	movq	%rax, %r12
	addq	%rax, %r15
	vmovdqa	%ymm4, -112(%rbp)
	vmulpd	%ymm0, %ymm2, %ymm5
	vmovdqa	%ymm1, -208(%rbp)
	vmovapd	%ymm5, -144(%rbp)
	.p2align 4
	.p2align 3
.L45:
	vmovdqa	%ymm3, %ymm0
	vpaddd	-112(%rbp), %ymm0, %ymm0
	vmovapd	-144(%rbp), %ymm5
	vpaddd	-208(%rbp), %ymm3, %ymm3
	addq	$64, %r12
	vcvtdq2pd	%xmm0, %ymm2
	vextracti128	$0x1, %ymm0, %xmm0
	vcvtdq2pd	%xmm0, %ymm0
	vmovdqa	%ymm3, -80(%rbp)
	vmulpd	%ymm5, %ymm2, %ymm2
	vmulpd	%ymm5, %ymm0, %ymm0
	vinsertf64x4	$0x1, %ymm0, %zmm2, %zmm0
	call	_ZGVeN8v_sin
	vmovapd	-176(%rbp), %ymm4
	vmovdqa	-80(%rbp), %ymm3
	vmovapd	%ymm0, %ymm2
	vextractf64x4	$0x1, %zmm0, %ymm0
	vmulpd	%ymm2, %ymm2, %ymm2
	vmulpd	%ymm0, %ymm0, %ymm0
	vmulpd	%ymm4, %ymm2, %ymm2
	vmulpd	%ymm4, %ymm0, %ymm0
	vmovupd	%ymm2, -64(%r12)
	vmovupd	%ymm0, -32(%r12)
	cmpq	%r12, %r15
	jne	.L45
	movl	%r13d, %eax
	andl	$-8, %eax
	movl	%eax, %r12d
	cmpl	%r13d, %eax
	je	.L80
.L44:
	movl	%r13d, %r15d
	subl	%eax, %r15d
	leal	-1(%r15), %edx
	cmpl	$2, %edx
	jbe	.L47
	vpbroadcastd	%r12d, %xmm1
	vpaddd	.LC10(%rip), %xmm1, %xmm1
	vmovddup	-240(%rbp), %xmm3
	vmulpd	.LC12(%rip){1to2}, %xmm3, %xmm3
	movq	-280(%rbp), %rcx
	vmovddup	-288(%rbp), %xmm2
	vmulpd	.LC4(%rip){1to2}, %xmm2, %xmm2
	vmovapd	%xmm2, -112(%rbp)
	vcvtdq2pd	%xmm1, %xmm0
	vpshufd	$238, %xmm1, %xmm1
	vcvtdq2pd	%xmm1, %xmm1
	leaq	(%rcx,%rax,8), %rax
	movq	%rax, -80(%rbp)
	vmulpd	%xmm3, %xmm1, %xmm1
	vmulpd	%xmm3, %xmm0, %xmm0
	vinsertf128	$0x1, %xmm1, %ymm0, %ymm0
	call	_ZGVdN4v_sin
	vmovapd	-112(%rbp), %xmm2
	movq	-80(%rbp), %rax
	vmovapd	%xmm0, %xmm1
	vextractf64x2	$0x1, %ymm0, %xmm0
	vmulpd	%xmm1, %xmm1, %xmm1
	vmulpd	%xmm0, %xmm0, %xmm0
	vmulpd	%xmm2, %xmm1, %xmm1
	vmulpd	%xmm2, %xmm0, %xmm0
	vmovupd	%xmm1, (%rax)
	vmovupd	%xmm0, 16(%rax)
	movl	%r15d, %eax
	andl	$-4, %eax
	addl	%eax, %r12d
	cmpl	%eax, %r15d
	je	.L80
.L47:
	leal	1(%r12), %eax
	vmovsd	-240(%rbp), %xmm6
	vxorpd	%xmm3, %xmm3, %xmm3
	vmulsd	.LC12(%rip), %xmm6, %xmm6
	vcvtsi2sdl	%eax, %xmm3, %xmm0
	movl	%eax, -112(%rbp)
	vmovsd	%xmm6, -80(%rbp)
	vmulsd	%xmm6, %xmm0, %xmm0
	vzeroupper
	call	sin
	movq	-280(%rbp), %rbx
	movl	-112(%rbp), %eax
	movslq	%r12d, %r15
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-272(%rbp), %xmm0, %xmm0
	salq	$3, %r15
	vmovsd	%xmm0, (%rbx,%r15)
	cmpl	%eax, %r13d
	jle	.L20
	leal	2(%r12), %eax
	vxorpd	%xmm3, %xmm3, %xmm3
	movl	%eax, -112(%rbp)
	vcvtsi2sdl	%eax, %xmm3, %xmm0
	vmulsd	-80(%rbp), %xmm0, %xmm0
	call	sin
	movl	-112(%rbp), %eax
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-272(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, 8(%rbx,%r15)
	cmpl	%eax, %r13d
	jle	.L20
	addl	$3, %r12d
	vxorpd	%xmm3, %xmm3, %xmm3
	vcvtsi2sdl	%r12d, %xmm3, %xmm0
	vmulsd	-80(%rbp), %xmm0, %xmm0
	call	sin
	vmulsd	%xmm0, %xmm0, %xmm0
	vmulsd	-272(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, 16(%rbx,%r15)
	jmp	.L20
	.p2align 4
	.p2align 3
.L82:
	call	block_fst_plan_free
	movq	%r15, %rdi
	movl	$1, %r14d
	call	block_fst_plan_free
	movq	48(%r12), %rdi
	call	free
	movq	56(%r12), %rdi
	movq	$0, 48(%r12)
	call	free
	movq	64(%r12), %rdi
	movq	$0, 56(%r12)
	call	free
	movq	72(%r12), %rdi
	movq	$0, 64(%r12)
	call	free
	movq	264(%r12), %rdi
	movq	$0, 72(%r12)
	call	free
	movq	272(%r12), %rdi
	movq	$0, 264(%r12)
	call	free
	movq	280(%r12), %rdi
	movq	$0, 272(%r12)
	call	free
	movq	288(%r12), %rdi
	movq	$0, 280(%r12)
	call	free
	movq	296(%r12), %rdi
	movq	$0, 288(%r12)
	call	free
	movq	$0, 296(%r12)
	jmp	.L20
	.p2align 4
	.p2align 3
.L83:
	movl	-80(%rbp), %ecx
	movq	stderr(%rip), %rdi
	movl	%r13d, %r8d
	movl	%r15d, %edx
	movl	$.LC2, %esi
	xorl	%eax, %eax
	call	fprintf
	jmp	.L20
	.p2align 4
	.p2align 3
.L80:
	vzeroupper
	jmp	.L20
	.p2align 4
	.p2align 3
.L84:
	movl	$0, (%rcx)
	jmp	.L49
.L64:
	xorl	%edi, %edi
	xorl	%esi, %esi
	jmp	.L50
.L65:
	xorl	%r9d, %r9d
	movl	$1, -112(%rbp)
	jmp	.L55
.L62:
	xorl	%eax, %eax
	xorl	%edx, %edx
	jmp	.L37
.L63:
	xorl	%r12d, %r12d
	xorl	%eax, %eax
	jmp	.L44
.L33:
	movq	%r12, %rdi
	movl	$2, %r14d
	call	poisson_plan_free
	jmp	.L20
	.cfi_endproc
.LFE9:
	.size	poisson_plan_init, .-poisson_plan_init
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC16:
	.string	"pairwise"
.LC17:
	.string	"allatonce"
	.section	.rodata.str1.8
	.align 8
.LC19:
	.string	"poisson_solve: P %5d  Nx %8d  Ny %8d  %-9s  elapsed %12.6f s  (fst %10.6f  transpose %10.6f)  GFLOPS %8.3f\n"
	.text
	.p2align 4
	.globl	poisson_solve
	.type	poisson_solve, @function
poisson_solve:
.LFB12:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	leaq	16(%rsp), %r13
	.cfi_def_cfa 13, 0
	andq	$-32, %rsp
	pushq	-8(%r13)
	pushq	%rbp
	movq	%rsp, %rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	pushq	%r15
	pushq	%r14
	pushq	%r13
	.cfi_escape 0xf,0x3,0x76,0x68,0x6
	.cfi_escape 0x10,0xf,0x2,0x76,0x78
	.cfi_escape 0x10,0xe,0x2,0x76,0x70
	pushq	%r12
	pushq	%rbx
	movq	%rdi, %r13
	subq	$232, %rsp
	.cfi_escape 0x10,0xc,0x2,0x76,0x60
	.cfi_escape 0x10,0x3,0x2,0x76,0x58
	movl	(%rdi), %eax
	movq	%rdx, -232(%rbp)
	movq	48(%r13), %rdx
	movl	4(%rdi), %edi
	movq	288(%r13), %r12
	movl	%eax, -148(%rbp)
	movslq	44(%r13), %rax
	movl	%edi, -152(%rbp)
	movl	%eax, -236(%rbp)
	salq	$2, %rax
	movl	(%rdx,%rax), %r15d
	movq	64(%r13), %rdx
	movl	(%rdx,%rax), %ecx
	movq	72(%r13), %rdx
	movslq	(%rdx,%rax), %r14
	movslq	%edi, %rax
	movq	280(%r13), %rdi
	movslq	%r15d, %rdx
	movl	%ecx, -160(%rbp)
	imulq	%rax, %rdx
	movq	%rdi, -136(%rbp)
	movq	%rdx, -168(%rbp)
	testq	%rdx, %rdx
	je	.L89
	leaq	0(,%rdx,8), %rdx
	call	memmove
.L89:
	call	msg_barrier
	call	msg_wtime
	vmovsd	%xmm0, -208(%rbp)
	call	msg_wtime
	movq	-136(%rbp), %rbx
	leaq	176(%r13), %rax
	movq	%rax, %rdi
	movq	%rax, -176(%rbp)
	vmovsd	%xmm0, -216(%rbp)
	movq	%rbx, %rsi
	call	block_fst_apply
	call	msg_wtime
	vmovsd	%xmm0, -184(%rbp)
	call	msg_wtime
	subq	$8, %rsp
	movl	%r15d, %ecx
	pushq	$101
	pushq	56(%r13)
	movq	%r12, %rdx
	movq	%rbx, %rsi
	pushq	48(%r13)
	movq	%r13, %rdi
	vmovsd	%xmm0, -120(%rbp)
	movq	72(%r13), %r9
	movq	64(%r13), %r8
	call	dist_transpose
	addq	$32, %rsp
	call	msg_wtime
	vsubsd	-120(%rbp), %xmm0, %xmm7
	vmovsd	%xmm7, -200(%rbp)
	call	msg_wtime
	leaq	88(%r13), %rax
	movq	%r12, %rsi
	vmovsd	%xmm0, -224(%rbp)
	movq	%rax, %rdi
	movq	%rax, -192(%rbp)
	call	block_fst_apply
	movl	-148(%rbp), %ecx
	testl	%ecx, %ecx
	jle	.L88
	movslq	-160(%rbp), %rax
	movq	264(%r13), %rdi
	movq	272(%r13), %rbx
	movq	%rax, %rcx
	vmovq	%rax, %xmm5
	movl	%eax, %r11d
	testl	%eax, %eax
	jle	.L88
	salq	$3, %rax
	movq	%r13, -248(%rbp)
	vmovd	%xmm5, %r15d
	vmovq	%rax, %xmm7
	leal	-1(%rcx), %eax
	andl	$-4, %r15d
	xorl	%edx, %edx
	leaq	8(%r12,%rax,8), %r8
	movl	%eax, -128(%rbp)
	movl	-148(%rbp), %eax
	vmovdqa	%xmm7, %xmm3
	movq	%r14, %r13
	decl	%eax
	leaq	8(%rdi,%rax,8), %rax
	movq	%rax, -120(%rbp)
	leaq	8(,%r14,8), %rax
	leaq	(%rbx,%rax), %rsi
	vmovq	%rsi, %xmm6
	leaq	-8(%rbx,%rax), %rsi
	movl	%ecx, %eax
	shrl	$2, %eax
	vmovdqa	%xmm6, %xmm2
	leal	-1(%rax), %ecx
	vmovd	%xmm5, %eax
	incq	%rcx
	andl	$3, %eax
	salq	$5, %rcx
	movl	%eax, -144(%rbp)
	.p2align 4
	.p2align 3
.L100:
	vmovq	%xmm2, %r10
	vmovsd	(%rdi), %xmm1
	leaq	(%r12,%rdx,8), %r9
	movq	%r9, %rax
	subq	%r10, %rax
	cmpq	$16, %rax
	jbe	.L110
	cmpl	$1, %r11d
	je	.L110
	cmpl	$2, -128(%rbp)
	jbe	.L107
	vbroadcastsd	%xmm1, %ymm4
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L98:
	vaddpd	(%rsi,%rax), %ymm4, %ymm0
	vmovupd	(%r9,%rax), %ymm6
	vdivpd	%ymm0, %ymm6, %ymm0
	vmovupd	%ymm0, (%r9,%rax)
	addq	$32, %rax
	cmpq	%rax, %rcx
	jne	.L98
	cmpl	%r15d, %r11d
	je	.L104
	movl	-144(%rbp), %eax
	movl	%r15d, %r9d
	cmpl	$1, %eax
	movl	%eax, %r10d
	movl	%r15d, %eax
	je	.L102
.L97:
	leaq	(%rdx,%r9), %r14
	addq	%r13, %r9
	vmovddup	%xmm1, %xmm0
	vaddpd	(%rbx,%r9,8), %xmm0, %xmm0
	leaq	(%r12,%r14,8), %r14
	movl	%r10d, %r9d
	vmovupd	(%r14), %xmm7
	andl	$-2, %r9d
	addl	%r9d, %eax
	vdivpd	%xmm0, %xmm7, %xmm0
	vmovupd	%xmm0, (%r14)
	cmpl	%r9d, %r10d
	je	.L104
.L102:
	cltq
	leaq	(%rdx,%rax), %r9
	addq	%r13, %rax
	leaq	(%r12,%r9,8), %r9
	vaddsd	(%rbx,%rax,8), %xmm1, %xmm1
	vmovsd	(%r9), %xmm0
	vdivsd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r9)
.L104:
	vmovq	%xmm5, %rax
	addq	$8, %rdi
	addq	%rax, %rdx
	vmovq	%xmm3, %rax
	addq	%rax, %r8
	cmpq	-120(%rbp), %rdi
	jne	.L100
	movq	-248(%rbp), %r13
	vzeroupper
.L88:
	movq	-192(%rbp), %rdi
	movq	%r12, %rsi
	call	block_fst_apply
	call	msg_wtime
	vmovsd	-216(%rbp), %xmm7
	vaddsd	-224(%rbp), %xmm7, %xmm7
	vmovsd	%xmm0, -120(%rbp)
	vmovsd	%xmm7, -144(%rbp)
	call	msg_wtime
	subq	$8, %rsp
	pushq	$102
	movq	-136(%rbp), %rbx
	movq	%r12, %rsi
	movq	%r13, %rdi
	pushq	72(%r13)
	pushq	64(%r13)
	movl	-160(%rbp), %ecx
	vmovsd	%xmm0, -128(%rbp)
	movq	56(%r13), %r9
	movq	48(%r13), %r8
	movq	%rbx, %rdx
	call	dist_transpose
	addq	$32, %rsp
	call	msg_wtime
	vsubsd	-128(%rbp), %xmm0, %xmm0
	vaddsd	-200(%rbp), %xmm0, %xmm5
	vmovq	%xmm5, %r12
	call	msg_wtime
	movq	-176(%rbp), %rdi
	movq	%rbx, %rsi
	vmovsd	%xmm0, -128(%rbp)
	call	block_fst_apply
	call	msg_wtime
	vmovsd	-184(%rbp), %xmm7
	vaddsd	-120(%rbp), %xmm0, %xmm0
	vsubsd	-128(%rbp), %xmm7, %xmm1
	vsubsd	-144(%rbp), %xmm0, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm1
	vmovsd	%xmm1, -120(%rbp)
	call	msg_wtime
	movl	$3, %edx
	leaq	-80(%rbp), %rsi
	leaq	-112(%rbp), %rdi
	vmovsd	-120(%rbp), %xmm1
	vsubsd	-208(%rbp), %xmm0, %xmm0
	vunpcklpd	%xmm1, %xmm0, %xmm0
	movq	%r12, -96(%rbp)
	vmovapd	%xmm0, -112(%rbp)
	call	gmax_double
	movl	-148(%rbp), %ebx
	movl	-152(%rbp), %r14d
	vmovsd	-80(%rbp), %xmm4
	vmovsd	-72(%rbp), %xmm1
	vmovsd	-64(%rbp), %xmm2
	vmovsd	%xmm4, -160(%rbp)
	vmovsd	%xmm1, -144(%rbp)
	vmovsd	%xmm2, -128(%rbp)
	vxorpd	%xmm7, %xmm7, %xmm7
	leal	1(%rbx), %r15d
	leal	1(%r14), %r12d
	vcvtsi2sdl	%r15d, %xmm7, %xmm0
	call	log2
	vxorpd	%xmm7, %xmm7, %xmm7
	vmovsd	%xmm0, -120(%rbp)
	vcvtsi2sdl	%r12d, %xmm7, %xmm0
	call	log2
	movl	84(%r13), %edx
	vmovsd	%xmm0, %xmm0, %xmm5
	vxorpd	%xmm7, %xmm7, %xmm7
	vmovsd	-160(%rbp), %xmm4
	vcvtsi2sdl	%r14d, %xmm7, %xmm3
	vcvtsi2sdl	%ebx, %xmm7, %xmm0
	vmovsd	-128(%rbp), %xmm2
	vmulsd	%xmm3, %xmm0, %xmm0
	vmovsd	-144(%rbp), %xmm1
	vaddsd	-120(%rbp), %xmm5, %xmm3
	vmulsd	.LC18(%rip), %xmm3, %xmm3
	vmulsd	%xmm3, %xmm0, %xmm3
	vunpcklpd	%xmm1, %xmm4, %xmm0
	vdivsd	%xmm4, %xmm3, %xmm3
	vunpcklpd	%xmm3, %xmm2, %xmm5
	vinsertf128	$0x1, %xmm5, %ymm0, %ymm0
	vmovupd	%ymm0, 304(%r13)
	testl	%edx, %edx
	je	.L123
	movl	-236(%rbp), %eax
	testl	%eax, %eax
	je	.L126
.L123:
	vzeroupper
.L92:
	movq	-168(%rbp), %rax
	testq	%rax, %rax
	je	.L124
	movq	-136(%rbp), %rsi
	movq	-232(%rbp), %rdi
	leaq	-40(%rbp), %rsp
	movq	%rax, %rdx
	popq	%rbx
	popq	%r12
	popq	%r13
	.cfi_remember_state
	.cfi_def_cfa 13, 0
	popq	%r14
	popq	%r15
	popq	%rbp
	salq	$3, %rdx
	leaq	-16(%r13), %rsp
	.cfi_def_cfa 7, 16
	popq	%r13
	.cfi_def_cfa_offset 8
	jmp	memmove
	.p2align 4
	.p2align 3
.L110:
	.cfi_restore_state
	movq	%rsi, %rax
	.p2align 4
	.p2align 3
.L96:
	vaddsd	(%rax), %xmm1, %xmm4
	vmovsd	(%r9), %xmm0
	addq	$8, %r9
	vdivsd	%xmm4, %xmm0, %xmm0
	addq	$8, %rax
	vmovsd	%xmm0, -8(%r9)
	cmpq	%r9, %r8
	jne	.L96
	jmp	.L104
.L124:
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	.cfi_remember_state
	.cfi_def_cfa 13, 0
	popq	%r14
	popq	%r15
	popq	%rbp
	leaq	-16(%r13), %rsp
	.cfi_def_cfa 7, 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L107:
	.cfi_restore_state
	movl	%r11d, %r10d
	xorl	%eax, %eax
	xorl	%r9d, %r9d
	jmp	.L97
.L126:
	cmpl	$1, 80(%r13)
	movl	40(%r13), %esi
	movl	$.LC17, %eax
	movl	$.LC16, %r8d
	movl	%r12d, %ecx
	movl	%r15d, %edx
	vmovsd	%xmm4, %xmm4, %xmm0
	movl	$.LC19, %edi
	cmovne	%rax, %r8
	movl	$4, %eax
	vzeroupper
	call	printf
	jmp	.L92
	.cfi_endproc
.LFE12:
	.size	poisson_solve, .-poisson_solve
	.p2align 4
	.globl	poisson_residual_op
	.type	poisson_residual_op, @function
poisson_residual_op:
.LFB13:
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
	vmovsd	.LC3(%rip), %xmm4
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	vmovsd	24(%rdi), %xmm3
	movl	4(%rdi), %ebx
	vmulsd	%xmm3, %xmm3, %xmm3
	vmovsd	32(%rdi), %xmm0
	vdivsd	%xmm3, %xmm4, %xmm3
	vmulsd	%xmm0, %xmm0, %xmm0
	movl	(%rdi), %ecx
	vdivsd	%xmm0, %xmm4, %xmm4
	movl	%ebx, -28(%rsp)
	testl	%ebx, %ebx
	jle	.L194
	decl	%ebx
	movq	%rdx, %r8
	movq	%rsi, %rax
	movl	%ebx, %edx
	jne	.L129
	xorl	%edi, %edi
.L130:
	leal	-1(%rdi), %r10d
	movl	%ecx, %r9d
	leal	-3(%rcx), %r11d
	leal	-1(%rcx), %esi
	imull	%ecx, %r10d
	andl	$-2, %r11d
	vxorpd	%xmm7, %xmm7, %xmm7
	imull	%edi, %r9d
	addl	$2, %r11d
	.p2align 4
	.p2align 3
.L157:
	testl	%ecx, %ecx
	jle	.L140
	movslq	%r9d, %r12
	leaq	0(,%r12,8), %rbx
	vmovsd	(%rax,%rbx), %xmm0
	testl	%edi, %edi
	je	.L198
	cmpl	$2, %ecx
	jle	.L177
	movslq	%r10d, %rdx
	addq	$16, %rbx
	xorl	%ebp, %ebp
	movl	%r11d, -16(%rsp)
	vmovq	%rdx, %xmm9
	leaq	(%rax,%rdx,8), %r13
	vmovsd	%xmm7, %xmm7, %xmm6
	leal	1(%r9), %edx
	vmovsd	%xmm7, %xmm7, %xmm1
	vmovsd	.LC21(%rip), %xmm2
	jmp	.L154
	.p2align 4
	.p2align 3
.L180:
	movq	%r14, %rbx
.L154:
	vmovsd	%xmm7, %xmm7, %xmm5
	cmpl	%ebp, %esi
	jle	.L152
	leal	(%rdx,%rbp), %r14d
	movslq	%r14d, %r14
	vmovsd	(%rax,%r14,8), %xmm5
.L152:
	vfmsub231sd	%xmm2, %xmm0, %xmm1
	movl	%ebp, %r14d
	leal	2(%rbp), %r15d
	incl	%r14d
	movl	%r15d, %ebp
	vsubsd	%xmm5, %xmm1, %xmm1
	vmovsd	%xmm0, %xmm0, %xmm5
	vfmsub213sd	0(%r13), %xmm2, %xmm5
	vmulsd	%xmm5, %xmm4, %xmm5
	vfmadd132sd	%xmm3, %xmm5, %xmm1
	vmovsd	%xmm7, %xmm7, %xmm5
	vmovsd	%xmm1, -16(%r8,%rbx)
	vmovsd	-8(%rax,%rbx), %xmm1
	cmpl	%r14d, %esi
	jle	.L153
	leal	(%r15,%r9), %r14d
	movslq	%r14d, %r14
	vmovsd	(%rax,%r14,8), %xmm5
.L153:
	vfmsub231sd	%xmm2, %xmm1, %xmm0
	leaq	16(%rbx), %r14
	addq	$16, %r13
	vsubsd	%xmm5, %xmm0, %xmm0
	vmovsd	%xmm1, %xmm1, %xmm5
	vfmsub213sd	-8(%r13), %xmm2, %xmm5
	vmulsd	%xmm5, %xmm4, %xmm5
	vfmadd132sd	%xmm3, %xmm5, %xmm0
	vmovsd	%xmm0, -8(%r8,%rbx)
	vmovsd	(%rax,%rbx), %xmm0
	cmpl	%r11d, %r15d
	jne	.L180
.L151:
	movslq	-16(%rsp), %rdx
	vmovq	%xmm9, %r15
	leaq	(%rax,%r15,8), %r14
	leal	1(%r9), %r15d
	leaq	1(%r12,%rdx), %rbp
	salq	$3, %rbp
	jmp	.L156
	.p2align 4
	.p2align 3
.L199:
	movq	%rbp, %rbx
	vmovsd	(%rax,%rbp), %xmm0
	vmovsd	-8(%rax,%rbp), %xmm1
	addq	$8, %rbp
.L156:
	vmovsd	%xmm6, %xmm6, %xmm5
	cmpl	%edx, %esi
	jle	.L155
	leal	(%r15,%rdx), %r12d
	movslq	%r12d, %r12
	vmovsd	(%rax,%r12,8), %xmm5
.L155:
	vfmsub231sd	%xmm2, %xmm0, %xmm1
	vfmsub213sd	(%r14,%rdx,8), %xmm2, %xmm0
	vmulsd	%xmm0, %xmm4, %xmm0
	incq	%rdx
	vsubsd	%xmm5, %xmm1, %xmm1
	vfmadd231sd	%xmm1, %xmm3, %xmm0
	vmovsd	%xmm0, (%r8,%rbx)
	cmpl	%edx, %ecx
	jg	.L199
.L140:
	incl	%edi
	addl	%ecx, %r10d
	addl	%ecx, %r9d
	cmpl	%edi, -28(%rsp)
	jg	.L157
.L194:
	popq	%rbx
	.cfi_remember_state
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
.L198:
	.cfi_restore_state
	cmpl	$2, %ecx
	jle	.L173
	addq	$16, %rbx
	movslq	%r11d, %rdx
	xorl	%r13d, %r13d
	leal	1(%r9), %r15d
	vmovsd	%xmm7, %xmm7, %xmm1
	jmp	.L146
	.p2align 4
	.p2align 3
.L176:
	movq	%rbp, %rbx
.L146:
	vmovsd	%xmm7, %xmm7, %xmm5
	cmpl	%r13d, %esi
	jle	.L144
	leal	(%r15,%r13), %ebp
	movslq	%ebp, %rbp
	vmovsd	(%rax,%rbp,8), %xmm5
.L144:
	vaddsd	%xmm0, %xmm0, %xmm2
	vsubsd	%xmm1, %xmm2, %xmm1
	vmulsd	%xmm2, %xmm4, %xmm2
	vsubsd	%xmm5, %xmm1, %xmm1
	vfmadd132sd	%xmm3, %xmm2, %xmm1
	movl	%r13d, %ebp
	leal	2(%r13), %r14d
	vmovsd	%xmm7, %xmm7, %xmm5
	incl	%ebp
	movl	%r14d, %r13d
	vmovsd	%xmm1, -16(%r8,%rbx)
	vmovsd	-8(%rax,%rbx), %xmm1
	cmpl	%ebp, %esi
	jle	.L145
	leal	(%r14,%r9), %ebp
	movslq	%ebp, %rbp
	vmovsd	(%rax,%rbp,8), %xmm5
.L145:
	vaddsd	%xmm1, %xmm1, %xmm2
	vsubsd	%xmm0, %xmm2, %xmm0
	vmulsd	%xmm2, %xmm4, %xmm2
	vsubsd	%xmm5, %xmm0, %xmm0
	vfmadd132sd	%xmm3, %xmm2, %xmm0
	leaq	16(%rbx), %rbp
	vmovsd	%xmm0, -8(%r8,%rbx)
	vmovsd	(%rax,%rbx), %xmm0
	cmpl	%r11d, %r14d
	jne	.L176
.L143:
	leaq	1(%r12,%rdx), %rbp
	leal	1(%r9), %r14d
	salq	$3, %rbp
	jmp	.L150
	.p2align 4
	.p2align 3
.L200:
	movq	%rbp, %rbx
	vmovsd	(%rax,%rbp), %xmm0
	vmovsd	-8(%rax,%rbp), %xmm1
	addq	$8, %rbp
.L150:
	vaddsd	%xmm0, %xmm0, %xmm0
	vsubsd	%xmm1, %xmm0, %xmm1
	cmpl	%edx, %esi
	jle	.L147
	leal	(%r14,%rdx), %r12d
	movslq	%r12d, %r12
	vsubsd	(%rax,%r12,8), %xmm1, %xmm1
.L147:
	vmulsd	%xmm0, %xmm4, %xmm0
	vfmadd231sd	%xmm1, %xmm3, %xmm0
	incq	%rdx
	vmovsd	%xmm0, (%r8,%rbx)
	cmpl	%edx, %ecx
	jg	.L200
	incl	%edi
	addl	%ecx, %r10d
	addl	%ecx, %r9d
	cmpl	%edi, -28(%rsp)
	jg	.L157
	jmp	.L194
	.p2align 4
	.p2align 3
.L129:
	movl	-28(%rsp), %edi
	movl	%ecx, %r11d
	vxorpd	%xmm8, %xmm8, %xmm8
	cmpl	%edi, %ebx
	movl	%ecx, %ebx
	cmovg	%edi, %edx
	negl	%ebx
	xorl	%r10d, %r10d
	movl	%edx, -16(%rsp)
	leal	-3(%rcx), %edx
	movl	%edx, %esi
	andl	$-2, %edx
	shrl	%esi
	leaq	(%rsi,%rsi), %rdi
	leal	2(%rdx), %esi
	movq	%rdi, -8(%rsp)
	xorl	%edi, %edi
	movl	%esi, -12(%rsp)
	.p2align 4
	.p2align 3
.L138:
	testl	%ecx, %ecx
	jle	.L160
	movslq	%r10d, %r15
	leal	-1(%rcx), %r9d
	leaq	0(,%r15,8), %rsi
	vmovsd	(%rax,%rsi), %xmm0
	testl	%edi, %edi
	je	.L161
	cmpl	$2, %ecx
	jle	.L201
	movslq	%r11d, %rdx
	leal	1(%r10), %r14d
	addq	$16, %rsi
	vmovsd	%xmm8, %xmm8, %xmm7
	vmovq	%rdx, %xmm5
	leaq	(%rax,%rdx,8), %rbp
	movq	-8(%rsp), %rdx
	vmovd	%r14d, %xmm11
	vmovq	%xmm5, %r13
	vmovsd	%xmm8, %xmm8, %xmm1
	vmovsd	.LC21(%rip), %xmm6
	leaq	4(%r15,%rdx), %r12
	leaq	0(,%r12,8), %rdx
	movslq	%ebx, %r12
	movq	%r12, %r14
	movq	%rdx, -24(%rsp)
	xorl	%edx, %edx
	subq	%r13, %r14
	jmp	.L131
	.p2align 4
	.p2align 3
.L170:
	movq	%r13, %rsi
.L131:
	vmovsd	%xmm8, %xmm8, %xmm10
	cmpl	%r9d, %edx
	jge	.L135
	vmovd	%xmm11, %r13d
	addl	%edx, %r13d
	movslq	%r13d, %r13
	vmovsd	(%rax,%r13,8), %xmm10
.L135:
	vfmsub231sd	%xmm6, %xmm0, %xmm1
	vmovsd	0(%rbp,%r14,8), %xmm2
	vaddsd	0(%rbp), %xmm2, %xmm2
	vfmsub231sd	%xmm6, %xmm0, %xmm2
	movl	%edx, %r13d
	addl	$2, %edx
	incl	%r13d
	vsubsd	%xmm10, %xmm1, %xmm1
	vmulsd	%xmm4, %xmm2, %xmm2
	vfmadd132sd	%xmm3, %xmm2, %xmm1
	vmovsd	%xmm8, %xmm8, %xmm10
	vmovsd	%xmm1, -16(%r8,%rsi)
	vmovsd	-8(%rax,%rsi), %xmm1
	cmpl	%r13d, %r9d
	jle	.L137
	leal	(%rdx,%r10), %r13d
	movslq	%r13d, %r13
	vmovsd	(%rax,%r13,8), %xmm10
.L137:
	vmovsd	8(%rbp,%r14,8), %xmm2
	vfmsub231sd	%xmm6, %xmm1, %xmm0
	vaddsd	8(%rbp), %xmm2, %xmm2
	vfmsub231sd	%xmm6, %xmm1, %xmm2
	leaq	16(%rsi), %r13
	addq	$16, %rbp
	vsubsd	%xmm10, %xmm0, %xmm0
	vmulsd	%xmm2, %xmm4, %xmm2
	vfmadd132sd	%xmm3, %xmm2, %xmm0
	vmovsd	%xmm0, -8(%r8,%rsi)
	vmovsd	(%rax,%rsi), %xmm0
	cmpq	-24(%rsp), %r13
	jne	.L170
.L163:
	movslq	%edx, %rdx
	leaq	(%rax,%r12,8), %r14
	leaq	1(%r15,%rdx), %rbp
	vmovq	%xmm5, %r15
	leaq	(%rax,%r15,8), %r13
	leal	1(%r10), %r15d
	salq	$3, %rbp
	jmp	.L134
	.p2align 4
	.p2align 3
.L202:
	movq	%rbp, %rsi
	vmovsd	(%rax,%rbp), %xmm0
	vmovsd	-8(%rax,%rbp), %xmm1
	addq	$8, %rbp
.L134:
	vmovsd	%xmm7, %xmm7, %xmm5
	cmpl	%edx, %r9d
	jle	.L132
	leal	(%r15,%rdx), %r12d
	movslq	%r12d, %r12
	vmovsd	(%rax,%r12,8), %xmm5
.L132:
	vfmsub231sd	%xmm6, %xmm0, %xmm1
	vmovsd	0(%r13,%rdx,8), %xmm2
	vaddsd	(%r14,%rdx,8), %xmm2, %xmm2
	vfmsub132sd	%xmm6, %xmm2, %xmm0
	incq	%rdx
	vsubsd	%xmm5, %xmm1, %xmm1
	vmulsd	%xmm0, %xmm4, %xmm0
	vfmadd231sd	%xmm1, %xmm3, %xmm0
	vmovsd	%xmm0, (%r8,%rsi)
	cmpl	%edx, %ecx
	jg	.L202
.L160:
	incl	%edi
	addl	%ecx, %ebx
	addl	%ecx, %r11d
	addl	%ecx, %r10d
	cmpl	-16(%rsp), %edi
	jl	.L138
.L204:
	cmpl	%edi, -28(%rsp)
	jg	.L130
	popq	%rbx
	.cfi_remember_state
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
.L161:
	.cfi_restore_state
	cmpl	$2, %ecx
	jle	.L182
	movslq	%r11d, %rdx
	addq	$16, %rsi
	xorl	%r13d, %r13d
	vmovsd	%xmm8, %xmm8, %xmm7
	vmovq	%rdx, %xmm5
	leaq	(%rax,%rdx,8), %rbp
	movl	-12(%rsp), %edx
	vmovsd	%xmm8, %xmm8, %xmm1
	vmovsd	.LC21(%rip), %xmm6
	movl	%edx, -24(%rsp)
	leal	1(%r10), %edx
	jmp	.L167
	.p2align 4
	.p2align 3
.L185:
	movq	%r12, %rsi
.L167:
	vmovsd	%xmm8, %xmm8, %xmm2
	cmpl	%r13d, %r9d
	jle	.L165
	leal	(%rdx,%r13), %r12d
	movslq	%r12d, %r12
	vmovsd	(%rax,%r12,8), %xmm2
.L165:
	vfmsub231sd	%xmm6, %xmm0, %xmm1
	movl	%r13d, %r12d
	leal	2(%r13), %r14d
	incl	%r12d
	movl	%r14d, %r13d
	vsubsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm0, %xmm0, %xmm2
	vfmsub213sd	0(%rbp), %xmm6, %xmm2
	vmulsd	%xmm2, %xmm4, %xmm2
	vfmadd132sd	%xmm3, %xmm2, %xmm1
	vmovsd	%xmm8, %xmm8, %xmm2
	vmovsd	%xmm1, -16(%r8,%rsi)
	vmovsd	-8(%rax,%rsi), %xmm1
	cmpl	%r12d, %r9d
	jle	.L166
	leal	(%r14,%r10), %r12d
	movslq	%r12d, %r12
	vmovsd	(%rax,%r12,8), %xmm2
.L166:
	vfmsub231sd	%xmm6, %xmm1, %xmm0
	leaq	16(%rsi), %r12
	addq	$16, %rbp
	vsubsd	%xmm2, %xmm0, %xmm0
	vmovsd	%xmm1, %xmm1, %xmm2
	vfmsub213sd	-8(%rbp), %xmm6, %xmm2
	vmulsd	%xmm4, %xmm2, %xmm2
	vfmadd132sd	%xmm3, %xmm2, %xmm0
	vmovsd	%xmm0, -8(%r8,%rsi)
	vmovsd	(%rax,%rsi), %xmm0
	cmpl	-12(%rsp), %r14d
	jne	.L185
.L164:
	movslq	-24(%rsp), %rdx
	leal	1(%r10), %r14d
	leaq	1(%r15,%rdx), %rbp
	vmovq	%xmm5, %r15
	salq	$3, %rbp
	leaq	(%rax,%r15,8), %r13
	jmp	.L169
	.p2align 4
	.p2align 3
.L203:
	movq	%rbp, %rsi
	vmovsd	(%rax,%rbp), %xmm0
	vmovsd	-8(%rax,%rbp), %xmm1
	addq	$8, %rbp
.L169:
	vmovsd	%xmm7, %xmm7, %xmm2
	cmpl	%edx, %r9d
	jle	.L168
	leal	(%r14,%rdx), %r12d
	movslq	%r12d, %r12
	vmovsd	(%rax,%r12,8), %xmm2
.L168:
	vfmsub231sd	%xmm6, %xmm0, %xmm1
	vfmsub213sd	0(%r13,%rdx,8), %xmm6, %xmm0
	vmulsd	%xmm0, %xmm4, %xmm0
	incq	%rdx
	vsubsd	%xmm2, %xmm1, %xmm1
	vfmadd231sd	%xmm1, %xmm3, %xmm0
	vmovsd	%xmm0, (%r8,%rsi)
	cmpl	%edx, %ecx
	jg	.L203
	incl	%edi
	addl	%ecx, %ebx
	addl	%ecx, %r11d
	addl	%ecx, %r10d
	cmpl	-16(%rsp), %edi
	jl	.L138
	jmp	.L204
.L201:
	movslq	%r11d, %r14
	vxorpd	%xmm7, %xmm7, %xmm7
	xorl	%edx, %edx
	movslq	%ebx, %r12
	vmovq	%r14, %xmm5
	vmovsd	%xmm7, %xmm7, %xmm1
	vmovsd	.LC21(%rip), %xmm6
	jmp	.L163
.L177:
	movslq	%r10d, %rdx
	vxorpd	%xmm6, %xmm6, %xmm6
	movl	$0, -16(%rsp)
	vmovsd	%xmm6, %xmm6, %xmm1
	vmovq	%rdx, %xmm9
	vmovsd	.LC21(%rip), %xmm2
	jmp	.L151
.L182:
	movslq	%r11d, %rdx
	movl	$0, -24(%rsp)
	vmovsd	%xmm8, %xmm8, %xmm7
	vmovsd	%xmm8, %xmm8, %xmm1
	vmovq	%rdx, %xmm5
	vmovsd	.LC21(%rip), %xmm6
	jmp	.L164
.L173:
	xorl	%edx, %edx
	vxorpd	%xmm1, %xmm1, %xmm1
	jmp	.L143
	.cfi_endproc
.LFE13:
	.size	poisson_residual_op, .-poisson_residual_op
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC0:
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.align 32
.LC1:
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.long	8
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC3:
	.long	0
	.long	1072693248
	.align 8
.LC4:
	.long	0
	.long	1074790400
	.set	.LC10,.LC1
	.align 8
.LC12:
	.long	1413754136
	.long	1073291771
	.set	.LC13,.LC0
	.set	.LC14,.LC1+28
	.set	.LC15,.LC0+4
	.align 8
.LC18:
	.long	-500134854
	.long	1045789070
	.align 8
.LC21:
	.long	0
	.long	1073741824
	.ident	"GCC: (GNU) 11.5.0 20240719 (Red Hat 11.5.0-14)"
	.section	.note.GNU-stack,"",@progbits
