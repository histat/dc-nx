

		.globl _asm_do_blit_transparent
		.globl _asm_do_blit_nontransparent
		

		.text
		.align	5
		
/* r4 = src
 r5 = dest
 r6 = copy width
 r7 = copy height
 parm4 = source pitch minus (copy width * 2)
 parm5 = dest pitch minus (copy width * 2) */
_asm_do_blit_transparent:
		mov.l	r8,@-r15
		mov.l	r9,@-r15
		mov.l	r10,@-r15
		mov.l	r11,@-r15
		mov.l	@(16,r15) ,r8		! load source pitch
		mov.l	@(20,r15) ,r9		! load dest pitch
	
heightLoop:
		mov	r6,r10	!X pixels remaining
widthLoop:
		mov.w	@r4+, r11
		tst		r11,r11
		bt		1f
		mov.w	r11,@r5
1:		
		dt		r10
		bf/s	widthLoop
		add		#2,r5
		dt		r7
		add		r8,r4		! add source pitch
		bf/s	heightLoop
		add		r9,r5       ! add dest pitch
		mov.l	@r15+,r11
		mov.l	@r15+,r10
		mov.l	@r15+,r9
		rts
		mov.l	@r15+,r8

		.align	5
_asm_do_blit_nontransparent:
		mov.l	r8,@-r15
		mov.l	r9,@-r15
		mov.l	r10,@-r15
		mov.l	r11,@-r15
		mov.l	@(16,r15) ,r8		! load source pitch
		mov.l	@(20,r15) ,r9		! load dest pitch
heightLoop2:
		mov		r6,r10	! X pixels remaining
widthLoop2:
		mov.w	@r4+, r11
		mov.w	r11,@r5
		dt		r10
		bf/s 	widthLoop2
		add		#2,r5
		dt	 	r7
		add		r8,r4		! add source pitch
		bf/s	heightLoop2
		add		r9,r5       ! add dest pitch
		mov.l	@r15+,r11
		mov.l	@r15+,r10
		mov.l	@r15+,r9
		rts
		mov.l	@r15+,r8


		.align	5
		.globl _fcopy32
_fcopy32:
	    fmov	@r4+,dr0
	    fmov	@r4+,dr2
	    mov		#8,r0
	    fmov	@(r0,r4),dr6
	    add		#32,r5
	    fmov	@r4+,dr4
	    add		#8,r4
	    fmov	dr6,@-r5
	    fmov	dr4,@-r5
	    fmov	dr2,@-r5
		rts
	    fmov	dr0,@-r5
		
		.end
		


































