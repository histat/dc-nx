#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "netif/bbaif.h" 
#include "netif/loopif.h"

static int syscall_info_flash(int sect, int *info)
{
  return (*(int (**)())0x8c0000b8)(sect,info,0,0);  
}

static int syscall_read_flash(int offs, void *buf, int cnt)
{
  return (*(int (**)())0x8c0000b8)(offs,buf,cnt,1);
}

static int flash_crc(const char *buf, int size)
{
  int i, c, n = -1;
  for(i=0; i<size; i++) {
    n ^= (buf[i]<<8);
    for(c=0; c<8; c++)
      if(n & 0x8000)
	n = (n << 1) ^ 4129;
      else
	n <<= 1;
  }
  return (unsigned short)~n;
}

static int flash_read_sector(int partition, int sec, char *dst)
{
  int s, r, n, b, bmb, got=0;
  int info[2];
  char buf[64];
  char bm[64];

  if((r = syscall_info_flash(partition, info))<0)
    return r;

  if((r = syscall_read_flash(info[0], buf, 64))<0)
    return r;

  if(memcmp(buf, "KATANA_FLASH", 12) ||
     buf[16] != partition || buf[17] != 0)
    return -2;

  n = (info[1]>>6)-1-((info[1] + 0x7fff)>>15);
  bmb = n+1;
  for(b = 0; b < n; b++) {
    if(!(b&511)) {
      if((r = syscall_read_flash(info[0] + (bmb++ << 6), bm, 64))<0)
	return r;
    }
    if(!(bm[(b>>3)&63] & (0x80>>(b&7))))
      if((r = syscall_read_flash(info[0] + ((b+1) << 6), buf, 64))<0)
	return r;
      else if((s=*(unsigned short *)(buf+0)) == sec &&
	      flash_crc(buf, 62) == *(unsigned short *)(buf+62)) {
	memcpy(dst+(s-sec)*60, buf+2, 60);
	got=1;
      }
  }

  return got;
}


static void tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = arg;
  sys_sem_signal(*sem);
}

void lwip_init()
{
  static int lwip_inited = 0;
  sys_sem_t sem;
  struct ip_addr ipaddr, netmask, gw;

  if(lwip_inited) return;
  sys_init();
  mem_init();
  memp_init();
  pbuf_init();
  netif_init();
  sem = sys_sem_new(0);
  tcpip_init(tcpip_init_done, &sem);
  sys_sem_wait(sem);
  sys_sem_free(sem);

  IP4_ADDR(&gw, 127,0,0,1); 
  IP4_ADDR(&ipaddr, 127,0,0,1);  
  IP4_ADDR(&netmask, 255,0,0,0);
  netif_set_default(netif_add(&ipaddr, &netmask, &gw, loopif_init, 
			      tcpip_input));

  if(gapspci_probe_bba()) {

    unsigned char sec[64];

    if(flash_read_sector(2,224,(char*)sec)==1 &&
       sec[0]=='S' && sec[1]=='E' && sec[2]=='G' && sec[3]=='A') {
      /* Broadband Passport settings */
      ipaddr.addr = *(u32_t*)(sec+8);
      netmask.addr = *(u32_t*)(sec+12);
      gw.addr = *(u32_t*)(sec+20);
    } else if(flash_read_sector(2,96,(char*)sec)==1 &&
	      sec[0]=='S' && sec[1]=='E' && sec[2]=='G' && sec[3]=='A') {
      /* Quake III Arena settings */
      ipaddr.addr = *(u32_t*)(sec+8);
      netmask.addr = *(u32_t*)(sec+12);
      gw.addr = *(u32_t*)(sec+20);
    } else {
      ipaddr.addr = netmask.addr = gw.addr = 0;
    }

    if(!ipaddr.addr) IP4_ADDR(&ipaddr, 192,168,1,2);
    if(!netmask.addr) netmask.addr = 0x00ffffff;
    if(!gw.addr) gw.addr = (ipaddr.addr & netmask.addr) | 0x01000000;
    
    netif_set_default(netif_add(&ipaddr, &netmask, &gw, bbaif_init,
				tcpip_input));

  }

  lwip_inited=1;
}
