#ifndef _RONIN_NOTLIBC_H
#define _RONIN_NOTLIBC_H been_here_before

/*
 * Functions used to make life easier when there is no full featured
 * clib available.
 */
#include <sys/time.h>
#include <stddef.h>
#include "common.h"
START_EXTERN_C


void exit(int rcode) /*__THROW*/ __attribute__ ((__noreturn__));
void free(void *ptr);
int brk( void *ebdds );
void *sbrk( ptrdiff_t incr );
/*  Standard (ANSI/SVID/...)  functions: */
void *malloc(size_t size);
void *calloc(size_t nelem, size_t elsize);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *memalign(size_t alignment, size_t size);
void *valloc(size_t size);
struct mallinfo mallinfo(void);
int mallopt(int cmd, int value);

/* 
  Additional functions:
    independent_calloc(size_t n_elements, size_t size, Void_t* chunks[]);
    independent_comalloc(size_t n_elements, size_t sizes[], Void_t* chunks[]);
    pvalloc(size_t n);
    cfree(Void_t* p);
    malloc_trim(size_t pad);
    malloc_usable_size(Void_t* p);
    malloc_stats();
*/

END_EXTERN_C

#endif //_RONIN_NOTLIBC_H
