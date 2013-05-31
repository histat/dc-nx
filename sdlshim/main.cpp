
#include "SDL/SDL.h"
#include "main.fdh"

static bool quitting = false;

int SDL_main(int argc, char *argv[])
{
	stat("Entering main loop");
	testblit();
	
	while(!quitting)
	{
		SDL_Event pie;
		if (SDL_PollEvent(&pie))
		{
			stat("Got event %d", pie.type);
			quitting = true;
		}
		
		SDL_Delay(10);
	}
	
	return 0;
}


void testblit(void)
{
SDL_Surface *screen, *image;

	screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);
	if (!screen) return;
#if 1	
	image = SDL_LoadBMP("data/MyChar.pbm");
//	image = SDL_LoadBMP("smalfont.bmp");
//	image = SDL_LoadBMP("data/bkBlack.pbm");
#else
	// w=244 h=144 bpp=4
	image = SDL_LoadBMP("data/TextBox.pbm");
#endif
	
	
	if (!image) return;

	stat("Blitting...");
	
	SDL_Rect srcrect;
#if 1
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = 16;
	srcrect.h = 16;
#else
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = 300;
	srcrect.h = 200;
#endif
	
	SDL_Rect dstrect;
//	dstrect.x = 0;
//	dstrect.y = 0;

	dstrect.x = 100;
	dstrect.y = 100;
	
	SDL_BlitSurface(image, &srcrect, screen, &dstrect);
	SDL_Flip(screen);

//	SDL_FillRect(screen, &srcrect, SDL_MapRGB(screen->format, 255, 255, 255));
	
//	DrawRect(screen, 220, 50, 260, 100);
//	SDL_Flip(screen);
	
	stat("Ok.");
}


void DrawRect(SDL_Surface *screen, int x1, int y1, int x2, int y2)
{
SDL_Rect rect;
uint32_t color = SDL_MapRGB(screen->format, 0, 255, 0);
#define SCALE	1

	// top and bottom
	rect.x = x1 * SCALE;
	rect.y = y1 * SCALE;
	rect.w = ((x2 - x1) + 1) * SCALE;
	rect.h = SCALE;
	SDL_FillRect(screen, &rect, color);
	
	rect.y = y2 * SCALE;
	SDL_FillRect(screen, &rect, color);
	
	// left and right
	rect.y = y1 * SCALE;
	rect.w = SCALE;
	rect.h = ((y2 - y1) + 1) * SCALE;
	SDL_FillRect(screen, &rect, color);
	
	rect.x = x2 * SCALE;
	SDL_FillRect(screen, &rect, color);
}











