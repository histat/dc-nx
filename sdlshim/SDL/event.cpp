
#include "../shim.h"

#include "SDL.h"
#include <kos.h>

#include "scrnsave.h"


static int last_keydown, last_keyup;
static int lastmx, lastmy;
//static bool poll_again;
static BList queue;

static kthread_t * evthd = NULL;
static void * event(void *arg);

static int joy_to_sdlk(int key)
{
	switch (key) {
	case CONT_DPAD_UP:case CONT_DPAD2_UP: return SDLK_UP;
	case CONT_DPAD_DOWN: case CONT_DPAD2_DOWN: return SDLK_DOWN;
	case CONT_DPAD_RIGHT: case CONT_DPAD2_RIGHT: return SDLK_RIGHT;
	case CONT_DPAD_LEFT: case CONT_DPAD2_LEFT: return SDLK_LEFT;
	case CONT_A: return SDLK_BTN_A;
	case CONT_B: return SDLK_BTN_B;
	case CONT_C: return SDLK_BTN_C;
	case CONT_X: return SDLK_BTN_X;
	case CONT_Y: return SDLK_BTN_Y;
	case CONT_Z: return SDLK_BTN_Z;
	case CONT_START: return SDLK_F3; //fixed for menu
	}

	return SDLK_UNKNOWN;
}

const static unsigned short sdl_shift[] = {
    SDLK_LCTRL,SDLK_LSHIFT,SDLK_LALT,SDLK_UNKNOWN,
    SDLK_RCTRL,SDLK_RSHIFT,SDLK_RALT,SDLK_UNKNOWN,
};

const static unsigned short sdl_key[]= {
    SDLK_UNKNOWN, SDLK_UNKNOWN, SDLK_UNKNOWN, SDLK_UNKNOWN,
    /*4*/
    SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h,
    SDLK_i, SDLK_j, SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p,
    SDLK_q, SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
    SDLK_y,SDLK_z,
    /*1e*/
    SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_0,
    /*28*/	SDLK_RETURN, SDLK_ESCAPE, SDLK_BACKSPACE, SDLK_TAB, SDLK_SPACE, SDLK_MINUS, SDLK_PLUS, SDLK_LEFTBRACKET,
    SDLK_RIGHTBRACKET, SDLK_BACKSLASH , SDLK_UNKNOWN, SDLK_SEMICOLON, SDLK_QUOTE,
    /*35*/
     SDLK_UNKNOWN, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_CAPSLOCK,

    /*3a*/
    SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10, SDLK_F11, SDLK_F12,

    /*46*/
    SDLK_PRINT, SDLK_SCROLLOCK, SDLK_PAUSE, SDLK_INSERT, SDLK_HOME, SDLK_PAGEUP, SDLK_DELETE, SDLK_END, SDLK_PAGEDOWN,
    /*4f*/
    SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP,
    /*53*/
    SDLK_NUMLOCK, SDLK_KP_DIVIDE, SDLK_KP_MULTIPLY, SDLK_KP_MINUS, SDLK_KP_PLUS, SDLK_KP_ENTER,
    SDLK_KP1, SDLK_KP2, SDLK_KP3, SDLK_KP4, SDLK_KP5, SDLK_KP6,
    /*5f*/
    SDLK_KP7, SDLK_KP8, SDLK_KP9, SDLK_KP0, SDLK_KP_PERIOD, SDLK_UNKNOWN
};

static const char *key_names[] =
{
	NULL,
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"Enter", "esc", "<-", "Tab", "[  ]", "-","=","{","}",
	"\\", ";", "'",
	"<", ">", "?", "Caps",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Prt", "scr", "Pause", "Ins", "Home", "PagUp", "del", "end", "PgDn",
	"right", "left", "down", "up",
	"Lock", "[/]", "[*]", "[-]", "[+]", "[ent]",
	"[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[0]", "[.]",
	"A btn", "B btn", "C btn", "X btn", "Y btn", "Z btn",
	"R-trig", "L-trig"
};

bool SDLS_EventInit(void)
{
        last_keydown = last_keyup = SDLK_UNKNOWN;

	evthd = thd_create(0, &event, NULL);
	return 0;
}

void SDLS_EventQuit(void)
{
        while(queue.CountItems())
	  delete (SDL_Event *)queue.RemoveItem(queue.CountItems() - 1);

	thd_destroy(evthd);
}

/*
void c------------------------------() {}
*/

int SDL_PollEvent(SDL_Event *event)
{
	SDL_PumpEvents();
	
	if (event)
	{
	  SDL_Event *nextevt = (SDL_Event *)queue.RemoveItem((int32)0);
		if (nextevt)
		{
			*event = *nextevt;
			delete nextevt;
			return 1;
		}
	}

	return 0;
}

void SDL_PumpEvents()
{
}

void SDL_PushEvent(SDL_Event *event)
{
	SDL_Event *evt = new SDL_Event;
	*evt = *event;

	queue.AddItem(evt);
}


const char *SDL_GetKeyName(SDLKey key)
{
	if (key >= 0 && key < SDLK_LAST)
		return key_names[key];
	
	return NULL;
}


void SDL_WarpMouse(uint16_t x, uint16_t y)
{
SDL_Event event;

	event.type = SDL_MOUSEMOTION;
	event.motion.x = x;
	event.motion.y = y;
	event.motion.xrel = (x - lastmx);
	event.motion.yrel = (y - lastmy);
	lastmx = x;
	lastmy = y;

	SDL_PushEvent(&event);
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

static const int sdl_buttons[] = {
    CONT_A,
    CONT_B,
    CONT_X,
    CONT_Y,
    CONT_START,
    CONT_C,
    CONT_D,
    CONT_Z,
};

static const int sdl_dpad[] = {
    CONT_DPAD_UP,
    CONT_DPAD_DOWN,
    CONT_DPAD_LEFT,
    CONT_DPAD_RIGHT,
    CONT_DPAD2_UP,
    CONT_DPAD2_DOWN,
    CONT_DPAD2_LEFT,
    CONT_DPAD2_RIGHT,
};

static cont_state_t joystick;

void JoystickUpdate() {
	maple_device_t *dev;
    cont_state_t *state, *prev_state;
	unsigned int buttons, i, changed;
	SDL_Event event;

	dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

	if(!(state = (cont_state_t *)maple_dev_status(dev))) {
	  return;
	}

	buttons = state->buttons;
	prev_state = &joystick;
	changed = buttons ^ prev_state->buttons;

	for(i = 0; i < sizeof(sdl_dpad) / sizeof(sdl_dpad[0]); ++i) {
		if(changed & sdl_dpad[i]) {
		  event.type = (buttons & sdl_dpad[i]) ? SDL_KEYDOWN :SDL_KEYUP;
		  event.key.keysym.sym = joy_to_sdlk(sdl_dpad[i]);
		  event.key.keysym.scancode = 0;

		  SDL_PushEvent(&event);
		}
	}

	for(i = 0; i < sizeof(sdl_buttons) / sizeof(sdl_buttons[0]); ++i) {
		if(changed & sdl_buttons[i]) {
		  event.type = (buttons & sdl_buttons[i]) ? SDL_KEYDOWN :SDL_KEYUP;
		  int sdlk = joy_to_sdlk(sdl_buttons[i]);

		  event.key.keysym.sym = sdlk;
		  event.key.keysym.scancode = 0;

		  SDL_PushEvent(&event);
		}
	}
	bool chaged = false;
	if(state->joyx != prev_state->joyx || state->joyy != prev_state->joyy) {
		event.motion.x = state->joyx;
		event.motion.y = state->joyy;
		event.motion.xrel = (event.motion.x - lastmx);
		event.motion.yrel = (event.motion.y - lastmy);
		//chaged = true;
	}

	if(state->rtrig != prev_state->rtrig) {
	  if ((state->rtrig - prev_state->rtrig) > 128) {
	    event.type = SDL_KEYDOWN;
	    event.key.keysym.sym = SDLK_RTRIG;
	    event.key.keysym.scancode = 0;

	    SDL_PushEvent(&event);
	  } else if(state->rtrig == 0) {
	    event.type = SDL_KEYUP;
	    event.key.keysym.sym = SDLK_RTRIG;
	    event.key.keysym.scancode = 0;

	    SDL_PushEvent(&event);
	  }
	}
	if(state->ltrig != prev_state->ltrig) {
	  if((state->ltrig - prev_state->ltrig) > 128) {
	    event.type = SDL_KEYDOWN;
	    event.key.keysym.sym = SDLK_LTRIG;
	    event.key.keysym.scancode = 0;

	    SDL_PushEvent(&event);
	  }  else if(state->ltrig == 0) {
	      event.type = SDL_KEYUP;
	      event.key.keysym.sym = SDLK_LTRIG;
	      event.key.keysym.scancode = 0;

	      SDL_PushEvent(&event);
	  }
	}

	if(state->joy2x != prev_state->joy2x || state->joy2y != prev_state->joy2y) {
		event.motion.x = state->joy2x;
		event.motion.y = state->joy2y;
		event.motion.xrel = (event.motion.x - lastmx);
		event.motion.yrel = (event.motion.y - lastmy);
		//chaged = true;
	}
	if(chaged) {
	  event.type = SDL_MOUSEMOTION;
	  SDL_PushEvent(&event);

	  lastmx = event.motion.x;
	  lastmy = event.motion.y;
	}

	joystick = *state;
}

static void keyboard_update(void) {
    static kbd_state_t old_state;
    kbd_state_t	*state;
    maple_device_t *dev;
    int shiftkeys;
    int i;
    SDL_Event event;

    if(!(dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD)))
        return;

    state = (kbd_state_t*)maple_dev_status(dev);

    if(!state)
        return;

    shiftkeys = state->shift_keys ^ old_state.shift_keys;
    for(i = 0; i < sizeof(sdl_shift); ++i) {
        if((shiftkeys >> i) & 1) {
	    event.type = ((state->shift_keys >> i) & 1) ? SDL_KEYDOWN :SDL_KEYUP;
	    event.key.keysym.sym = sdl_shift[i];
	    event.key.keysym.scancode = 0;
	    SDL_PushEvent(&event);
        }
    }

    for(i = 0; i < sizeof(sdl_key)/sizeof(sdl_key[0]); ++i) {
        if(state->matrix[i] != old_state.matrix[i]) {
	  int key = sdl_key[i];
            if(key != SDLK_UNKNOWN) {
		event.type = state->matrix[i] ? SDL_KEYDOWN :SDL_KEYUP;

		event.key.keysym.sym = key;

		event.key.keysym.scancode = 0;

		SDL_PushEvent(&event);
            }
        }
    }

    old_state = *state;
}


void handleInput()
{
  JoystickUpdate();
  keyboard_update();
}


static void * event(void *arg) {
  (void)arg;

  while(1)  {
    handleInput();
    thd_sleep(10);
  }
}
