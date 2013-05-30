
#ifndef _EVENT_H
#define _EVENT_H

#include "SDL_keysym.h"

typedef struct
{
	uint8_t sym;
	uint8_t scancode;
} SDL_keysym;

typedef struct
{
	uint8_t type;
	SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef struct
{
	uint8_t type;
	uint16_t x, y;
	int16_t xrel, yrel;
} SDL_MouseMotionEvent;

typedef struct
{
	uint8_t type;
	uint8_t button;
	uint16_t x, y;
} SDL_MouseButtonEvent;

typedef struct
{
	uint8_t type;
} SDL_QuitEvent;

typedef union
{
	uint8_t type;
	SDL_KeyboardEvent key;
	SDL_MouseMotionEvent motion;
	SDL_MouseButtonEvent button;
	SDL_QuitEvent quit;
} SDL_Event;


typedef enum
{
	SDL_KEYDOWN = 1,
	SDL_KEYUP,
	SDL_MOUSEMOTION,
	SDL_MOUSEBUTTONDOWN,
	SDL_MOUSEBUTTONUP,
	SDL_QUIT
} SDL_EventType;


#define SDL_APPMOUSEFOCUS		0x01
#define SDL_APPINPUTFOCUS		0x02
#define SDL_APPACTIVE			0x04

#endif

