
#ifndef _FASTBLIT_H
#define _FASTBLIT_H

// r0	: UL-corner of destination surface address
// r1	: pitch of destination surface, minus width of source image, *2 (words to bytes)

// r2	 : pointer to data pool
// r3-r10: current colors
typedef void (*FastBlitFunction)(uint8_t *dest, int sfcpitch);

#define NUM_CREGS		8		// how many registers to use for holding colors
#define FIRST_CREG		3		// reg number of first color register (e.g. r3)

// how long a run can be before it's collapsed into a loop
#define FB_LOOP_THRESHOLD	6


struct FastBlit
{
	FastBlitFunction entry_point;
	uint8_t *buffer;
	int length;
};


struct Sprite
{
	uint8_t *image;
	uint8_t *mask;
	int width, height;
	
	FastBlit fastblit;
};

// structure used while generating a FastBlit
struct FBGen
{
	DBuffer *code, *pool;
	
	int creg[NUM_CREGS];
	int next_creg;
};

#endif
