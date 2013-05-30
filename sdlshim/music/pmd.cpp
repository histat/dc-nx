
#include "shim.h"
#include <unistd.h>
#include <math.h>
#include "common/basics.h"
#include "common/BList.h"

#include "audio.h"
#include "pmd.h"
#include "pmd.fdh"

static stSong song;
static BList running_notes;

#define BUFFER_SAMPLES			(AUDIO_SAMPLE_RATE * 8)
static struct
{
	// pointer to the raw PCM sound data
	// 16-bit stereo audio
	uint8_t samples[BUFFER_SAMPLES*2];		// *2 for stereo
} final_buffer[2];

/*#define NUM_DRUMS		7
static struct
{
	uint8_t *samples;
	uint32_t length;
} drum[NUM_DRUMS];*/

/*static const char *drum_names[NUM_DRUMS] =
{
	"bass1", "bass2", "snare1", NULL, "hat1", "hat2", "symbal1"
};*/

static int curbuffer;
static bool buffer_needed;

static const int pitch_table[] =
{
	0x060F, 0x0674, 0x06D3, 0x0738,
	0x07A3, 0x081A, 0x089D, 0x0914,
	0x099D, 0x0A38, 0x0AD2, 0x0B7A
};


bool pmd_init(void)
{
	memset(&song, 0, sizeof(song));
	memset(&final_buffer, 0, sizeof(final_buffer));
	
	if (load_drums())
		return 1;
	
	//SSReserveChannel(PMD_CHANNEL);
	//SSSetVolume(PMD_CHANNEL, SDL_MIX_MAXVOLUME);
	stat("pmd player ready");
	return 0;
}

void pmd_close(void)
{
	/*SSAbortChannel(PMD_CHANNEL);
	
	for(int i=0;i<NUM_DRUMS;i++)
	{
		if (drum[i].samples)
			SDL_FreeWAV(drum[i].samples);
	}*/
	
	dump_running_notes();
}

/*
void c------------------------------() {}
*/

bool pmd_load(const char *fname)
{
FILE *fp;
int i, t;

	stat("----------------------");
	stat("pmd_load: opening '%s'", fname);
	
	fp = fopen(fname, "rb");
	if (!fp)
	{
		staterr("pmd_load: failed to open '%s'", fname);
		return 1;
	}
	
	if (!fverifystring(fp, "PMD"))
	{
		staterr("pmd_load: not a PMD file: '%s'", fname);
		fclose(fp);
		return 1;
	}
	
	fgetc(fp);		// I dunno
	fgetl(fp);		// I dunno
	song.ms_per_beat = fgetl(fp);	// music wait
	song.loop_start = fgetl(fp);
	song.loop_end = fgetl(fp);
	song.nnotes = fgetl(fp);
	
	stat("song wait: %d (0x%04x)", song.ms_per_beat, song.ms_per_beat);
	stat("nnotes: %d (0x%04x)", song.nnotes, song.nnotes);
	stat("loop: %d-%d", song.loop_start, song.loop_end);
	
	// original PiyoPiyo and Ikachan seem to play things slightly slower than specified,
	// although PiyoPiyoPlayer plays it as spec'd in the file.
	if (strstr(fname, "Buriki"))
		song.ms_per_beat += 10;
	else
		song.ms_per_beat += 5;
	
	// load instrument samples
	for(i=0;i<NUM_TRACKS;i++)
		load_instrument(fp, &song.track[i].instrument);
	
	for(i=0;i<TOTAL_TRACKS;i++)
		song.track[i].no = i;
	
	// drum volume
	song.drum_volume = fgetl(fp);
	
	// load music notes
	for(t=0;t<TOTAL_TRACKS;t++)
	{
		stTrack *track = &song.track[t];
		
		for(i=0;i<song.nnotes;i++)
		{
			uint32_t mask = fgetl(fp);
			
			track->note[i].notemask = (mask & 0xffffff);
			
			track->note[i].panning = (mask >> 24);
			if (!track->note[i].panning)
				track->note[i].panning = PAN_CENTER;
			else
				track->note[i].panning--;
		}
	}
	
	return 0;
}

// loads octave, icon, and wavetable for a track.
static void load_instrument(FILE *fp, stInstrument *ins)
{
int i;

	ins->octave = fgetc(fp);
	ins->icon = fgetc(fp);
	
	fgeti(fp);
	
	ins->notelength = fgetl(fp);
	ins->volume = fgeti(fp);
	
	for(i=0;i<10;i++)
		fgetc(fp);
	
	// read instrument waveform
	for(i=0;i<WAVE_LENGTH;i++)
		ins->wave[i] = fgetc(fp);
	
	// read envelope
	for(i=0;i<ENV_LENGTH;i++)
		ins->env[i] = fgetc(fp);
}

/*
void c------------------------------() {}
*/

static bool load_drums(void)
{
/*SDL_AudioSpec spec;
char fname[MAXPATHLEN];

	for(int i=0;i<NUM_DRUMS;i++)
	{
		if (!drum_names[i])
		{
			drum[i].samples = NULL;
			continue;
		}
		
		GetSoundFilename(drum_names[i], fname);
		if (!SDL_LoadWAV(fname, &spec, &drum[i].samples, &drum[i].length))
		{
			staterr("failed to open %s", fname);
			return 1;
		}
		
		if (spec.freq != SAMPLE_RATE || spec.format != AUDIO_U8)
		{
			staterr("unexpected audio format in %s", fname);
			return 1;
		}
	}*/
	
	return 0;
}


static void drum_begin(int note, int pan)
{
/*int drumno, volume;
	
	drumno = (note / 2);
	if (drumno >= NUM_DRUMS || !drum[drumno].samples)
	{
		staterr("invalid/silent drum note %d / drum %d", note, drumno);
		return;
	}
	
	// compute volume ratios
	if (note & 1)
		volume = (song.drum_volume * 70) / 100;
	else
		volume = song.drum_volume;
	
	double master_volume_ratio, volume_left_ratio, volume_right_ratio;
	ComputeVolumeRatios(volume, pan, &master_volume_ratio, &volume_left_ratio, &volume_right_ratio);
	
	// create queueable sound buffer
	stRunningNote *snd = new stRunningNote;
	snd->curpos = 0;
	
	snd->nsamples = drum[drumno].length;
	snd->samples = (int16_t *)malloc(snd->nsamples * sizeof(int16_t) * 2);
	
	// upgrade 8-bit to 16 and apply pan
	int outpos = 0;
	for(int i=0;i<snd->nsamples;i++)
	{
		double audioval = (int8_t)(drum[drumno].samples[i] - 0x80);
		
		audioval *= 256;
		if (audioval > 32767)  audioval = 32767;
		if (audioval < -32768) audioval = -32768;
		audioval *= master_volume_ratio;
		
		snd->samples[outpos++] = (int)(audioval * volume_left_ratio);
		snd->samples[outpos++] = (int)(audioval * volume_right_ratio);
	}
	
	running_notes.AddItem(snd);*/
}

/*
void c------------------------------() {}
*/

void pmd_start(void)
{
	song.beat = 0;
	song.samples = 0;
	song.samples_per_beat = MSToSamples(song.ms_per_beat);
	song.samples_left_in_beat = 0;
	
	running_notes.MakeEmpty();
	
	queue_and_start_buffer(0);
	queue_and_start_buffer(1);
	buffer_needed = false;
	curbuffer = 0;
	
	stat("Starting pmd playback: %d ms per beat, %d samples per beat", \
		song.ms_per_beat, song.samples_per_beat);
	
	pmd_set_volume(PMD_NORMAL_VOLUME);
	song.playing = true;
}

void pmd_stop(void)
{
	if (song.playing)
	{
		//SSAbortChannel(PMD_CHANNEL);
		dump_running_notes();
		song.playing = false;
	}
}

bool pmd_is_playing()
{
	return song.playing;
}

void pmd_run(void)
{
	if (!song.playing)
		return;
	
	/*debug("beat: %d", song.beat);
	debug("curbuffer: %d", curbuffer);
	debug("buffer_needed: %d", buffer_needed);
	debug("running_notes: %d", running_notes.CountItems());
	debug("wait: %d/%d", song.ms_per_beat, song.samples_per_beat);
	*/
	
	if (buffer_needed)
	{
		queue_and_start_buffer(curbuffer);
		curbuffer ^= 1;
		buffer_needed = false;
	}
	
	run_fade();
}

void pmd_buffer_finished(void *crap)	//int chan, void *userdata)
{
	buffer_needed = true;
}


static void queue_and_start_buffer(int bufferno)
{
	stat("Generating %d samples -> %d", BUFFER_SAMPLES, bufferno);
	
	uint8_t *buffer = final_buffer[bufferno].samples;
	generate_music(buffer, BUFFER_SAMPLES);
	
	audio_submit_block(buffer, BUFFER_SAMPLES);
	stat("Buffer %d now playing", bufferno);
	//SSEnqueueChunk(PMD_CHANNEL, buffer, BUFFER_SAMPLES, (void *)bufferno, pmd_buffer_finished);
}

// mix audio from any running notes into the output buffer,
// and start any new notes as we run across their sample-positions
static void generate_music(uint8_t *outbuffer, int len)
{
	// account for stereo
	len *= 2;
	
	int outpos = 0;
	while(outpos < len)
	{
		// start any notes that are supposed to begin on this sample
		if (--song.samples_left_in_beat <= 0)
		{
			for(int t=0;t<TOTAL_TRACKS;t++)
				start_notes(&song.track[t], song.beat);
			
			song.beat++;
			song.samples_left_in_beat = song.samples_per_beat;
			
			if (song.beat >= song.loop_end)
				song.beat = song.loop_start;
		}
		
		// mix
		int mix_left = 0;
		int mix_right = 0;
		for(int chan=0;;chan++)
		{
			stRunningNote *snd = (stRunningNote *)running_notes.ItemAt(chan);
			if (!snd) break;
			
			mix_left  += snd->samples[snd->curpos++];
			mix_right += snd->samples[snd->curpos++];
			
			if (snd->curpos >= snd->nsamples*2)
			{
				free_running_note(snd);
				running_notes.RemoveItem(chan);
				chan--;
			}
		}
		
		mix_left /= 3;
		mix_right /= 3;
		
		mix_left = (mix_left / 256) + 128;
		mix_right = (mix_right / 256) + 128;
		
		if (mix_left  < 0) mix_left = 0;
		if (mix_right < 0) mix_right = 0;
		if (mix_left  > 255) mix_left = 255;
		if (mix_right > 255) mix_right = 255;
		
		outbuffer[outpos++] = mix_left;
		outbuffer[outpos++] = mix_right;
	}
}

/*
void c------------------------------() {}
*/

static void start_notes(stTrack *track, int beat)
{
	uint32_t notemask = track->note[beat].notemask;
	
	if (notemask != 0)
	{
		//stat(" %d>>  %08x : %d", track->no, notemask, track->instrument.volume);
		
		for(int note=0;note<24;note++)
		{
			if (notemask & 1)
			{
				if (track->no == 3)
					drum_begin(note, track->note[beat].panning);
				else
					note_begin(note, track->note[beat].panning, &track->instrument);
			}
			
			notemask >>= 1;
		}
	}
}


static void note_begin(int note, int pan, stInstrument *ins)
{
	stRunningNote *snd = create_running_note();
	
	synthesize_note(note, pan, ins, snd);
	snd->curpos = 0;
	
	running_notes.AddItem(snd);
}

/*
void c------------------------------() {}
*/

static void synthesize_note(int note, int pan, stInstrument *ins, stRunningNote *snd)
{
int i;

	int notelength = ins->notelength;
	int octscaler = (1 << ins->octave);
	
	uint8_t *output_buffer = (uint8_t *)malloc(notelength);
	uint8_t *ptr = output_buffer;
	
	int wavepos = 0;
	
	for(i=0;i<notelength;i++)
	{
		uint8_t readpos = (wavepos / 256);
		int sample = (signed char)ins->wave[readpos];
		
		// equivalent to (i * (ENV_LENGTH / notelength)) but better precision
		sample *= ins->env[(i * ENV_LENGTH) / notelength];
		sample /= 128;
		
		*ptr = sample;
		
		// each pmd "octave" actually covers two musical octaves...so for
		// the upper half of the range repeat the lower notes but play them
		// twice as high.
		if (note < 12)
			wavepos += (pitch_table[note] * octscaler) / 16;
		else
			wavepos += (pitch_table[note - 12] * octscaler) / 8;
		
		ptr++;
	}
	
	// convert sound to 16-bit and add panning
	int total_samples = (notelength * 2);	// *2 for stereo - "total samples"
	snd->samples = (int16_t *)malloc(total_samples * 2 * sizeof(int16_t));
	snd->nsamples = notelength;			// number of "stereo samples" we'll tell sslib
	
	double master_volume_ratio, volume_left_ratio, volume_right_ratio;
	ComputeVolumeRatios(ins->volume, pan,
				&master_volume_ratio, &volume_left_ratio, &volume_right_ratio);
	
	int outpos = 0;
	
	for(i=0;i<notelength;i++)
	{
		double audioval = (int8_t)output_buffer[i];
		
		audioval *= 256;
		if (audioval > 32767)  audioval = 32767;
		if (audioval < -32768) audioval = -32768;
		audioval *= master_volume_ratio;
		
		snd->samples[outpos++] = (int)(audioval * volume_left_ratio);
		snd->samples[outpos++] = (int)(audioval * volume_right_ratio);
	}
	
	free(output_buffer);
}

/*
void c------------------------------() {}
*/

static void dump_running_notes(void)
{
	while(running_notes.CountItems())
		free_running_note((stRunningNote *)running_notes.RemoveItem(0));
}

/*
void c------------------------------() {}
*/
/*
static void draw_samples(signed short *samples, int nsamples)
{
int i;
double x;
double xrate;

	// draw backdrop
	FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
	const char *str = stprintf("xrate %.2f nsamples %d", xrate, nsamples);
	font_draw(4, 16, str, 5, &greenfont);
	
	DrawLine(0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2, 80, 192, 80);
	
	// get width rate
	xrate = (SCREEN_WIDTH / (double)nsamples);
	
	// samples to stereo samples
	nsamples *= 2;
	
	for(i=0;i<nsamples;i+=2)
	{
		plot_sample(samples[i], (int)x, 0, 255, 255);
		plot_sample(samples[i+1], (int)x, 0, 255, 0);
		x += xrate;
	}
}

static void plot_sample(signed short sample, int x, int r, int g, int b)
{
int y;
double yrate = (double)(SCREEN_HEIGHT / 2) / (double)32767;

	y = (SCREEN_HEIGHT / 2) + (sample * yrate);
	DrawPixel(x, y, r, g, b);
}
*/
/*
void c------------------------------() {}
*/

// set the song volume to the specified value immediately and cancel any fade.
void pmd_set_volume(int vol)
{
	song.volume = vol;
	song.desired_volume = vol;
	song.volume_timer = 0;
	
	//SSSetVolume(PMD_CHANNEL, song.volume);
}

// fade the song volume gradually either up or down to the specified volume.
void pmd_fade(int vol)
{
	song.desired_volume = vol;
}


static void run_fade(void)
{
	if (song.volume != song.desired_volume)
	{
		if (++song.volume_timer > PMD_FADE_RATE)
		{
			song.volume_timer = 0;
			song.volume += (song.volume < song.desired_volume) ? 1 : -1;
			
			//SSSetVolume(PMD_CHANNEL, song.volume);
		}
	}
}

/*
void c------------------------------() {}
*/

// given a volume and a panning value, it returns three values
// between 0 and 1.00 which are how much to scale:
//	the whole sound (volume_ratio)
//  just the left channel (volume_left_ratio)
//  just the right channel (volume_right_ratio)
static void ComputeVolumeRatios(int volume, int panning, double *volume_ratio, \
								double *volume_left_ratio, double *volume_right_ratio)
{
	*volume_ratio = ((double)volume / PMD_MAX_VOLUME);
	
	// get volume ratios for left and right channels (panning)
	if (panning < PAN_CENTER)
	{	// panning left (make right channel quieter)
		*volume_right_ratio = ((double)panning / PAN_CENTER);
		*volume_left_ratio = 1.00f;
	}
	else if (panning > PAN_CENTER)
	{	// panning right (make left channel quieter)
		*volume_left_ratio = ((double)(PAN_FULL_RIGHT - panning) / PAN_CENTER);
		*volume_right_ratio = 1.00f;
	}
	else
	{	// perfectly centered (both channels get the full volume)
		*volume_left_ratio = 1.00f;
		*volume_right_ratio = 1.00f;
	}
}

// converts a time in milliseconds to that same time length in samples
static int MSToSamples(int ms)
{
	return (int)(((double)AUDIO_SAMPLE_RATE / (double)1000) * (double)ms);
}

/*
void c------------------------------() {}
*/

static stRunningNote *create_running_note()
{
	stRunningNote *note = (stRunningNote *)malloc(sizeof(stRunningNote));
	note->samples = NULL;
	return note;
}

void free_running_note(stRunningNote *note)
{
	if (note->samples)
		free(note->samples);
	
	free(note);
}









