
#include "SDL/SDL.h"
//#include "main.fdh"

#include "shim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/basics.h"
#include "sound/sslib.h"
#include "sound/org.h"
#include "sound/pxt.h"
#include "watchdog.h"


extern int random(int min, int max);
extern void testblit(void);

static bool quitting = false;

SDL_Surface *screen;
void testblit(void)
{
SDL_Surface *image;

	screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);
	if (!screen) return;
	
	image = SDL_LoadBMP("data/MyChar.pbm");
	//image = SDL_LoadBMP("data/ItemImage.pbm");
	//image = SDL_LoadBMP("data/TextBox.pbm");
	if (!image) return;
	
	stat("Blitting...");

	SDL_Rect srcrect;
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = 16;
	srcrect.h = 16;
	
	SDL_Rect dstrect;
	dstrect.x = 5;
	dstrect.y = 5;
#if 1
	SDL_BlitSurface(image, &srcrect, screen, &dstrect);
#else
	SDL_BlitSurface(image, NULL, screen, NULL);
#endif
	//SDL_Flip(screen);
	
	//SDL_FillRect(screen, &srcrect, SDL_MapRGB(screen->format, 255, 0, 255));
	
	//DrawRect(screen, 220, 50, 260, 100);
	//SDL_Flip(screen);
	
	stat("Ok.");
}


void DrawRect(SDL_Surface *screen, int x1, int y1, int x2, int y2)
{
SDL_Rect rect;
uint32_t color = SDL_MapRGB(screen->format, 255, 0, 255);
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

#if 1
int SDL_main(int argc, char *argv[])
{
	set_console_visible(true);

	stat("Entering main loop");
	testblit();

#ifndef NOSERIAL
	wdResume();
#endif
	while(!quitting)
	{
		#ifndef NOSERIAL
			wdPet();
		#endif

		SDL_Event pie;
		if (SDL_PollEvent(&pie))
		{
		  //stat("Got event %d", pie.type);
		  stat("Got key %d", pie.key.keysym.sym);
			if(pie.key.keysym.sym == SDLK_F3)
			  quitting = true;
		}

		SDL_Delay(10);
		cursor_run();
		SDL_Flip(screen);
	}
	

	return 0;
}
#else

static const char *org_dir = "org/";
static const char *pxt_dir = "pxt/";
static const char *sndcache = "sndcache.pcm";
static const char *org_wavetable = "wavetable.dat";


#define NUM_SOUNDS		0x75
#define ORG_VOLUME		75

const char *org_names[] =
{
	NULL,
	"egg", "safety", "gameover", "gravity", "grasstown", "meltdown2", "eyesofflame",
	"gestation", "town", "fanfale1", "balrog", "cemetary", "plant", "pulse", "fanfale2",
	"fanfale3", "tyrant", "run", "jenka1", "labyrinth", "access", "oppression", "geothermal",
	"theme", "oside", "heroend", "scorching", "quiet", "lastcave", "balcony", "charge",
	"lastbattle", "credits", "zombie", "breakdown", "hell", "jenka2", "waterway", "seal",
	"toroko", "white", "azarashi", NULL
};

static void start_track(int songno)
{

char fname[MAXPATHLEN];

	if (songno == 0)
	{
		org_stop();
		return;
	}

	strcpy(fname, org_dir);
	strcat(fname, org_names[songno]);
	strcat(fname, ".org");
	
	if (!org_load(fname))
	{
		org_start(0);
	}
}

void music(int songno)
{

	org_stop();

	start_track(songno);
}

void test()
{
}

int SDL_main(int argc, char *argv[])
{
	int i = 1;
	set_console_visible(true);

	stat("Entering main loop");

	if (SSInit()) return 1;
	if (pxt_init()) return 1;
	
	if (pxt_LoadSoundFX(pxt_dir, sndcache, 0x75)) {
		staterr("Can't load\n");
		return 1;
	}

	if (org_init(org_wavetable, pxt_dir, ORG_VOLUME))
	{
		staterr("Music failed to initialize");
		return 1;
	}

	testblit();

	music(19);

#ifndef NOSERIAL
	wdResume();
#endif

	while(!quitting)
	{
		#ifndef NOSERIAL
			wdPet();
		#endif

		org_run();
		
		SDL_Event pie;
		if (SDL_PollEvent(&pie))
		{
			if (pie.type == SDL_KEYDOWN) {
				
				if(pie.key.keysym.sym == SDLK_F3)
					quitting = true;
				else if(pie.key.keysym.sym == SDLK_BTN_A)
					pxt_Play(-1, i, 1);
				else if(pie.key.keysym.sym == SDLK_BTN_B)
					org_stop();
				else if(pie.key.keysym.sym == SDLK_BTN_Y)
					music(random(1,42));
				else if(pie.key.keysym.sym == SDLK_BTN_X)
				    i++;
			}
		}

		SDL_Delay(10);
		cursor_run();
		SDL_Flip(screen);
	}

	pxt_freeSoundFX();
	SSClose();

	return 0;
}


#endif









