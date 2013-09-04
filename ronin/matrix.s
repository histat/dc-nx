
	! SH4 matrix operations
	!
	! The matrix is kept in the XMTRX register set between calls

	
	.globl _clear_matrix, _ortho_matrix, _load_matrix, _save_matrix
	.globl _apply_matrix, _transform_coords, _ta_commit_vertex

	.text


	! Initialize the matrix to the identity matrix
	!
	! no args
	
_clear_matrix:
	fldi0 fr0
	fldi0 fr1
	fldi1 fr2
	fldi0 fr3
	fldi0 fr4
	fldi1 fr5
	fschg
	fmov dr2,xd0
	fmov dr0,xd2
	fmov dr4,xd4
	fmov dr0,xd6
	fmov dr0,xd8
	fmov dr2,xd10
	fmov dr0,xd12
	fmov dr4,xd14
	rts
	fschg


	! Initialize the matrix to the inverse identity matrix
	!
	! no args
	
_ortho_matrix:
	fldi1 fr2
	fldi0 fr0
	fldi0 fr1
	fneg fr2
	fldi0 fr3
	fldi0 fr4
	fmov fr2,fr5
	fschg
	fmov dr2,xd0
	fmov dr0,xd2
	fmov dr4,xd4
	fmov dr0,xd6
	fmov dr0,xd8
	fmov dr2,xd10
	fmov dr0,xd12
	fmov dr4,xd14
	rts
	fschg

	
	! Load matrix from memory
	!
	! r4 = pointer to matrix (4 * 4 floats)

_load_matrix:
	frchg
	fmov.s @r4+,fr0
	fmov.s @r4+,fr1
	fmov.s @r4+,fr2
	fmov.s @r4+,fr3
	fmov.s @r4+,fr4
	fmov.s @r4+,fr5
	fmov.s @r4+,fr6
	fmov.s @r4+,fr7
	fmov.s @r4+,fr8
	fmov.s @r4+,fr9
	fmov.s @r4+,fr10
	fmov.s @r4+,fr11
	fmov.s @r4+,fr12
	fmov.s @r4+,fr13
	fmov.s @r4+,fr14
	fmov.s @r4+,fr15
	rts
	frchg

	
	! Save matrix to memory
	!
	! r4 = pointer to memory (4 * 4 floats)

_save_matrix:
	add #4*4*4,r4
	frchg
	fmov.s fr15,@-r4
	fmov.s fr14,@-r4
	fmov.s fr13,@-r4
	fmov.s fr12,@-r4
	fmov.s fr11,@-r4
	fmov.s fr10,@-r4
	fmov.s fr9,@-r4
	fmov.s fr8,@-r4
	fmov.s fr7,@-r4
	fmov.s fr6,@-r4
	fmov.s fr5,@-r4
	fmov.s fr4,@-r4
	fmov.s fr3,@-r4
	fmov.s fr2,@-r4
	fmov.s fr1,@-r4
	fmov.s fr0,@-r4
	rts
	frchg

	
	! Multiply another matrix to the current matrix
	!
	! r4 = pointer to the other matrix (4 * 4 floats)

_apply_matrix:
	mov #-7,r0
	fschg
	and r15,r0
	fmov dr8,@-r0
	fmov dr10,@-r0
	fmov dr12,@-r0
	fmov dr14,@-r0
	fschg
	fmov.s @r4+,fr0
	fmov.s @r4+,fr1
	fmov.s @r4+,fr2
	fmov.s @r4+,fr3
	ftrv xmtrx,fv0
	fmov.s @r4+,fr4
	fmov.s @r4+,fr5
	fmov.s @r4+,fr6
	fmov.s @r4+,fr7
	ftrv xmtrx,fv4
	fmov.s @r4+,fr8
	fmov.s @r4+,fr9
	fmov.s @r4+,fr10
	fmov.s @r4+,fr11
	ftrv xmtrx,fv8
	fmov.s @r4+,fr12
	fmov.s @r4+,fr13
	fmov.s @r4+,fr14
	fmov.s @r4+,fr15
	fschg
	ftrv xmtrx,fv12
	frchg
	fmov @r0+,dr14
	fmov @r0+,dr12
	fmov @r0+,dr10
	fmov @r0+,dr8
	rts
	fschg


	! Multiply a set of 3D vectors with the matrix
	! (vectors are extended to 4D homogenous coordinates by
	!  setting W=1), and then normalize the resulting
	! vectors before storing them in the result array
	!
	! r4 = pointer to a source set of 3D vectors (n * 3 floats)
	! r5 = pointer to a destination set of 3D vectors - ( -''-)
	! r6 = number of vectors to transform

_transform_coords:
	pref @r4
	mov r5,r0
	mov #4,r1
	mov r4,r3
	mov #8,r2
	add #32,r3
.loop:
	fmov.s @r4+,fr0
	fmov.s @r4+,fr1
	fmov.s @r4+,fr2
	fldi1 fr3
	pref @r3
	ftrv xmtrx,fv0
	fldi1 fr2
	dt r6
	fdiv fr3,fr2
	add #4*3,r3
	fmul fr2,fr0
	fmul fr2,fr1
	fneg fr2
	fmov.s fr0,@r0
	fmov.s fr1,@(r0,r1)
	fmov.s fr2,@(r0,r2)
	bf/s .loop
	add #4*3,r0
	rts	
	nop


	! Sumbit a 3D vertex to the TA, multiplying it with the matrix
	!
	! r4  = pointer to a source vertex; x,y,z fields ignored
	! fr4 = x coordinate
	! fr5 = y coordinate
	! fr6 = z coordinate

_ta_commit_vertex:
	mov.l @(0,r4),r1
	mov.l sq_address,r0
	fmov fr7,fr3
	fldi1 fr7
	mov.l r1,@r0
	ftrv xmtrx,fv4
	mov r0,r2
	fldi1 fr6
	mov.l @(16,r4),r1
	fdiv fr7,fr6
	add #16,r2
	fmul fr6,fr4
	fmul fr6,fr5
	fneg fr6
	mov.l @(20,r4),r3
	fmov.s fr6,@-r2
	mov.l r1,@(16,r0)
	fmov.s fr5,@-r2
	mov.l r3,@(20,r0)
	fmov.s fr4,@-r2
	mov.l @(24,r4),r1
	mov.l @(28,r4),r3
	mov.l r1,@(24,r0)
	fmov fr3,fr7
	mov.l r3,@(28,r0)
	rts
	pref @r0

	.align 2

sq_address:
	.long	  0xe0000000



	! sdivsi3_i4 and udivsi3_i4 are both borken in libgcc,
	! they clobber the FR bit in FPSCR.  These don't.

	.globl ___sdivsi3_i4, ___udivsi3_i4
	
___sdivsi3_i4:	
	mov.l r3,@-r15
        sts fpscr,r2
        mov #8,r3
        swap.w r3,r3
	or r2,r3
        lds r3,fpscr
        lds r4,fpul
        float fpul,dr0
        lds r5,fpul
        float fpul,dr2
        fdiv dr2,dr0
        ftrc dr0,fpul
        lds r2,fpscr
        rts
	mov.l @r15+,r3

___udivsi3_i4:
        mov #1,r1
        cmp/hi r1,r5
        bf trivial
	sts fpscr,r0
        rotr r1
	mov.l r0,@-r15
        xor r1,r4
        xor r1,r5
	mov #8,r1
	swap.w r1,r1
	or r0,r1
        mova 1f,r0
	lds r1,fpscr
        lds r4,fpul
        fmov.s @r0+,fr5
        fmov.s @r0,fr4
        float fpul,dr0
        lds r5,fpul
        float fpul,dr2
        fadd dr4,dr0
        fadd dr4,dr2
        fdiv dr2,dr0
        ftrc dr0,fpul
        rts
        lds.l @r15+,fpscr

trivial:
        rts
        lds r4,fpul

        .align 2

1:	.double 2147483648

	.end



