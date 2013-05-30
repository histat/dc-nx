
#ifndef _AUDIO_H
#define _AUDIO_H

#define SDL_MIX_MAXVOLUME			128

typedef struct
{
	int freq;
	uint16_t format;
	uint8_t channels;
	uint8_t silence;
	uint16_t samples;
	uint32_t size;
	void (*callback)(void *userdata, uint8_t *stream, int len);
	void *userdata;
} SDL_AudioSpec;

#define AUDIO_S16		0x01

#endif
