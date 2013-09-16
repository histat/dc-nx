
#include "SDL/SDL.h"
#include <ronin/sound.h>
#include <ronin/soundcommon.h>
#include "main.fdh"

static bool quitting = false;

#if 1

extern bool vmfile_search(const char *fname, int *vm);

int SDL_main(int argc, char *argv[])
{
	int vm;
	unsigned int size;
	uint8_t *data;
	char *srcfile = "profile3.dat";
	
	if(!vmfile_search(srcfile, &vm))
		vm = 1;

	FILE *fp;
	fp = fopen(srcfile, "rb");
	if (!fp)
	{
		stat("failed to open!");
		return 1;
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	data = (uint8_t*)malloc(size);
	
	if(!data) {
		fclose(fp);
		return 1;
	}

	memset(data, 0, size);

	fread(data, 1, size, fp);
	fclose(fp);

	save_to_vmu(vm, srcfile, (const char*)data, size);

	free(data);
	return 0;
}
#else

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


int SDL_main(int argc, char *argv[])
{
	stat("Entering main loop");
	testblit();

	if (SSInit()) return 1;
	if (pxt_init()) return 1;
	
	if (pxt_LoadSoundFX(pxt_dir, sndcache, 0x75)) {
		printf("Can't load\n");
		return 1;
	}

	if (org_init(org_wavetable, pxt_dir, ORG_VOLUME))
	{
		staterr("Music failed to initialize");
		return 1;
	}

	music(19);

	while(!quitting)
	{
		org_run();
		
		SDL_Event pie;
		if (SDL_PollEvent(&pie))
		{
			if (pie.type == SDL_KEYDOWN) {
				
				if(pie.key.keysym.sym == SDLK_F3)
					quitting = true;
				else if(pie.key.keysym.sym == SDLK_BTN_A)
					pxt_Play(-1, 1, 1);
				else if(pie.key.keysym.sym == SDLK_BTN_B)
					org_stop();
				else if(pie.key.keysym.sym == SDLK_BTN_Y)
					music(random(1,42));
			}
		}
	}


	pxt_freeSoundFX();
	SSClose();

	return 0;
}
#endif










