
#include "shim.h"
#include "support.fdh"

/*
void c------------------------------() {}
*/

// it's malloc. but the block of memory it returns is guaranteed to
// be 4-byte aligned so you can DWORD-copy from it.
void *malloc_aligned(size_t size)
{
	uint8_t *ptr = (uint8_t *)malloc(size + 4);
	if (!ptr) return NULL;

	int offs = 3 - ((uint32_t)ptr & 3);
	ptr[offs] = offs;
	return (ptr + offs + 1);
}

// memory allocated with malloc_aligned() had dang well BETTER be
// freed using this special function.
void free_aligned(void *ptr)
{
	uint8_t *vptr = (uint8_t *)ptr;
	vptr--;
	vptr -= *vptr;
	free(vptr);
}
























