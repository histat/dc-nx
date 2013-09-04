
static __inline int getimask(void)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  return (sr >> 4) & 0x0f;
}

static __inline void setimask(int m)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  sr = (sr & ~0xf0) | (m << 4);
  __asm__("ldc %0,sr" : : "r" (sr));
}


#define G2_LOCK()						\
	int _s = getimask();					\
	setimask(15);						\
        while((*(volatile unsigned int *)0xa05f688c) & 32)

#define G2_UNLOCK()				\
	setimask(_s)

int pci_setup()
{
  int i;

  G2_LOCK();

  for(i=0; i<16; i++)
    if(*(volatile char *)(void *)(0xa1001400+i) != "GAPSPCI_BRIDGE_2"[i]) {
      G2_UNLOCK();
      return -1;
    }

  *(volatile unsigned int *)(void *)(0xa1001418) = 0x5a14a501;

  for(i=0; i<1000000; i++)
    ;

  if(*(volatile unsigned int *)(void *)(0xa1001418) != 1) {
    G2_UNLOCK();
    return -1;
  }

  *(volatile unsigned int *)(void *)(0xa1001420) = 0x1000000;
  *(volatile unsigned int *)(void *)(0xa1001424) = 0x1000000;
  *(volatile unsigned int *)(void *)(0xa1001428) = 0x1840000;
  *(volatile unsigned int *)(void *)(0xa1001414) = 1;
  *(volatile unsigned int *)(void *)(0xa1001434) = 1;

  *(volatile unsigned short *)(void *)(0xa1001606) = 0xf900;
  *(volatile unsigned int *)(void *)(0xa1001630) = 0;
  *(volatile unsigned char *)(void *)(0xa100163c) = 0;
  *(volatile unsigned char *)(void *)(0xa100160d) = 0xf0;
  *(volatile unsigned short *)(void *)(0xa1001604) |= 6;
  *(volatile unsigned int *)(void *)(0xa1001614) = 0x1000000;
  
  if((*(volatile unsigned char *)(void *)(0xa1001650))&1) {
    G2_UNLOCK();
    return -1;
  }

  G2_UNLOCK();
  return 0;
}

int gapspci_probe_bba()
{
  return pci_setup()>=0;
}

unsigned int pci_read32(int reg)
{
  unsigned int ret;
  G2_LOCK();
  ret = *(volatile unsigned int*)(void *)(0xa1001700+reg);
  G2_UNLOCK();
  return ret;
}

unsigned short pci_read16(int reg)
{
  unsigned short ret;
  G2_LOCK();
  ret = *(volatile unsigned short*)(void *)(0xa1001700+reg);
  G2_UNLOCK();
  return ret;
}

unsigned char pci_read8(int reg)
{
  unsigned char ret;
  G2_LOCK();
  ret = *(volatile unsigned char*)(void *)(0xa1001700+reg);
  G2_UNLOCK();
  return ret;
}

void pci_write32(int reg, unsigned int val)
{
  G2_LOCK();
  *(volatile unsigned int*)(void *)(0xa1001700+reg) = val;
  G2_UNLOCK();
}

void pci_write16(int reg, unsigned short val)
{
  G2_LOCK();
  *(volatile unsigned short*)(void *)(0xa1001700+reg) = val;
  G2_UNLOCK();
}

void pci_write8(int reg, unsigned char val)
{
  G2_LOCK();
  *(volatile unsigned char*)(void *)(0xa1001700+reg) = val;
  G2_UNLOCK();
}

