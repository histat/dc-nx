
#include <ronin/ronin.h>
#include "SDL.h"
#include "init.fdh"

#undef	main
bool did_quit = false;

static void (*end_call)(void) = NULL;

int _atexit (void (*function)(void))
{
	end_call = function;
	return 0;
}

int SDL_Init(int what)
{
	return 0;
}

void SDL_Quit(void)
{
	if (!did_quit)
	{
		did_quit = true;
		
		SDLS_EventQuit();
		SDLS_CloseScreen();
		console_close();
		ronin_close();
	} 

#ifdef NOSERIAL
	(*(void(**)(int))0x8c0000e0)(1);
	while (1) { }
#else
	printf("call %s\n", __func__);
#endif
}

/*
void c------------------------------() {}
*/

int main(int argc, char *argv[])
{
#ifndef NOSERIAL
	serial_init(57600);
	usleep(20000);
	printf("Serial OK\n");
#endif

	cdfs_init();
	maple_init();
	dc_setup_ta();
	init_arm();

	if (SDLS_EventInit()) return 1;
	if (ronin_init()) return 1;
	if (console_init()) return 1;
	
	atexit(SDL_Quit);
#if 0
	return SDL_main(argc, argv);
#else
	SDL_main(argc, argv);

	(*end_call)();
#endif
}





