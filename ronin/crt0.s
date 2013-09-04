
	! Simple C startup strap
	!
	! Just enables cache and jumps to main
	!

	
	.globl	start
	
	.text

start:
	! First, make sure to run in the P2 area
        
       	mov.l	setup_cache_addr,r0
	mov.l	p2_mask,r1
	or	r1,r0
	jmp	@r0
	nop
setup_cache:
	! Now that we are in P2, it's safe
	! to enable the cache
	mov.l	ccr_addr,r0
	mov.w	ccr_data,r1
	mov.l	r1,@r0
	! After changing CCR, eight instructions
	! must be executed before it's safe to enter
	! a cached area such as P1
	mov.l	init_addr,r0	! 1
	mov	#0,r1		! 2
	nop			! 3
	nop			! 4
	nop			! 5
	nop			! 6
	nop			! 7
	nop			! 8
	jmp	@r0		! go
	mov	r1,r0


init:
	mov.l	stack_pointer,r15
	mov.l	bss_start_addr,r0
	mov.l	bss_end_addr,r2
	sub	r0,r2
	shlr	r2
	shlr	r2
	mov	#0,r1
.loop:	dt	r2
	mov.l	r1,@r0
	bf/s	.loop
	add	#4,r0

	mov.l	start_addr,r0
	ldc	r0,vbr

	stc	sr,r0
	mov.l	xsr_bl_mask,r1
	and	r1,r0
	ldc	r0,sr


	! Set up floating point ops, call constructors, and jump to main
	mov	#0,r2
	lds	r2,fpscr
	sts.l	pr,@-r15
	mov.l	ctors_start,r0
.ctors_loop:	
	mov.l	ctors_end,r1
	cmp/eq	r0,r1
	bt	.endctors
	mov.l	@r0+,r1
	jsr	@r1
	mov.l	r0,@-r15
	bra	.ctors_loop
	mov.l	@r15+,r0
.endctors:
	lds.l	@r15+,pr

	mov.l	main_addr,r0
	mov	#0,r1
	jmp	@r0
	mov	r1,r0

		
	.align	2
stack_pointer:
	.long	0x8cfffffc
p2_mask:
	.long	0xa0000000
setup_cache_addr:
	.long	setup_cache
init_addr:
	.long	init
main_addr:
	.long	_main
xvbr_addr:
	.long	0x8c00f400
xsr_bl_mask:
	.long	0x400083f3
bss_start_addr:
	.long	__bss_start
bss_end_addr:
	.long	_end
ctors_start:
	.long	___ctors
ctors_end:	
	.long	___ctors_end
start_addr:
	.long	start
ccr_addr:
	.long	0xff00001c
ccr_data:
	.word	0x090d


	.align	8

exc_hdler:	
	mov.l	exc_stack,r15
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	expevt_addr,r0
	mov.l	@r0,r0
	mov	r0,r7
	sts	pr,r6
	stc	spc,r5
	mova	exc_message,r0
	mov.l	reportf_addr,r8
	jsr	@r8
	mov	r0,r4
exc_hdler_stage_2:
	stc	r3_bank,r3
	stc	r2_bank,r2
	stc	r1_bank,r1
	stc	r0_bank,r0
	bsr	printregs
	mov	#3,r4
	stc	r7_bank,r3
	stc	r6_bank,r2
	stc	r5_bank,r1
	stc	r4_bank,r0
	bsr	printregs
	mov	#7,r4
	mov	r11,r3
	mov	r10,r2
	mov.l	@r15,r1
	mov.l	@(4,r15),r0
	bsr	printregs
	mov	#11,r4
	.word	0x033a	; stc	sgr,r3
	mov	r14,r2
	mov	r13,r1
	mov	r12,r0
	bsr	printregs
	mov	#15,r4
	
	mov.l	serflush_addr,r9
	jsr	@r9
	nop

	.word	0x0a3a	; stc	sgr,r10
	mov	#8,r11
.psloop:
	mov.l	@r10,r6
	mov	r10,r5
	mova	exc_msg3,r0
	mov	r0,r4
	jsr	@r8
	add	#4,r10
	dt	r11
	bf/s	.psloop
	add	#8,r15

	jsr	@r9
	nop
	
	mov.l	reboot_addr,r0
	jmp	@r0
	nop

printregs:
	sts.l	pr,@-r15
	mov.l	r3,@-r15
	mov.l	r4,@-r15
	add	#-1,r4
	mov.l	r2,@-r15
	mov.l	r4,@-r15
	add	#-1,r4
	mov.l	r1,@-r15
	mov	r4,r7
	add	#-1,r4
	mov	r0,r6
	mov	r4,r5
	mova	exc_msg2,r0
	jsr	@r8
	mov	r0,r4
	add	#20,r15
	lds.l	@r15+,pr
	rts
	nop
	
	.align	2

exc_stack:
	.long	exc_hdler_end
expevt_addr:
	.long	0xff000024
reportf_addr:
	.long	_reportf
serflush_addr:
	.long	_serial_flush
reboot_addr:
	.long	0xa0000000
	
	.align	2
exc_message:
	.ascii	"EXCEPTION AT %p {pr = %p} : %x\n"
	.byte	0

	.align	2
exc_msg2:
	.ascii	"r%d = 0x%x, r%d = 0x%x, r%d = 0x%x, r%d = 0x%x\n"
	.byte	0

	.align	2
exc_msg3:	
	.ascii	"%p : %p\n"
	.byte	0
		
	.align	10

mmu_exc:
	bra	exc_hdler
	nop
			
	.align	9

exc_hdler_end:	
int_hdler:	
	mov.l	intevt_addr,r0
	mov.w	pvr_intevt,r1
	mov.l	@r0,r0
	cmp/eq	r0,r1
	bf	unknown_irq
		
	mov.l	pvrevt_addr,r0
	mov.l	@r0,r4
	mov.l	pvrint_mask,r2
	and	r2,r4
	tst	r4,r4
	bt/s	dummyirq
	mov.l	r4,@r0

		mov.l	int_stack,r15
	sts.l	mach,@-r15
	sts.l	macl,@-r15
	sts.l	pr,@-r15
		mov.l	r4,@-r15

	mov.l	c_int_hdlr,r0
	jsr	@r0
	nop

		mov.l	@r15+,r0
		tst	#0x8,r0
		bt	.done
		
		mov.l	vsync_int_hdlr,r0
		jsr	@r0
		nop
.done:
		
	lds.l	@r15+,pr
	lds.l	@r15+,macl
	lds.l	@r15+,mach
		stc	sgr,r15
dummyirq:
	rte
	nop

unknown_irq:	
	mov.l	exc_stack2,r15
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	r0,@-r15
	sts.l	pr,@-r15
	stc.l	spc,@-r15
	mova	int_message,r0
	mov.l	reportf_addr2,r8
	jsr	@r8
	mov	r0,r4
	bra	exc_hdler_stage_2
	add	#12,r15
	
	.align	2

intevt_addr:	
	.long	0xff000028
pvrevt_addr:
	.long	0xa05f6900
c_int_hdlr:
	.long	_ta_interrupt

vsync_int_hdlr:
	.long	_vsync_event
				
int_stack:
	.long	exc_hdler_end
exc_stack2:
	.long	exc_hdler_end
reportf_addr2:
	.long	_reportf

pvrint_mask:
	.long	0x0020078c
pvr_intevt:
	.word	0x320

	.align	2
int_message:
	.ascii	"UNHANDLED INTERRUPT AT %p {pr = %p} : %x\n"
	.byte	0
	

	.end


