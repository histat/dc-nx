
#ifndef _PMD_H
#define _PMD_H

#define PMD_CHANNEL		12

// constant instrument sample lengths
#define WAVE_LENGTH		256
#define ENV_LENGTH		64

// # of channels, not counting the "P" drum channel.
#define NUM_TRACKS		3
#define TOTAL_TRACKS	4		// total tracks including the drum channel

// maximum possible volume of an instrument settable by PiyoPiyo
#define PMD_MAX_VOLUME		300

// SDL volumes for the SSLib channel, usable for music fading.
// the music is prescaled to 1/3 volume before sending to SSLib
// (doing it that way prevents clipping on loud songs), and so
// saying SDL_MIX_MAXVOLUME here does not really mean the song will
// be played at maximum volume/as loud as the SFX.
#define PMD_NORMAL_VOLUME	128//SDL_MIX_MAXVOLUME
#define PMD_QUIET_VOLUME	(128 / 3)
#define PMD_ZERO_VOLUME		0

#define PMD_FADE_RATE		1

// which panning value is perfectly centered
// in the file 1 is full left, 7 is full right, 4 is centered,
// and 0 is unspecified (which plays the same as centered).
// during load the panning is remapped to be 0-based.
// 1, 2, 3, *4*, 5, 6, 7
// 0, 1, 2, *3*, 4, 5, 6
#define PAN_CENTER		3
#define PAN_FULL_RIGHT	(PAN_CENTER + PAN_CENTER)

// valid octaves are 0-5.
#define NOTES_PER_OCTAVE	12
#define NUM_NOTES			(6 * NOTES_PER_OCTAVE)	// range of note values: 00-this.

#define MAX_SONG_LENGTH		5000


// this handles the actual synthesis
struct stNoteChannel
{
	signed short *outbuffer;
	
	// position inside outbuffer (not the same as samples_so_far because org module outputs stereo.
	// outpos counts data, samples_so_far counts samples. samples_so_far=outpos*2)
	int outpos;				// incs by 2 for each samples_so_far, one for left ch one for right ch
	int samples_so_far;		// number of samples generated so far into outbuffer
	
	double phaseacc;		// current read position inside wavetable sample
	double sample_inc;		// speed at which to iterate over the wavetable waveform
	
	// for drums
	double master_volume_ratio, volume_left_ratio, volume_right_ratio;
	
	int volume;				// last volume value sent to note_gen
	int panning;			// last panning value sent to note_gen
	
	int number;				// the channel number of this channel
};

// a single synthesized note
struct stRunningNote
{
	/*stRunningNote()
	{
		samples = NULL;
	}
	
	~stRunningNote()
	{
		if (samples)
			free(samples);
	}*/
	
	// 16-bit stereo audio
	int16_t *samples;
	int nsamples;
	int curpos;
};

// wavetable data loaded from file for synthesis of instruments
struct stInstrument
{
	uint8_t octave;
	uint8_t icon;			// note icon shown in PiyoPiyoEditor
	
	int8_t wave[WAVE_LENGTH];
	uint8_t env[ENV_LENGTH];
	
	uint32_t notelength;	// total samples of wave/each note
	int volume;				// volume of this track
};

struct stNote
{
	uint32_t notemask;		// which notes should play
	uint8_t panning;
};

struct stTrack
{
	stInstrument instrument;
	stNote note[MAX_SONG_LENGTH];
	int no;
};


struct stSong
{
	bool playing;
	int beat;				// current note position
	int ms_per_beat;		// aka "music wait"
	
	// for fading
	int volume;
	int desired_volume;
	int volume_timer;
	
	int loop_start;
	int loop_end;
	
	int samples;			// total samples that have been generated
	int samples_per_beat;
	int samples_left_in_beat;
	int drum_volume;
	
	stTrack track[TOTAL_TRACKS];
	int nnotes;
};

void pmd_buffer_finished(void *crap);

#endif

