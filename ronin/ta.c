#include "ta.h"
#include "maple.h"
#include "report.h"


/*

Resource: Binlists (2)
User: TA, CPU (finalization), ISP
State: Unused, binning, finalized, rendering
                  TA    CPU           ISP

Transitions:                                func              event
  Unused -> binning  (wait for & init TA) : ta_begin_frame  : Prgm
  binning -> finalized  (finalize)        : ta_interrupt    : TA done
  finalized -> rendering (start ISP)      : ta_interrupt    : ISP/FB done
  rendering -> unused                     : ta_interrupt    : ISP done


Resource: TA
State: Idle (binning = -1), binning (binning = list #)

Transitions:
   Idle -> binning    : ta_begin_frame  : Prgm
   binning -> idle    : ta_interrupt    : TA done


Resource: ISP
State: Idle (rendering = -1), rendering (rendering = list/FB #)

Transitions:
   Idle -> rendering      : ta_interrupt    : TA/FB done
   rendering -> rendering : ta_interrupt    : ISP done
   rendering -> idle      : ta_interrupt    : ISP done


Resource: FB (2)
User: ISP, RAMDAC
State: unused, rendering, pendning, displaying
                  ISP                 RAMDAC

Transitions:
   Unused -> rendering     : ta_interrupt    : TA/ISP done
   rendering -> pending    : ta_interrupt    : ISP done
   pending -> displaying   : ta_interrupt    : VBL
   displaying -> unused    : ta_interrupt    : VBL
   displaying -> rendering : ta_interrupt    : VBL & ISP done

*/


#define BINLIST_UNUSED    0
#define BINLIST_BINNING   1
#define BINLIST_FINALIZED 2
#define BINLIST_RENDERING 3

#define FB_UNUSED     0
#define FB_RENDERING  1
#define FB_PENDING    2
/* #define FB_DISPLAYING 3   the displaying fb always has this state,  */
                          /* and fb_other_state never has it, so no    */
                          /* actual value is needed...                 */

static volatile struct renderstate {

  /* TA */
  int ta_binning;    /* -1 or binlist number */
  int ta_robin;      /* binlist number       */
  int ta_events;     /* set of TA events to receive before job is done */

  /* ISP */
  int isp_rendering;  /* -1 or binlist number */
  int isp_robin;      /* binlist number       */

  /* FB */
  int fb_displaying;   /* fb number */
  int fb_other_state;

  /* binlists */
  int binlist_state[2];
  void *binlist_taend[2];

} renderstate;

static char *ta_target_dlist;
static unsigned int ta_dlist_size, ta_dlist_left;

struct ta_buffers ta_buffers[2];


/* Functions to send a list (the basic command unit of the TA) to the
   command list compiler.  ta_commit_list sends a 32-byte list, and
   ta_commit_list2 sends a 64-byte list.                              */

#define SQ_MAKE_ADDRESS(a) ((void*)((((unsigned int)(void*)(a))&0x03ffffff)|0xe0000000))
#define SQ_WAIT_STORE_QUEUES() do { unsigned int *d = (unsigned int *)0xe0000000; d[0] = d[8] = 0; } while(0)
#define TA_BINNING_ADDR ((void*)0xb0000000)
#define TA_BINNING_SIZE 0x04000000
#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)

void ta_commit_list(void *src)
{
  unsigned int *s = (unsigned int *)src;
  unsigned int *d = (unsigned int *)0xe0000000/*ta_target_dlist*/;

  if((ta_dlist_left -= 32)<0) {
    ta_dlist_left += 32;
    return;
  }
  d[0] = *s++;
  d[1] = *s++;
  d[2] = *s++;
  d[3] = *s++;
  d[4] = *s++;
  d[5] = *s++;
  d[6] = *s++;
  d[7] = *s++;
  asm("pref @%0" : : "r" (d));
  ta_target_dlist += 32;
}

void ta_commit_list2(void *src)
{
  unsigned int *s = (unsigned int *)src;
  unsigned int *d = (unsigned int *)0xe0000000/*ta_target_dlist*/;

  if((ta_dlist_left -= 64)<0) {
    ta_dlist_left += 64;
    return;
  }
  d[0] = *s++;
  d[1] = *s++;
  d[2] = *s++;
  d[3] = *s++;
  d[4] = *s++;
  d[5] = *s++;
  d[6] = *s++;
  d[7] = *s++;
  asm("pref @%0" : : "r" (d));
  d[8] = *s++;
  d[9] = *s++;
  d[10] = *s++;
  d[11] = *s++;
  d[12] = *s++;
  d[13] = *s++;
  d[14] = *s++;
  d[15] = *s++;
  asm("pref @%0" : : "r" (d+8));
  ta_target_dlist += 64;
}


/* Send the special end of list command */

void ta_commit_end()
{
  unsigned int words[8] = { 0 };
  ta_commit_list(words);
}


/* Create a compiled displaylist */

void ta_begin_dlist(void *list, unsigned int size)
{
  int t;
  if((t=((unsigned int)list)&31)) {
    t = 32-t;
    list = (void *)(((char *)list)+t);
    size -= t;
  }
  ta_target_dlist = SQ_MAKE_ADDRESS(list);
  ta_dlist_size = ta_dlist_left = size;
  QACR0 = QACR1 = ((((unsigned int)list)>>26)<<2)&0x1c;
  SQ_WAIT_STORE_QUEUES();
}

unsigned int ta_end_dlist()
{
  unsigned int ret = ta_dlist_size - ta_dlist_left;
  ta_dlist_size = ta_dlist_left = 0;
  SQ_WAIT_STORE_QUEUES();
  return ret;
}


/* Interrupt handler */

void ta_interrupt(unsigned int events)
{
  int n;
  struct renderstate *rs = (struct renderstate *)&renderstate;
                        /* ^^ discard volatile ^^ */

  /* check for events */

  if((events & 0x780 /* TA event */) &&
     (n = rs->ta_binning) >= 0) {

    if(!((rs->ta_events &= ~(events & 0x780)))) {

      /* Finalize binlist */
      unsigned int *taend = (unsigned int *)
	(void *)(0xa5000000|*(volatile unsigned int *)0xa05f8138);
      int i;

      for (i=0; i<0x12; i++)
	taend[i] = 0;
      
      rs->binlist_taend[n] = taend;
      rs->binlist_state[n] = BINLIST_FINALIZED;
      rs->ta_binning = -1;
    }

  }

  if((events & 0x04 /* ISP event */) &&
     (n = rs->isp_rendering) >= 0) {
    rs->binlist_state[n] = BINLIST_UNUSED;
    rs->isp_rendering = -1;
    rs->fb_other_state = FB_PENDING;
  }

  if((events & 0x08 /* Bottom raster event */) &&
     rs->fb_other_state == FB_PENDING) {
    rs->fb_displaying ^= 1;

    /* set bitplane pointers */
    {
      struct ta_buffers *b = &ta_buffers[rs->fb_displaying];
      volatile unsigned int *reg = (volatile unsigned int *)(void*)0xa05f8050;
      unsigned int addr = ((unsigned int)b->fb_base)&0x007fffff;
      reg[0] = addr;
      if(reg[32]&(1<<4) /*LACE*/)
	reg[1] = addr+b->fb_modulo;
      else
	reg[1] = addr;
    }

    rs->fb_other_state = FB_UNUSED;
  }

  if(rs->isp_rendering < 0 &&
     rs->fb_other_state == FB_UNUSED &&
     rs->binlist_state[(n = rs->isp_robin)] == BINLIST_FINALIZED) {
    int fb = rs->fb_displaying^1;
    rs->binlist_state[n] = BINLIST_RENDERING;
    rs->fb_other_state = FB_RENDERING;
    rs->isp_rendering = n;

    /* start ISP render binlist n to fb fb */
    {
      volatile unsigned int *regs = (volatile unsigned int *)0xa05f8000;
      struct ta_buffers *blb = &ta_buffers[n];
      struct ta_buffers *fbb = &ta_buffers[fb];

      unsigned int cmdl = ((unsigned int)blb->ta_cmdlist)&0x007fffff;
      unsigned int tls = ((unsigned int)blb->ta_tiles)&0x007fffff;
      unsigned int scn = ((unsigned int)fbb->fb_base)&0x007fffff;

      regs[0x02c/4] = tls;
      regs[0x020/4] = cmdl;
      regs[0x060/4] = scn;
      regs[0x064/4] = scn;
      regs[0x08c/4] = 0x01000000 |
	((((unsigned int)rs->binlist_taend[n])&0x003fffff)<<1);
      regs[0x088/4] = 0x3e4cccc0 /* zclip */;
      regs[0x068/4] = (blb->ta_clipw-1)<<16;
      regs[0x06c/4] = (blb->ta_cliph-1)<<16;
      regs[0x04c/4] = fbb->fb_modulo>>3;
      regs[0x048/4] = fbb->fb_pixfmt;
      regs[0x014/4] = 0xffffffff; /* Launch! */
    }

    rs->isp_robin ^= 1;
  }

  if(events & 0x08)
    maple_vbl_handler();
}

/* Wait for interrupt */

static void ta_wait(volatile int *ptr, int val)
{
  while(*ptr != val)
    __asm__("sleep");
}


/* Frame creation */

void ta_begin_frame()
{
  int list;
  ta_wait(&renderstate.ta_binning, -1);
  renderstate.ta_binning = list = renderstate.ta_robin;
  renderstate.ta_robin = list ^ 1;
  ta_wait(&renderstate.binlist_state[list], BINLIST_UNUSED);

  /* setup TA for binning to binlist list... */
  {
    volatile unsigned int *regs = (volatile unsigned int *)0xa05f8000;
    struct ta_buffers *blb = &ta_buffers[list];
    unsigned int cmdl = ((unsigned int)blb->ta_cmdlist)&0x007fffff;
    unsigned int tbuf = ((unsigned int)blb->ta_tilebuf)&0x007fffff;
    
    regs[0x008/4] = 1;		/* Reset TA */
    regs[0x008/4] = 0;

    renderstate.ta_events = ((blb->ta_lists&0xf)<<7)|
      ((blb->ta_lists&0x10)<<17);
    renderstate.binlist_state[list] = BINLIST_BINNING;

    regs[0x124/4] = tbuf;
    regs[0x12c/4] = tbuf-(blb->ta_extra_segments<<6);
    regs[0x128/4] = cmdl;
    regs[0x130/4] = cmdl+blb->ta_cmdlistsize;
    regs[0x138/4] = cmdl;
    regs[0x13c/4] = ((blb->ta_tileh-1)<<16)|(blb->ta_tilew-1);
    regs[0x140/4] = 0x00100202;
    regs[0x140/4] = 0x00100000|
      ((blb->ta_lists & TA_LIST_OPAQUEPOLY)? 0x2 : 0)|
      ((blb->ta_lists & TA_LIST_OPAQUEMOD)? 0x20 : 0)|
      ((blb->ta_lists & TA_LIST_TRANSPOLY)? 0x200 : 0)|
      ((blb->ta_lists & TA_LIST_TRANSMOD)? 0x2000 : 0)|
      ((blb->ta_lists & TA_LIST_PUNCHTHROUGH)? 0x20000 : 0);
    regs[0x164/4] = tbuf;
    regs[0x144/4] = 0x80000000;	/* Confirm settings */
    
    while(regs[0x144/4] & 0x80000000);
  }

  ta_begin_dlist(TA_BINNING_ADDR, TA_BINNING_SIZE);
}

void ta_commit_frame()
{
  ta_commit_end();
  ta_end_dlist();
}

void ta_rollback_frame()
{
  int list;
  ta_end_dlist();
  if((list = renderstate.ta_binning)>=0) {
    renderstate.binlist_state[list] = BINLIST_UNUSED;
    renderstate.ta_robin = list;
  }
  renderstate.ta_binning = -1;
}


/* Wait for all rendering to complete */

void ta_sync()
{
  ta_wait(&renderstate.binlist_state[0], BINLIST_UNUSED);
  ta_wait(&renderstate.binlist_state[1], BINLIST_UNUSED);
  ta_wait(&renderstate.fb_other_state, FB_UNUSED);
}


/* Set up buffers and descriptors for a tilespace */

static void *ta_create_tile_descriptors(void *ptr, void *buf, int w, int h,
					int lists)
{
  /* each tile desriptor is 6 words.  In addition, there's a 6 word header */
  /* so, there are 6*(w*h+1) words stored at ptr.                          */

  /* each tile uses 64 bytes of buffer space per list.      */
  /* So buf must point to 64*w*h*num_lists bytes of data.   */

  int x, y;
  unsigned int *vr = ptr;
  unsigned int bf = ((unsigned int)buf)&0x007fffff;

  *vr++ = 0x10000000;
  *vr++ = 0x80000000;
  *vr++ = 0x80000000;
  *vr++ = 0x80000000;
  *vr++ = 0x80000000;
  *vr++ = 0x80000000;
  for (x=0; x<w; x++) {
    for (y=0; y<h; y++) {
      int bft = bf+((x+y*w)<<6);
      *vr++ = (y << 8) | (x << 2) | (1<<29);
      if(lists & TA_LIST_OPAQUEPOLY) {
	*vr++ = bft;
	bft += (w*h)<<6;
      } else
	*vr++ = 0x80000000;
      if(lists & TA_LIST_OPAQUEMOD) {
	*vr++ = bft;
	bft += (w*h)<<6;
      } else
	*vr++ = 0x80000000;
      if(lists & TA_LIST_TRANSPOLY) {
	*vr++ = bft;
	bft += (w*h)<<6;
      } else
	*vr++ = 0x80000000;
      if(lists & TA_LIST_TRANSMOD) {
	*vr++ = bft;
	bft += (w*h)<<6;
      } else
	*vr++ = 0x80000000;
      if(lists & TA_LIST_PUNCHTHROUGH) {
	*vr++ = bft;
	bft += (w*h)<<6;
      } else
	*vr++ = 0x80000000;
    }
  }
  vr[-6] |= 0x80000000;

  return (char *)ptr;
}


/* Initialize rendering engine state */

#define ALIGN_AMOUNT 16384
#define ALIGN_VMEM(a) \
  ((void*)((((unsigned int)(void *)(a))+ALIGN_AMOUNT-1)&~(ALIGN_AMOUNT-1)))

void ta_init_renderstate()
{
  int i, j, n;

  renderstate.ta_binning = -1;
  renderstate.isp_rendering = -1;
  renderstate.ta_robin = 0;
  renderstate.isp_robin = 0;
  renderstate.fb_displaying = 0;
  renderstate.fb_other_state = FB_UNUSED;
  renderstate.binlist_state[0] = BINLIST_UNUSED;
  renderstate.binlist_state[1] = BINLIST_UNUSED;

  for(i=0; i<2; i++) {
    struct ta_buffers *b = &ta_buffers[i];

    char *vmem_ptr = (void *)(i? 0xa5400000 : 0xa5000000);
    char *vmem_end = vmem_ptr + 0x00200000;

    b->fb_base = vmem_ptr;
    vmem_ptr += b->fb_modulo * b->fb_lines;
    vmem_ptr = ALIGN_VMEM(vmem_ptr);

    b->ta_tiledescr = vmem_ptr;
    vmem_ptr += 6*sizeof(int)*(b->ta_tilew*b->ta_tileh+1);
    vmem_ptr = ALIGN_VMEM(vmem_ptr);

    n = 0;
    for(j=0; j<5; j++)
      if(b->ta_lists & (1<<j))
	n++;

    vmem_ptr += b->ta_extra_segments*64;
    b->ta_tilebuf = vmem_ptr;
    vmem_ptr += n*64*b->ta_tilew*b->ta_tileh;
    vmem_ptr = ALIGN_VMEM(vmem_ptr);

    b->ta_cmdlist = vmem_ptr;
    b->ta_cmdlistsize = vmem_end - vmem_ptr;

    b->ta_tiles =
      ta_create_tile_descriptors(b->ta_tiledescr, b->ta_tilebuf,
				 b->ta_tilew, b->ta_tileh, b->ta_lists);
  }

}


/* Enable IRQ handling */

void ta_enable_irq()
{
  volatile unsigned char *STB = (void*)0xffc00000;
  register unsigned int sr = 0;
  int mask;

  STB[4] = 0;
  STB[0x10] = 0;

  *((unsigned int *)0xa05f6930)|=0x8020078c;

  __asm__("stc sr,%0" : "=r" (sr));
  mask = (sr >> 4) & 0x0f;
  if(mask > 5)
    mask = 5;
  sr = (sr & 0xffffff0f) | (mask << 4);
  __asm__("ldc %0,sr" : : "r" (sr));
}


/* Disable IRQ */

void ta_disable_irq()
{
  *((unsigned int *)0xa05f6930)&=~0x20078c;
}

static unsigned char *tx_ptr = (unsigned char*)0xa4400000, *tx_last=0;
extern void reportf(const char *fmt, ...);

void *ta_txmark()
{
  return tx_ptr;
}

void ta_txrelease(void *ptr)
{
  tx_ptr = ptr;
  tx_last = 0;
}

void *ta_txalloc(unsigned int size)
{

  tx_last = tx_ptr;
  tx_ptr += (size+15)&~15;
  reportf("Alloc %d texture bytes at %p\n", size, tx_last); 
  return tx_last;
}

void ta_txfree(void *ptr)
{
  if(ptr == tx_last)
    tx_ptr = tx_last;
  else
    reportf("TXFree called with non-last block (%p). Wasting some memory.\n",
	    ptr);
}
