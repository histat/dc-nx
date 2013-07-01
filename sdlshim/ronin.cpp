#include "shim.h"
#include "sdfs.h"

uint16_t *vram = NULL;

static unsigned char dc_screen[VRAM_SIZE] __attribute__((aligned (32)));

bool ronin_init()
{
	memset(dc_screen, 0, sizeof(dc_screen));
	
	vram = (uint16_t *)dc_screen;

#ifdef __SDCARD__
	sdfs_init();

	chdir("NX");
	
	mkdir("replay", 0);
#else
	// to get vmu_avail before reading vmu
	int mask = getimask();
	setimask(15);
	handleInput(locked_get_pads());
	setimask(mask);
#endif

	return 0;
}

void ronin_close()
{
#ifdef __SDCARD__
	sdfs_exit();
#endif
}

unsigned int ronin_gettick()
{
	static unsigned int count = 0;
	static unsigned int old = 0;
	unsigned int now = Timer();
  
	if (now != old) {
		unsigned int diff = now - old;
		unsigned int steps = (diff<<6) / (100000>>5);
		diff -= (steps*(100000>>5))>>6;
		old = now - diff;
		return (count += steps);
	}
  
	return count;
}

// ---

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
