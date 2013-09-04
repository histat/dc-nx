
	! Derived from Dreamcast Video example by marcus
        ! Modification by ZinO
        !
	! Some global identifiers to be used from C.


	.globl	_dc_draw_string, _dc_draw_char12
        .globl  _dc_clrscr, _dc_check_cable, _dc_waitvbl
	
	.text

	.align	2

	! Draw a text string on screen
	!
	! Assumes a 640*480 screen with RGB555 or RGB565 pixels

	! r4 = x
	! r5 = y
	! r6 = string
	! r7 = colour
_dc_draw_string:
        ! void draw_string(int x, int y, char *message, int color)
	mov.l	r14,@-r15
	sts	pr,r14
	mov.l	r13,@-r15
	mov.l	r12,@-r15
	mov.l	r11,@-r15
	mov.l	r10,@-r15
	mov	r4,r10
	mov	r5,r11
	mov	r6,r12
	mov	r7,r13
ds_loop:
	mov.b	@r12+,r6
	mov	r10,r4
	mov	r11,r5
	tst	r6,r6	! string is NUL terminated
	bt	ds_done
	extu.b	r6,r6	! undo sign-extension of char
	bsr	_dc_draw_char12
	mov	r13,r7
	bra	ds_loop
	add	#12,r10
ds_done:			
	mov.l	@r15+,r10
	mov.l	@r15+,r11
	mov.l	@r15+,r12
	mov.l	@r15+,r13
	lds	r14,pr
	rts
	mov.l	@r15+,r14


	! Draw a "narrow" character on screen
	!
	! Assumes a 640*480 screen with RGB555 or RGB565 pixels

	! r4 = x
	! r5 = y
	! r6 = char
	! r7 = colour
_dc_draw_char12:
        ! void draw_char12(int x, int y, int char, int color)
	! First get the address of the ROM font
	sts	pr,r3
	mov.l	syscall_b4,r0
	mov.l	@r0,r0
	jsr	@r0
	mov	#0,r1
	lds	r3,pr
	mov	r0,r2

	! Then, compute the destination address
	shll	r4
	mov	r5,r0
	shll2	r0
	add	r5,r0
	shll8	r0
	add	r4,r0
	mov.l	vrambase,r1
	add	r1,r0

	! Find right char in font
	mov	#32,r1
	cmp/gt	r1,r6
	bt	okchar1
	! <= 32 = space or unprintable
blank:
	mov	#72,r6	! Char # 72 in font is blank
	bra	decided
	shll2	r6
okchar1:
	mov	#127,r1
	cmp/ge	r1,r6
	bf/s	decided	! 33-126 = ASCII, Char # 1-94 in font
	add	#-32,r6
	cmp/gt	r1,r6
	bf	blank	! 127-159 = unprintable
	add	#-96,r6
	cmp/gt	r1,r6
	bt	blank	! 256- = ?
	! 160-255 = JISX0201, char # 192-287 in font
	add	#64,r6
	add	#96,r6
	
	! Add offset of selected char to font addr
decided:
	mov	r6,r1
	shll2	r1
	shll	r1
	add	r6,r1
	shll2	r1
	add	r2,r1

	! Copy ROM data into cache so we can access it as bytes
	! Char data is 36 bytes, so we need to fetch two cache lines
	pref	@r1
	mov	r1,r2
	add	#32,r2
	pref	@r2
	
	mov	#24,r2	! char is 24 lines high
drawy:	
	! Each pixel line is stored as 1½ bytes, so we'll load
	! 3 bytes into r4 and draw two lines in one go
	mov.b	@r1+,r4
	shll8	r4
	mov.b	@r1+,r5
	extu.b	r5,r5
	or	r5,r4
	shll8	r4
	mov.b	@r1+,r5
	extu.b	r5,r5
	or	r5,r4
	shll8	r4
	! Even line
	mov	#12,r3
drawx1:	
	rotl	r4
	bf/s	nopixel1
	dt	r3
	mov.w	r7,@r0	! Set pixel
nopixel1:
	bf/s	drawx1
	add	#2,r0
	mov.w	drawmod,r3
	dt	r2
	add	r3,r0
	! Odd line
	mov	#12,r3
drawx2:	
	rotl	r4
	bf/s	nopixel2
	dt	r3
	mov.w	r7,@r0	! Set pixel
nopixel2:
	bf/s	drawx2
	add	#2,r0
	mov.w	drawmod,r3
	dt	r2
	bf/s	drawy
	add	r3,r0

	rts
	nop

	.align	4
syscall_b4:	
	.long	0x8c0000b4
drawmod:
	.word	2*(640-12)


	! Clear screen
	!
	! Assumes a 640*480 screen with RGB555 or RGB565 pixels

	! r4 = pixel colour
_dc_clrscr:	
        ! void clrscr(int color)
        mov.l	vrambase,r0
	mov.l	clrcount,r1
clrloop:
	mov.w	r4,@r0	! clear one pixel
	dt	r1
	bf/s	clrloop
	add	#2,r0
	rts
	nop

	.align	4
vrambase:
	.long	0xa5000000
clrcount:
	.long	640*480*3


	! Check type of A/V cable connected
	!
	! 0 = VGA
	! 1 = ---
	! 2 = RGB
	! 3 = Composite

_dc_check_cable:
        ! int check_cable()
        ! set PORT8 and PORT9 to input
	mov.l	porta,r0
	mov.l	pctra_clr,r2
	mov.l	@r0,r1
	mov.l	pctra_set,r3
	and	r2,r1
	or	r3,r1
	mov.l	r1,@r0
	! read PORT8 and PORT9
	mov.w	@(4,r0),r0
	shlr8	r0
	rts
	and	#3,r0

	.align 4
porta:
	.long	0xff80002c
pctra_clr:
	.long	0xfff0ffff
pctra_set:
	.long	0x000a0000

	
        ! waitvbl   (cortesy HeroZero)
_dc_waitvbl:
        ! void dc_waitvbl()
        mov.l   vsyncreg,r4
        mov.l   vsyncmask,r5
vsyncw1:
        mov.l   @r4,r2                  ! r2=vsyncreg
        tst     r5,r2                   ! already in vblank?
        bf      vsyncw1                 ! w8 until its done...
vsyncw2:
        mov.l   @r4,r3
        tst     r5,r3
        bt      vsyncw2
        rts
        nop

vsyncreg:
        .long   0xa05f810c
vsyncmask:
        .word   0x01ff

	.end
	

