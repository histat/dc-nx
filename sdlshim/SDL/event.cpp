
#include "../shim.h"
#include "../dcsound.h"
#include "event.fdh"

char caption[32];

static const char *key_names[] =
{
	NULL,
	"up", "down", "right", "left",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"esc", "spc",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"
};

bool SDLS_EventInit(void)
{
	return 0;
}

void SDLS_EventQuit(void)
{
}

/*
void c------------------------------() {}
*/

static int sdl_func[12] ={
	SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8,SDLK_F9,
	SDLK_F10, SDLK_F11, SDLK_F12
};

static int sdl_num[10] ={
	SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
	SDLK_8, SDLK_9, SDLK_0
};

static int sdl_alph[26] ={
	SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h,
	SDLK_i, SDLK_j, SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p,
	SDLK_q, SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
	SDLK_y,	SDLK_z
};

static int sdl_key[4] ={
	SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP
};

static int joy_to_sdlk(int key)
{
	switch (key) {
	case JOY_UP: return SDLK_UP;
	case JOY_DOWN: return SDLK_DOWN;
	case JOY_RIGHT: return SDLK_RIGHT;
	case JOY_LEFT: return SDLK_LEFT;
	case JOY_RTRIGGER: return SDLK_a;
	case JOY_LTRIGGER: return SDLK_s;
	case JOY_A: return SDLK_z;
	case JOY_B: return SDLK_x;
	case JOY_X: return SDLK_q;
	case JOY_Y: return SDLK_w;
	case JOY_START: return SDLK_F3;
	case JOY_C: return SDLK_c;
	case JOY_Z: return SDLK_v;
	case JOY_D: return SDLK_d;
	}
	
	return SDLK_UNKNOWN;
}

static int key_to_sdlk(int key)
{
	switch (key) {
	case 0x04 ... 0x1d:	return sdl_alph[key - 0x04];
	case 0x1e ... 0x27:	return sdl_num[key - 0x1e];
	case 0x4f ... 0x52:	return sdl_key[key - 0x4f];
	case 0x59 ... 0x62:	return sdl_num[key - 0x59];
	case 0x29: return SDLK_ESCAPE;
//	case 0x2c: return SDLK_SPACE;
	case 0x3a ... 0x45:	return sdl_func[key - 0x3a];
	}

	return SDLK_UNKNOWN;
}

int SDL_PollEvent(SDL_Event *event)
{
	static  unsigned int tick = 0;
	unsigned int  tm = Timer() - tick;
	if (tm < USEC_TO_TIMER(1000000/60)) {
		return 0;
	}
#if 0
	update_polygon();
#endif
	
	int mask = getimask();
	setimask(15);
	update_audio();
	handleInput(locked_get_pads());
	setimask(mask);
	
	tick += tm;
	
	
	Event ev;

	if (PollEvent(ev))	{

		switch(ev.type) {
		case EVENT_JOYBUTTONDOWN:
		{
			int sdlk = joy_to_sdlk(ev.jbutton.button);

			if (sdlk != SDLK_UNKNOWN)
			{
				event->type = SDL_KEYDOWN;
				event->key.keysym.sym = sdlk;
				event->key.keysym.scancode = 0;

				return 1;
			}
		}
		break;

		case EVENT_JOYBUTTONUP:
		{
			int sdlk = joy_to_sdlk(ev.jbutton.button);
						
			if (sdlk != SDLK_UNKNOWN)
			{
				event->type = SDL_KEYUP;
				event->key.keysym.sym = sdlk;
				event->key.keysym.scancode = 0;

				return 1;
			}
		}
		break;

		case EVENT_KEYDOWN:
		{
			int sdlk = key_to_sdlk(ev.key.keycode);

#if !defined(NOSERIAL) && defined(__SDCARD__)
			if(ev.key.keycode == 0x46)
				screensave();
#endif
			
			if (sdlk != SDLK_UNKNOWN)
			{
				event->type = SDL_KEYDOWN;
				event->key.keysym.sym = sdlk;
				event->key.keysym.scancode = 0;

				return 1;
			}

		}
		break;

		case EVENT_KEYUP:
		{
			int sdlk = key_to_sdlk(ev.key.keycode);

			if (sdlk != SDLK_UNKNOWN)
			{
				event->type = SDL_KEYUP;
				event->key.keysym.sym = sdlk;
				event->key.keysym.scancode = 0;

				return 1;
			}
		}
		break;

		case EVENT_QUIT:
		{
			event->type = SDL_QUIT;
			
			return 1;

		}
		break;
		}
	}

	return 0;
}


const char *SDL_GetKeyName(SDLKey key)
{
	if (key >= 0 && key < SDLK_LAST)
		return key_names[key];
	
	return NULL;
}


void SDL_WarpMouse(uint16_t x, uint16_t y)
{
}


void SDL_WM_SetCaption(const char *title, const char *icon)
{
	strncpy(caption, title, sizeof(caption));
}


uint8_t SDL_GetAppState(void)
{
	return (SDL_APPACTIVE | SDL_APPINPUTFOCUS /*| SDL_APPMOUSEFOCUS*/);
}

/*
void c------------------------------() {}
*/


