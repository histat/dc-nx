
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
		close_hardware();
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
  if (init_hardware()) return 1;
  if (SDLS_EventInit()) return 1;
  if (console_init()) return 1;

  SDL_main(argc, argv);

  SDL_Quit();

  return 0;
}


