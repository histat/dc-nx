
#include "shim.h"

uint32_t SDL_GetTicks()
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);

	return (uint32_t)(tv.tv_sec*1000 + tv.tv_usec/1000);
}

void SDL_Delay(int ms)
{
#if 0
	unsigned int t0 = SDL_GetTicks();

	while((SDL_GetTicks()-t0) < ms)
		update_audio();
#endif
}

/*
void c------------------------------() {}
*/

const char *SDL_GetError(void)
{
	return "Unknown Error";
}
