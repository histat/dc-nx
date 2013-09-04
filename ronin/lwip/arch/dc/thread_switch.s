
	.globl	_sys_thread_switch

	.text

	.align 1

_sys_thread_switch:
	mov.l r14,@-r15
	sts pr,r0
	mov.l r13,@-r15
	sts macl,r1
	mov.l r12,@-r15
	sts mach,r2
	mov.l r11,@-r15
	mov.l r10,@-r15
	mov.l r9,@-r15
	mov.l r8,@-r15
	mov.l r2,@-r15
	mov.l r1,@-r15
	mov.l r0,@-r15
	mov.l r15,@r4
	mov.l @r5,r15
	mov.l @r15+,r0
	mov.l @r15+,r1
	mov.l @r15+,r2
	mov.l @r15+,r8
	mov.l @r15+,r9
	mov.l @r15+,r10
	mov.l @r15+,r11
	mov.l @r15+,r12
	lds r0,pr
	mov.l @r15+,r13
	lds r1,macl
	mov.l @r15+,r14
	lds r2,mach
	rts
	mov r0,r6

