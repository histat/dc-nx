
#ifndef _SHIM_H
#define _SHIM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include <kos.h>

#include "common/DBuffer.h"
#include "common/BList.h"
#include "SDL/SDL.h"

int init_hardware();
void close_hardware();
void update_audio();
extern void *screen_tx[];

extern uint16_t *vram;
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		240
#define SCREEN_PITCH		(SCREEN_WIDTH * 2)
#define VRAM_SIZE			(SCREEN_HEIGHT * SCREEN_PITCH)

#define FONT_WIDTH			8
#define FONT_HEIGHT			8
#define SCREEN_CHARS_WIDTH	(SCREEN_WIDTH/FONT_WIDTH)
#define SCREEN_CHARS_HEIGHT	(SCREEN_HEIGHT/FONT_HEIGHT)

#define SCREEN_BUFFER_SIZE 4


#define RING_BUFFER_SAMPLES 44100
#define SAMPLES_TO_BYTES(n) ((n)<<1)

extern FILE *SDLS_fopen(const char *fname, const char *mode);

extern void cursor_run(void);

#endif
