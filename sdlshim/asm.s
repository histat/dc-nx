
	.text

	.align	5

/* r4 = src
 r5 = dest
 r6 = copy width
 r7 = copy height
 parm4 = source pitch minus (copy width * 2)
 parm5 = dest pitch minus (copy width * 2) */

.globl _asm_do_blit_transparent
_asm_do_blit_transparent:
		mov.l	@(0,r15) ,r0		! load source pitch
		mov.l	@(4,r15) ,r1		! load dest pitch
	
heightLoop:
		mov		r6,r2	!X pixels remaining
widthLoop:
		mov.w	@r4+, r3
		tst		r3,r3
		bt		1f
		mov.w	r3,@r5
1:		
		dt		r2
		bf/s	widthLoop
		add		#2,r5
		dt		r7
		add		r0,r4		! add source pitch
		bf/s	heightLoop
		add		r1,r5       ! add dest pitch
		rts
		nop

.globl _asm_do_blit_nontransparent		
_asm_do_blit_nontransparent:
		mov.l	@(0,r15) ,r0		! load source pitch
		mov.l	@(4,r15) ,r1		! load dest pitch
heightLoop2:
		mov		r6,r2		! X pixels remaining
widthLoop2:
		mov.w	@r4+, r3
		dt		r2
		mov.w	r3,@r5
		bf/s 	widthLoop2
		add		#2,r5
		dt	 	r7
		add		r0,r4		! add source pitch
		bf/s	heightLoop2
		add		r1,r5       ! add dest pitch
		rts
		nop


/*
@ r0 = dest
@ r1 = src
@ r2 = length

.global asm_fast_memcpy_2
asm_fast_memcpy_2:
	stmfd	sp!, {r3-r10}
	
	@ldr		r3, [r1], #4
	@ldr		r4, [r1], #4
	@ldr		r5, [r1], #4
	@ldr		r6, [r1], #4
	@ldr		r7, [r1], #4
	@ldr		r8, [r1], #4
	@ldr		r9, [r1], #4
	@ldr		r10, [r1], #4
	
	@str		r3, [r0], #4
	@str		r4, [r0], #4
	@str		r5, [r0], #4
	@str		r6, [r0], #4
	@str		r7, [r0], #4
	@str		r8, [r0], #4
	@str		r9, [r0], #4
	@str		r10, [r0], #4

copy_loop:
	ldrmib	pc, [r1, #0x60]
	ldrmib	pc, [r1, #0x80]

	ldmia	r1!, {r3-r10}
	stmia	r0!, {r3-r10}
	ldmia	r1!, {r3-r10}
	stmia	r0!, {r3-r10}
	
	subs	r2, r2, #0x40
	bne		copy_loop
	
done:
	ldmfd	sp!, {r3-r10}
	bx		lr
	
	

.global asm_fast_memcpy
asm_fast_memcpy:
	stmfd	sp!, {r3-r10}
	
	@ bytes -> groups of 16 words
	mov		r2, r2, lsr #5
	
old_copy_loop:
	ldmia	r1!, {r3-r10}
	subs	r2, r2, #1
	stmia	r0!, {r3-r10}
	bne		old_copy_loop
	
	ldmfd	sp!, {r3-r10}
	bx		lr

*/






































