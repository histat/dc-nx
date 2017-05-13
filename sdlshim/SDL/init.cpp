
#include <ronin/ronin.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "../shim.h"
#include "init.fdh"

#undef	main
bool did_quit = false;


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
	exit(1);
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
	
	SDL_main(argc, argv);

	SDL_Quit();

	return 0;
}


