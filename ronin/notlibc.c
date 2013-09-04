
/* 
 * Rudimentary aproximations and stubs for various libc functions. 
 */

//#include <unistd.h> /* (s)brk() definitions */
//#include <stddef.h>   /* for size_t */
//#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"
#include "dc_time.h"
#include "notlibc.h"
#include "report.h"
#include "cdfs.h"
#include "serial.h"

int errno = 0;

#if 0
void _Console_Getc( char c )
{
}

void _Console_Putc( char c )
{
}
#endif

void exit(int rcode);

#if DC_GLIBC
/* Stuff needed to make the binary smaller (some 2Mb or so...)
   remember to link in libronin before libc and libgcc to avoid
   multiple definition conflicts. */
static FILE _stderr;
FILE *stderr = &_stderr;
#endif
int atexit(void (*function)(void)){ return 0; }
void abort()
{
    void *p;
    int i;
    unsigned long _stext = 0x8c010000;
    extern char _etext[];

    report("abort called.\n");
    for(i=0; i<8; i++) {
        switch(i) {
	    case 0: p = __builtin_return_address(0); break;
	    case 1: p = __builtin_return_address(1); break;
	    case 2: p = __builtin_return_address(2); break;
	    case 3: p = __builtin_return_address(3); break;
	    case 4: p = __builtin_return_address(4); break;
	    case 5: p = __builtin_return_address(5); break;
	    case 6: p = __builtin_return_address(6); break;
	    case 7: p = __builtin_return_address(7); break;
        }
        p -= 4;
        if(p >= (void *)_stext && p < (void *)_etext)
            printf("@ %p\n", p);
        else
            break;
    }
    printf("============\n");
    int *sp = (int *)&p;
    if( ((int)sp) & 3 )
    {
	printf("Invalid stack address!\n");
	sp = (int *)((int)sp&~3);
    }
    for( i = 0; i<100; i++ )
    {
	printf( "%p: %08x\n", sp+i, sp[i] );
    }
    printf("============\n");
    exit(1);
}
#ifdef DC_GLIBC
int sprintf(char *str, const char *format, ...)
 {report("sprintf ignored\n");return 0;}
int fprintf(FILE *stream,  const  char  *format, ...)
 {report("fprintf ignored\n");return -1;}
int printf(const char  *format, ...){report("printf ignored\n");return -1;}
int fputs( const char *s, FILE *stream ){report("fputs ignored\n"); return -1;}
int __write(){report("__write ignored\n"); return -1;}
FILE *fopen(const char *f, const char *m){report("fopen ignored\n"); return 0;}
#warning "Ignore one warnings for __isnan."
int __isnan(){} //Expect warning.
void __assert_fail(char *message){report("__asser_fail ignored\n");}
#endif
void __main(){}
//void matherr( void *exp ){report("matherr ignored\n");}


int _read (int file, char *ptr, int len) { return read(file, ptr, len); }
int _lseek (int file, int ptr, int dir) { return lseek(file, ptr, dir); }
#ifdef __SDCARD__
int _write ( int file, char *ptr, int len) { return write(file,ptr,len);}
#else
int _write ( int file, char *ptr, int len) {
  int n=len;
  if(file!=1 && file!=2) return -1;
#ifndef NOSERIAL
  while(n-->0) serial_putc(*ptr++);
  serial_flush();
#endif
  return len;
}
#endif

int _close (int file) { return close(file); }
caddr_t _sbrk (int incr) { return sbrk(incr); }
int _open (const char *path, int flags, ...) { return open(path, flags, 0); }
int _fstat (int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int isatty (int fd) { return fd>=0 && fd<=2; }
int _fcntl(int fildes, int cmd, ...)
{
  switch(cmd) {
  case F_GETFL:
    return (fildes==1 || fildes==2)? O_WRONLY : O_RDONLY;
  }
  return -1;
}


/* Real implementations of functions normally found in libc */

/*! @decl void exit(int rcode)
 *!
 *! @[rcode] is ignored.
 */
void exit(int rcode)
{
  report("Exit called. Exiting to menu.\n");
  /* -3:Slave, -1:Reset, 0|1:Menu */
  (*(void(**)())0x8c0000e0)(-3);
  for(;;);
}

/* 8c 000000  notused         64Kb - vectors
 * 8c 010000  binary start    2.9 Mb
 *
 * 8c 280000  malloc start    
 * 8c efffff  malloc end       12 Mb
 *
 * 8c f00000  stack end       
 * 8c fffffc  stack start     1 Mb
 */
extern char end[];
#define MEMSTART ((int)end)
#define MEMEND   0x8cf00000
static int total_size;

#ifndef OLDMALLOC
static int end_break=MEMSTART;

int brk( void *ebdds )
{
    //reportf( "brk( %x )\n", ebdds );
  int new_break = (int)ebdds;
  if(new_break >= MEMSTART && new_break <= MEMEND) {
    end_break = new_break;
    return 0;
  } else
    return -1;
}

#if 0
int stat(const char *path2, struct stat *buf)
{
//reportf("stat\n");
    memset( (void *)buf, 0, sizeof( struct stat ) );
    int fd = open( path2, O_RDONLY );
    if( fd == -1 )
    {
	DIR *q = opendir(path2);
	if( !q )
	    return -1;
	buf->st_mode  = 040555;
	closedir(q);
    }
    else
	buf->st_mode  = 000555;
    buf->st_size = file_size( fd );
    buf->st_blocks = (buf->st_size+1024) / 1024;
    buf->st_blksize = 1024;
    close( fd );
    return 0;
}

int access( const char *fname, int mode )
{
    int fd = open( fname, O_RDONLY );
    if( fd >= 0 )
    {
	close( fd );
	return 0;
    }
    DIR *dp = opendir( fname );
    if( dp )
    {
	closedir( dp );
	return 0;
    }
    return -1;
}
#endif

void *sbrk( ptrdiff_t incr )
{
  int prior_break = end_break;
  int newend_break = prior_break + incr;

  if(newend_break > MEMEND) {
/*     reportf("sbrk: Out of allocatable memory! (incr = %d)\n", incr); */
    return (void *)-1;
    /* Should set errno here if we are to be really compliant. */
  }

  total_size += incr;
  end_break = newend_break;

  reportf("sbrk [%dB alloc,\t%dk tot,\t%dk left -> %p]\r\n",
          incr,
          total_size/1024,
          (MEMEND-end_break)/1024,
          end_break);

  return (void *)(prior_break);
}


#else
static int last;
static int mallocpointer=MEMSTART;

void *malloc(size_t size)
{
  int keep;
  total_size += size;

  if(mallocpointer & 31)
    mallocpointer += 32-(mallocpointer&31);

  /* Point somewhere else next time... */
  keep=mallocpointer;
  mallocpointer += size;

/*   reportf("malloc [%d %d %d -> %p]\r\n", */
/*           size/1024, */
/*           total_size/1024, */
/*           (MEMEND-mallocpointer)/1024,  */
/*           mallocpointer); */

  if(mallocpointer > MEMEND) {
/*     report("Out of allocatable memory!\n"); */
    return 0;
  }

  return (void *)(last=keep);
}

void free(void *ptr)
{
  if( (int)ptr == last )
  {
    total_size -= mallocpointer-last;
    mallocpointer = last;
    last = 0;
  } else
    reportf("Free called with non-last block (%p). Wasting some memory.\n", ptr);
}
#endif

static unsigned int low_read_time()
{
  volatile unsigned int *rtc = (volatile unsigned int *)0xa0710000;
  return ((rtc[0]&0xffff)<<16)|(rtc[1]&0xffff);
}

#warning "Ignore one warnings for time:new."
time_t time(time_t *tloc)
{
  unsigned int old, new; //Expect stupid warning.
  time_t tmp;
  if(tloc == NULL)
    tloc = &tmp;
  old = low_read_time();
  for(;;) {
    int i;
    for(i=0; i<3; i++) {
      new = low_read_time();
      if(new != old)
	break;
    }
    if(i<3)
      old = new;
    else
      break;
  }
  return *tloc = new-631152000;
}

const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };
