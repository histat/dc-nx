
#include "shim.h"

uint32_t SDL_GetTicks()
{
	return timer_ms_gettime64();
}

void SDL_Delay(int ms)
{
	thd_sleep(ms);
}

/*
void c------------------------------() {}
*/

const char *SDL_GetError(void)
{
	return "Unknown Error";
}
