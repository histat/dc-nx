#include "shim.h"
#include "sdfs.h"
#include "ronin.fdh"

uint16_t *vram = NULL;

static unsigned char dc_screen[VRAM_SIZE] __attribute__((aligned (32)));

bool ronin_init()
{
	memset(dc_screen, 0, sizeof(dc_screen));
	
	vram = (uint16_t *)dc_screen;

#ifdef __SDCARD__
	
	chdir("NX");

	mkdir("replay", 0);
	
#else
	
	vmu_init();
#endif

	return 0;
}

void ronin_close()
{
}

// ---

extern "C" void vsync_event()
{
	SSRunMixer();
}

// ---

int gettimeofday(struct timeval *tp, struct timezone *tz)
{
  static unsigned long last_tm = 0;
  static unsigned long tmhi = 0;
  unsigned long tmlo = Timer();
  if (tmlo < last_tm)
    tmhi++;

  unsigned long long usecs = 
    ((((unsigned long long)tmlo)<<11)|
     (((unsigned long long)tmhi)<<43))/100;

  tp->tv_usec = usecs % 1000000;
  tp->tv_sec = usecs / 1000000;

  last_tm = tmlo;
  return 0;
}

int system(const char *command)
{
	return -1;
}
#ifndef __SDCARD__
int remove(const char *pathname)
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
#endif
char* tmpnam(const char *s)
{
	return NULL;
}

clock_t clock()
{
	return 0;
}
