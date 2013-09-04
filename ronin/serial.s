
	! Derived from Dreamcast Serial example by marcus
        ! Modification by peter
        !
	! Some global identifiers to be used from C.



	.globl	_serial_init, _serial_flush, _serial_getc_blocking
        .globl  _serial_puts, _serial_putc, _serial_getc, 
	
	.text

	.align	2


	! Initialize SCIF registers
	!
	! r4 = baudrate (must be >= 6092 bps)
_serial_init:
        ! void init_serial(int baudrate)
        ! Get base address of SCIF
	mov	#-24,r2
	shll16	r2
	! disable interrupts, disable transmit/receive, use internal clock
	mov	#0,r0
	mov.w	r0,@(8,r2)
	! 8N1, use PØ clock
	mov.w	r0,@r2
	! Set baudrate, N = PØ/(32*B)-1
	div0u
	mov.l	Pphi_32rds,r0
	mov	r4,r1
	shlr	r1
	add	r1,r0
	mov	#0,r1
	.rep	32
	rotcl	r0
	div1	r4,r1
	.endr
	rotcl	r0
	add	#-1,r0
	mov.b	r0,@(4,r2)
	! reset FIFOs, enable hardware flow control
	add	#8,r2
	mov	#12,r0
	mov.w	r0,@(16,r2)
	mov	#8,r0
	mov.w	r0,@(16,r2)
	! disable manual pin control
	mov	#0,r0
	mov.w	r0,@(24,r2)
	! clear status
	mov.w	@(8,r2),r0
	mov	#0x60,r0
	mov.w	r0,@(8,r2)
	mov.w	@(28,r2),r0
	mov	#0,r0
	mov.w	r0,@(28,r2)
	! enable transmit/receive
	mov	#0x30,r0
	rts
	mov.w	r0,@r2

	.align	4

Pphi_32rds:
	.long	50000000/32


	! Send a NUL-terminated string to the serial port
	!
	! r4 = string
_serial_puts:
        ! void serial_puts(char *message)
	mov	#-24,r2
	shll16	r2
	add	#12,r2
.ssloop1:
	mov.b	@r4+,r1
	tst	r1,r1
	bt	.endstr
	extu.b	r1,r1
.ssloop2:
	mov.w	@(4,r2),r0
	tst	#0x20,r0
	bt	.ssloop2
	mov.b	r1,@r2
	nop
	and	#0x9f,r0
	bra	.ssloop1
	mov.w	r0,@(4,r2)
.endstr:
	nop
	rts


	! Send a single character to the serial port
	!
	! r4 = character
_serial_putc:
        ! void serial_putc(int char)
	mov	#-24,r2
	shll16	r2
	add	#12,r2
.sloop:	
	mov.w	@(4,r2),r0
	tst	#0x20,r0
	bt	.sloop
	mov.b	r4,@r2
	nop
	and	#0x9f,r0
	rts
	mov.w	r0,@(4,r2)


	! Check for serial input
	!
	! returns character or -1
_serial_getc:
        ! int serial_getc()
	mov	#-24,r2
	shll16	r2
	add	#12,r2
	mov.w	@(16,r2),r0
	tst	#0x1f,r0
	bt	.norecv
	mov.b	@(8,r2),r0
	extu.b	r0,r1
	mov.w	@(4,r2),r0
	and	#0x6d,r0
	mov.w	r0,@(4,r2)
	rts
	mov	r1,r0
.norecv:
	rts
	mov	#-1,r0


	! Wait for serial input
	!
	! returns character
_serial_getc_blocking:
        ! int serial_getc_blocking()
	sts	pr,r3
.rcvlp:	
	bsr	_serial_getc
	nop
	cmp/pz	r0
	bf	.rcvlp
	lds	r3,pr
	rts
	nop


	! Flush output FIFO
_serial_flush:
        ! void serial_flush()
	mov	#-24,r2
	shll16	r2
	add	#12,r2
	mov.w	@(4,r2),r0
	and	#0xbf,r0
	mov.w	r0,@(4,r2)
.wflush:	
	mov.w	@(4,r2),r0
	tst	#64,r0
	bt	.wflush	
	and	#0xbf,r0
	rts
	mov.w	r0,@(4,r2)
	
	
	.end

	
