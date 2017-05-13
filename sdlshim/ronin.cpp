#include "shim.h"
#include "scrnsave.h"
#ifdef USE_ARM
#include <ronin/soundcommon.h>
#include "arm/arm_pxt_code.h"
#endif

EXTERN_C void *memcpy4(void *s1, const void *s2, unsigned int n);


uint16_t *vram = NULL;
void *screen_tx[SCREEN_BUFFER_SIZE] = {NULL};

static unsigned char dc_screen[VRAM_SIZE] __attribute__((aligned (32)));

bool ronin_init()
{
    memset(dc_screen, 0, sizeof(dc_screen));
    
    vram = (uint16_t *)dc_screen;
    
    *(volatile unsigned int*)(0xa05f80e4) = SCREEN_WIDTH >> 5; //for stride
    
    for (int i=0; i<SCREEN_BUFFER_SIZE; i++)
	    screen_tx[i] = ta_txalloc(VRAM_SIZE);

#ifdef USE_ARM
	memcpy4((void*)0xa0800000+EXPANSION_BASE_ADDR, arm_pxt_code, sizeof(arm_pxt_code));
#endif


#ifdef __SDCARD__
	sd_init();
#endif

    return 0;
}

void ronin_close()
{
#ifdef USE_ARM
	stop_cdda();
#endif

#ifdef __SDCARD__
	sd_exit();
#endif
}

int gettimeofday (struct timeval *tv, void *tz)
{
  static unsigned long last_tm = 0;
  static unsigned long tmhi = 0;
  unsigned long tmlo = Timer();
  if (tmlo < last_tm)
    tmhi++;

  unsigned long long usecs = 
    ((((unsigned long long)tmlo)<<11)|
     (((unsigned long long)tmhi)<<43))/100;

  tv->tv_usec = usecs % 1000000;
  tv->tv_sec = usecs / 1000000;

  last_tm = tmlo;
  return 0;
}

int system(const char *command)
{
	return -1;
}


int unlink(char *pathname)
{
	
	return -1; 
}


int rename(const char *oldpath, const char *newpath){
	
	return -1;
}
#if 0
char* tmpnam(const char *s)
{
	return NULL;
}
#endif
clock_t clock()
{
	return 0;
}

