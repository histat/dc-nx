
#include "../shim.h"
#include "../dcsound.h"
#include "audio.fdh"

#define ADJUST_VOLUME(s, v)		(s = ((s*v) / SDL_MIX_MAXVOLUME))

void (*dc_callback)(void *userdata, uint8_t *stream, int len) = NULL;

extern "C" void *memcpy4s(void *s1, const void *s2, unsigned int n);

static uint32_t dsstreambytes;
static uint8_t dsstreamevent;

#define AUDIO_SIZE (RING_BUFFER_SAMPLES>>3)
static short tmp_sound_buffer[AUDIO_SIZE] __attribute__((aligned (32)));

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	int samples;

	stop_sound();
	do_sound_command(CMD_SET_BUFFER(3));
	do_sound_command(CMD_SET_STEREO(1));
	do_sound_command(CMD_SET_FREQ_EXP(FREQ_22050_EXP));
	memset(tmp_sound_buffer, 0, sizeof(tmp_sound_buffer));

	samples = read_sound_int(&SOUNDSTATUS->ring_length);

	samples /= 2;

	dsstreambytes = samples;

	obtained->freq = read_sound_int(&SOUNDSTATUS->freq);
	obtained->format = AUDIO_S16;
	obtained->channels = 2;
	obtained->samples = samples;
	obtained->size = (obtained->samples * 2);

	dc_callback = desired->callback;

	dsstreamevent = (uint8_t)-1;
	
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

static void streamwrite(uint32_t pos) {

	memset(tmp_sound_buffer, 0, sizeof(tmp_sound_buffer));
	
	(*dc_callback)(NULL, (uint8_t *)tmp_sound_buffer, 2*SAMPLES_TO_BYTES(dsstreambytes));
	
	memcpy4s(RING_BUF + pos, tmp_sound_buffer, SAMPLES_TO_BYTES(dsstreambytes));
}

void update_audio()
{
	handleInput(locked_get_pads());
	
	uint32_t pos = read_sound_int(&SOUNDSTATUS->samplepos);
  
	if (pos >= dsstreambytes) {
		if (dsstreamevent != 0) {
			dsstreamevent = 0;
			streamwrite(0);
		}
	} else {
		if (dsstreamevent != 1) {
			dsstreamevent = 1;
			streamwrite(dsstreambytes);
		}
	}
}
