
#include "SDL.h"
#include "../dcsound.h"
#include "misc.fdh"

uint32_t SDL_GetTicks()
{
	return ronin_gettick();
}

void SDL_Delay(int ms)
{
	unsigned int t0 = SDL_GetTicks();

	while((SDL_GetTicks()-t0) < ms) {
	}
}

/*
void c------------------------------() {}
*/

const char *SDL_GetError(void)
{
	return "Unknown Error";
}
