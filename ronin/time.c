#include "dc_time.h"

//#define TIMER_HZ 48828

#define TMU_REG(n) ((volatile void *)(0xffd80000+(n)))
#define TOCR (*(volatile unsigned char *)TMU_REG(0))
#define TSTR (*(volatile unsigned char *)TMU_REG(4))
#define TCOR0 (*(volatile unsigned int *)TMU_REG(8))
#define TCNT0 (*(volatile unsigned int *)TMU_REG(12))
#define TCR0 (*(volatile unsigned short *)TMU_REG(16))

static void init_tmr0()
{
  TSTR = 0;
  TOCR = 0;
  TCOR0 = ~0;
  TCNT0 = ~0;
  TCR0 = 4;
  TSTR = 1;
}

unsigned long Timer( )
{
  static int do_init_tmr = 1;
  if(do_init_tmr) {
    init_tmr0();
    do_init_tmr = 0;
  }
  return ~TCNT0;
}

void usleep(unsigned int usec)
{
  unsigned int t0 = Timer();
  unsigned int dly = USEC_TO_TIMER(usec);
  while( ((unsigned int)(Timer()-t0)) < dly );
}
