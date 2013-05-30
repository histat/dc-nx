
#ifndef _SDL_H
#define _SDL_H

#define __SDLSHIM__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "init.h"
#include "screen.h"
#include "event.h"
#include "audio.h"

// init
int SDL_Init(int what);
void SDL_Quit(void);

// screen
SDL_Surface *SDL_SetVideoMode(int width, int height, int bitsperpixel, uint32_t flags);
void SDL_Flip(SDL_Surface *screen);
SDL_Surface *SDL_LoadBMP(const char *fname);
int SDL_SetColorKey(SDL_Surface *surface, uint32_t flag, uint32_t key);
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect_in, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect);
void SDL_FreeSurface(SDL_Surface *sfc);
SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface *SDL_DisplayFormat(SDL_Surface *sfc);
uint32_t SDL_MapRGB(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b);
int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color);
void SDL_ShowCursor(int enable);
int SDL_SetAlpha(SDL_Surface *surface, uint32_t flag, uint8_t alpha);

// event
int SDL_PollEvent(SDL_Event *event);
void SDL_PumpEvents();
void SDL_PushEvent(SDL_Event *event);
const char *SDL_GetKeyName(SDLKey key);
void SDL_WarpMouse(uint16_t x, uint16_t y);
void SDL_WM_SetCaption(const char *title, const char *icon);
uint8_t SDL_GetAppState(void);

// audio
int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
void SDL_PauseAudio(int pause_on);
void SDL_LockAudio(void);
void SDL_UnlockAudio(void);
void SDL_CloseAudio(void);
void SDL_MixAudio(uint8_t *dst, const uint8_t *src, uint32_t len, int volume);

// misc
uint32_t SDL_GetTicks();
void SDL_Delay(int ms);
const char *SDL_GetError(void);

// SDLS
FILE *SDLS_fopen(const char *filename, const char *mode);


int SDL_main(int argc, char *argv[]);
#define main	SDL_main


#define MAXPATHLEN 255

#define atexit _atexit

int _atexit(void (*function)(void));

#endif

