
#ifndef USE_ARM

#include "../shim.h"
#include "audio.fdh"
#include <ronin/soundcommon.h>
#include <ronin/gddrive.h>


#define ADJUST_VOLUME(s, v)		(s = ((s*v) / SDL_MIX_MAXVOLUME))


EXTERN_C void *memcpy4s(void *s1, const void *s2, unsigned int n);

#define AUDIO_SIZE (RING_BUFFER_SAMPLES>>3)
signed short tmp_sound_buffer[AUDIO_SIZE] __attribute__((aligned (32)));


static void (*dc_callback)(void *userdata, uint8_t *stream, int len) = NULL;

static uint8_t dsstreamevent;
static uint32_t dsstreambytes;

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	stop_sound();
	do_sound_command(CMD_SET_BUFFER(3));
	do_sound_command(CMD_SET_STEREO(1));
	do_sound_command(CMD_SET_FREQ_EXP(FREQ_22050_EXP));

	
	obtained->freq = read_sound_int(&SOUNDSTATUS->freq);
	obtained->format = AUDIO_S16;
	obtained->channels = 2;

	dsstreambytes = read_sound_int(&SOUNDSTATUS->ring_length) / 2;
	obtained->samples = dsstreambytes;
	//obtained->samples = desired->samples;
	obtained->size = (obtained->samples * 2);
	//obtained->callback = desired->callback;
	dc_callback = desired->callback;

	dsstreamevent = 0;

	return 0;
}

void SDL_PauseAudio(int pause_on)
{
    if (!pause_on)
		start_sound();
    else
		stop_sound();
}

void SDL_LockAudio(void)
{
}

void SDL_UnlockAudio(void)
{
}

void SDL_CloseAudio(void)
{
    stop_sound();
}

/*
void c------------------------------() {}
*/

// adapted from the real SDL_MixAudio.
// handles only 16bpp LSB audio data.
void SDL_MixAudio(uint8_t *dst, const uint8_t *src, uint32_t len, int volume)
{
	int16_t sample1, sample2;
	int dst_sample;
	#define MAX_AUDIOVAL	((1<<(16-1))-1)
	#define MIN_AUDIOVAL	-(1<<(16-1))
	
	len /= 2;
	while(len--)
	{
		sample1 = (src[1] << 8) | src[0];
		ADJUST_VOLUME(sample1, volume);
		sample2 = (dst[1] << 8) | dst[0];
		src += 2;
		
		dst_sample = (sample1 + sample2);
		
		if (dst_sample > MAX_AUDIOVAL) dst_sample = MAX_AUDIOVAL;
		else if (dst_sample < MIN_AUDIOVAL) dst_sample = MIN_AUDIOVAL;
		
		dst[0] = dst_sample; dst_sample >>= 8;
		dst[1] = dst_sample;
		dst += 2;
	}
}

void update_audio()
{
    int n = read_sound_int(&SOUNDSTATUS->samplepos);
    int pos;

    if (n >= dsstreambytes) {
	
      if (dsstreamevent == 0) return;
	
	  dsstreamevent = 0;
	  pos = 0;
    } else {

      if (dsstreamevent == 1) return;

	  dsstreamevent = 1;
	  pos = dsstreambytes;
    }
    
    memset(tmp_sound_buffer, 0, sizeof(tmp_sound_buffer));

    (*dc_callback)(NULL, (uint8_t *)tmp_sound_buffer, 2*SAMPLES_TO_BYTES(dsstreambytes));
	
    memcpy4s(RING_BUF + pos, tmp_sound_buffer, SAMPLES_TO_BYTES(dsstreambytes));
}

#endif
