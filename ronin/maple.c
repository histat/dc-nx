#include "common.h"
#include "maple.h"

#define MAPLE(x) (*(volatile unsigned long *)(0xa05f6c00+(x)))

static struct mapledev dev[4];

static struct {
  unsigned int rbuf[4][256];
  unsigned int sbuf[4*(2+2)+2];
  char padding[32];
} *dmabuffer, dmabuffer0;

static int pending, gun_pending, lock;

static unsigned char xdmabuffer[ 1024 + 1024 + 4 + 4 + 32 ];

void maple_init()
{
  int i;

  MAPLE(0x8c) = 0x6155404f;
  MAPLE(0x10) = 0;
  MAPLE(0x80) = (50000<<16)|0;
  MAPLE(0x14) = 1;
  dmabuffer =
    (void*)(((((unsigned long)(void *)&dmabuffer0)+31)&~31)|0xa0000000);
  for(i=0; i<4; i++)
    dev[i].func = dev[i].xfunc = dev[i].ttl = 0;
  pending = gun_pending = lock = 0;
}

static void maple_wait_dma()
{
  while(MAPLE(0x18) & 1)
    ;
}

void *maple_docmd(int port, int unit, int cmd, int datalen, void *data)
{
  unsigned long *sendbuf, *recvbuf;
  int to, from;

  int s = getimask();
  setimask(15);
  if(lock) {
    setimask(s);
    return 0;
  }
  lock = 1;
  setimask(s);

  port &= 3;

  /* Compute sender and recipient address */
  from = port << 6;
  to = (port << 6) | (unit>0? ((1<<(unit-1))&0x1f) : 0x20);

  /* Max data length = 255 longs = 1020 bytes */
  if(datalen > 255)
    datalen = 255;
  else if(datalen < 0)
    datalen = 0;

  /* Allocate a 1024 byte receieve buffer at the beginning of
     dmabuffer, with proper alignment.  Also mark the buffer as
     uncacheable.                                               */
  recvbuf =
    (unsigned long *) (((((unsigned long)xdmabuffer)+31) & ~31) | 0xa0000000);

  /* Place the send buffer right after the receive buffer.  This
     automatically gives proper alignment and uncacheability.    */
  sendbuf =
    (unsigned long *) (((unsigned char *)recvbuf) + 1024);

  /* Make sure no DMA operation is currently in progress */
  maple_wait_dma();

  /* Set hardware DMA pointer to beginning of send buffer */
  MAPLE(0x04) = ((unsigned long)sendbuf) & 0xfffffff;

  /* Setup DMA data.  Each message consists of two control words followed
     by the request frame.  The first control word determines the port,
     the length of the request transmission, and a flag bit marking the
     last message in the burst.  The second control word specifies the
     address where the response frame will be stored.  If no response is
     received within the timeout period, -1 will be written to this address. */

  /* Here we know only one frame should be send and received, so
     the final message control bit will always be set...          */
  *sendbuf++ = datalen | (port << 16) | 0x80000000;

  /* Write address to receive buffer where the response frame should be put */
  *sendbuf++ = ((unsigned long)recvbuf) & 0xfffffff;

  /* Create the frame header.  The fields are assembled "backwards"
     because of the Maple Bus big-endianness.                       */
  *sendbuf++ = (cmd & 0xff) | (to << 8) | (from << 16) | (datalen << 24);

  /* Copy parameter data, if any */
  if(datalen > 0) {
    unsigned long *param = data;
    int i;
    for(i=0; i<datalen; i++)
      *sendbuf++ = *param++;
  }

  /* Frame is finished, and DMA list is terminated with the flag bit.
     Time to activate the DMA channel.                                */
  MAPLE(0x18) = 1;

  /* Wait for the complete cycle to finish, so that the response
     buffer is valid.                                            */
  maple_wait_dma();

  lock = 0;

  /* Return a pointer to the response frame */
  return recvbuf;
}

struct mapledev *locked_get_pads()
{
  return dev;
}

static int gun_mode=-1;
int gun_x=-1, gun_y=-1;

void maple_set_gun_mode(int m)
{
  gun_mode = m;
}

static struct mapledev *check_pads()
{
  unsigned int *buf = dmabuffer->sbuf;
  unsigned int *link;
  int i;

  if(lock)
    return NULL;
  lock = 1;

  if(MAPLE(0x18) & 1) {
    lock = 0;
    return NULL;
  }

  if(pending) {
    pending = 0;

    if(gun_pending) {
      unsigned int g = *(volatile unsigned int *)0xa05f80c4;
      gun_pending = 0;
      gun_x = g&0x3ff;
      gun_y = (g>>16)&0x3ff;
    } else {
      gun_x = gun_y = -1;
    }

    for(i=0; i<4; i++) {
      unsigned char *r = (unsigned char *)dmabuffer->rbuf[i];
      int fc = (r[6]<<8)|r[7];
      if(*r == MAPLE_RESPONSE_DEVINFO || *r == MAPLE_RESPONSE_DATATRF)
	dev[i].present = r[2] & 0x3f;
      if(*r == MAPLE_RESPONSE_DATATRF && r[3]>=3 &&
	 (fc & (dev[i].xfunc & ~MAPLE_FUNC_LIGHTGUN)) &&
	 (r[3]>=6 || !(dev[i].xfunc & MAPLE_FUNC_MOUSE))) {
	dev[i].cond.data[0] = ((unsigned int *)r)[2];
	if(dev[i].xfunc & MAPLE_FUNC_MOUSE) {
	  dev[i].cond.mouse.axis1 += ((int)((unsigned short *)r)[6]) - 0x200;
	  dev[i].cond.mouse.axis2 += ((int)((unsigned short *)r)[7]) - 0x200;
	  dev[i].cond.mouse.axis3 += ((int)((unsigned short *)r)[8]) - 0x200;
	  dev[i].cond.mouse.axis4 += ((int)((unsigned short *)r)[9]) - 0x200;
	  dev[i].cond.mouse.axis5 += ((int)((unsigned short *)r)[10]) - 0x200;
	  dev[i].cond.mouse.axis6 += ((int)((unsigned short *)r)[11]) - 0x200;
	  dev[i].cond.mouse.axis7 += ((int)((unsigned short *)r)[12]) - 0x200;
	  dev[i].cond.mouse.axis8 += ((int)((unsigned short *)r)[13]) - 0x200;
	} else
	  dev[i].cond.data[1] = ((unsigned int *)r)[3];
	dev[i].func = dev[i].xfunc;
	dev[i].ttl = 10;
      } else if(*r == MAPLE_RESPONSE_DEVINFO && r[3]>=28 &&
		(fc & (MAPLE_FUNC_CONTROLLER | MAPLE_FUNC_KEYBOARD |
			 MAPLE_FUNC_MOUSE))) {
	dev[i].xfunc = ((fc & MAPLE_FUNC_CONTROLLER)?
			(fc &(MAPLE_FUNC_CONTROLLER|MAPLE_FUNC_LIGHTGUN)) :
			((fc & MAPLE_FUNC_MOUSE)? MAPLE_FUNC_MOUSE :
			 MAPLE_FUNC_KEYBOARD));
	dev[i].xcond = (((dev[i].xfunc&0xff)<<24)|((dev[i].xfunc&0xff00)<<8))&
	  ~(MAPLE_FUNC_LIGHTGUN<<24);
	dev[i].func = 0;
	dev[i].ttl = 10;
	if(dev[i].xfunc & MAPLE_FUNC_MOUSE) {
	  dev[i].cond.mouse.axis1 = dev[i].cond.mouse.axis2 = 
	    dev[i].cond.mouse.axis3 = dev[i].cond.mouse.axis4 = 
	    dev[i].cond.mouse.axis5 = dev[i].cond.mouse.axis6 = 
	    dev[i].cond.mouse.axis7 = dev[i].cond.mouse.axis8 = 0;
	}	  
      } else if(*r == MAPLE_RESPONSE_DEVINFO) {
	dev[i].ttl = 10;
      } else if(*r != 0xff || !dev[i].ttl) {
	dev[i].func = dev[i].xfunc = 0;
	dev[i].ttl = dev[i].present = 0;
      } else
	--dev[i].ttl;
    }
  }

  for(i=0; i<4; i++) {
    unsigned int addr = i<<(6+8);
    addr |= (0x20|addr)<<8;
    link = buf;
    *buf++ = (i << 16);
    *buf++ = ((unsigned long)(void*)dmabuffer->rbuf[i]) & 0xfffffff;
    switch(dev[i].xfunc) {
     case MAPLE_FUNC_CONTROLLER:
     case MAPLE_FUNC_CONTROLLER|MAPLE_FUNC_LIGHTGUN:
     case MAPLE_FUNC_KEYBOARD:
     case MAPLE_FUNC_MOUSE:
       buf[-2] |= 1;
       *buf++ = MAPLE_COMMAND_GETCOND | addr | (1<<24);
       *buf++ = dev[i].xcond;
       break;
     default:
       *buf++ = MAPLE_COMMAND_DEVINFO | addr;
       break;
    }
  }
  if(gun_mode>=0 && gun_mode<4 && (dev[gun_mode].xfunc & MAPLE_FUNC_LIGHTGUN)){
    link = buf;
    /*    *(volatile unsigned int *)0xa05f80c4 = ~0; */
    *buf++ = (gun_mode << 16)|0x200;
    *buf++ = 0;
    *buf++ = 0;
    gun_pending = 1;
  }
  *link |= 0x80000000;
  MAPLE(0x04) = ((unsigned long)(void*)dmabuffer->sbuf) & 0xfffffff;
  MAPLE(0x18) = 1;

  pending = 1;

  lock = 0;

  return dev;
}

void maple_vbl_handler()
{
  check_pads();
}
