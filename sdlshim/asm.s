
		.text
		.align 2
		
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


		.end

































