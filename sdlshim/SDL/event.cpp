
#include "../shim.h"

#include "SDL.h"
#include "dcevent.h"
#include "scrnsave.h"

//static int last_keydown, last_keyup;
//static int lastmx, lastmy;
//static bool poll_again;
//static BList queue;


static const char *key_names[] =
{
	NULL,
	"up", "down", "right", "left",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"esc", "space",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"A btn", "B btn", "C btn", "X btn", "Y btn", "Z btn",
	"R-trig", "L-trig"
};


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
	case JOY_RTRIGGER: return SDLK_RTRIG;
	case JOY_LTRIGGER: return SDLK_LTRIG;
	case JOY_A: return SDLK_BTN_A;
	case JOY_B: return SDLK_BTN_B;
	case JOY_C: return SDLK_BTN_C;		
	case JOY_X: return SDLK_BTN_X;
	case JOY_Y: return SDLK_BTN_Y;
	case JOY_Z: return SDLK_BTN_Z;
	case JOY_START: return SDLK_F3; //fixed for menu
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
	case 0x2c: return SDLK_SPACE;
	case 0x3a ... 0x45:	return sdl_func[key - 0x3a];
	}

	return SDLK_UNKNOWN;
}

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

int SDL_PollEvent(SDL_Event *event)
{
    Event ev;

#ifndef USE_ARM
	update_audio();
#endif

    if (PollEvent(ev)) {

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

	case EVENT_MOUSEMOTION:
	{
	}
	break;
	    
	case EVENT_MOUSEBUTTONDOWN:
	{
	}
	break;
	    
	case EVENT_MOUSEBUTTONUP:
	{
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

void SDL_PushEvent(SDL_Event *event)
{
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
}

uint8_t SDL_GetAppState(void)
{
	return (SDL_APPACTIVE | SDL_APPINPUTFOCUS | SDL_APPMOUSEFOCUS);
}

/*
void c------------------------------() {}
*/

