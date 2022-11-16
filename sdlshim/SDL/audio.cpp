
#ifndef ENABLE_CDDA

#include "../shim.h"
#include "../audio.h"
#include "audio.fdh"

#define ADJUST_VOLUME(s, v)		(s = ((s*v) / SDL_MIX_MAXVOLUME))

#define AUDIO_SIZE (RING_BUFFER_SAMPLES>>3)
static signed short buffer[AUDIO_SIZE] __attribute__((aligned (32)));


static void (*dc_callback)(void *userdata, uint8_t *stream, int len) = NULL;

#define SAMPLERATE 22050
#define BUFSIZE (AUDIO_SIZE / 2)

static volatile int paused = 0;

void start_sound() {
  paused = 0;
}

void stop_sound() {
  paused = 1;
}

static kthread_t * sndthd = NULL;
static void *sndfill(void *arg);
void update_audio();

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	stop_sound();
	obtained->freq = SAMPLERATE;
	obtained->format = AUDIO_S16;
	obtained->channels = 2;

	obtained->samples = BUFSIZE;
	//obtained->samples = desired->samples;
	obtained->size = (obtained->samples * 2);
	//obtained->callback = desired->callback;
	dc_callback = desired->callback;

	audio_register_ringbuffer(AUDIO_FORMAT_16BIT, obtained->freq, BUFSIZE);
	sndthd = thd_create(0, &sndfill, NULL);

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

static void *sndfill(void *arg)
{
  (void*)arg;

  while (1) {
      update_audio();
  }
}

void update_audio()
{
    uint32_t *samples = (uint32_t *)buffer;
    memset(buffer, 0, sizeof(buffer));

    (*dc_callback)(NULL, (uint8_t *)samples, 2*SAMPLES_TO_BYTES(BUFSIZE));

    int numsamples = BUFSIZE;

    while (numsamples > 0) {
      unsigned int actual_written = audio_write_stereo_data(samples, numsamples);

      if (actual_written < numsamples) {
	numsamples -= actual_written;
	samples += actual_written;
	thd_sleep(10);
      } else {
	numsamples = 0;
      }
    }
}

#endif
