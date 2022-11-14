#ifdef USE_ARM

#include "shim.h"
#include <kos.h>


static int last_track;
static bool is_playing;
static bool enable_cdda;

static const char *song_names[] =
{
	NULL,
	"egg", "safety", "gameover", "gravity", "grasstown", "meltdown2", "eyesofflame",
	"gestation", "town", "fanfale1", "balrog", "cemetary", "plant", "pulse", "fanfale2",
	"fanfale3", "tyrant", "run", "jenka1", "labyrinth", "access", "oppression", "geothermal",
	"theme", "oside", "heroend", "scorching", "quiet", "lastcave", "balcony", "charge",
	"lastbattle", "credits", "zombie", "breakdown", "hell", "jenka2", "waterway", "seal",
	"toroko", "white", NULL
};

int org_init(const char *wavetable_fname, const char *drum_pxt_dir, int org_volume)
{
	last_track = 0;
	is_playing = false;
	enable_cdda = false;

	int first,last;
	CDROM_TOC toc;
	cdrom_read_toc(&toc, 0);

	if (toc) {

		first = TOC_TRACK(toc->first);
		last = TOC_TRACK(toc->last);
		stat("%x -> %x \n",first, last);

		if( (last-1) >= 41) {
			enable_cdda = true;
		}
	}

	stat("CD-DA play back  %s \n", enable_cdda?"enable":"disable");

	return 0;
}

bool org_start(int startbeat)
{
	is_playing = true;

	return 0;
}

static char *file_getname(const char *path) 
{
	const char *ret;
	ret = path;

	while (*path !='\0') {

		if ( *path ) {
			if (*path == '/') {
				ret = path + 1;
			}
		}

		path++;
	}

	return (char *)ret;
}

char org_load(char *fname)
{
	int i;
	int track;
	int len;
	char *buffer;
	char *p;

	if (!enable_cdda) return 1;

	stat("song name  = %s \n", fname);

	// cut org directory and ext
	buffer = file_getname(fname);

	p = buffer;
	while (*p != '\0') {
		if (*p == '.') {
			*p = '\0';
			break;
		}
		p++;
	}

	len = strlen(buffer);

	//stat("name = %s len = %d\n", buffer, len);

	for (i=1; i < 42; ++i) 
		if (!strncmp(buffer, song_names[i], len)) {

			track = i;

			stat("song tarck = %d\n", track);

			cdrom_cdda_play(track, track, 15, CD_CDDA);

			last_track = track;

			return 0;
		}


	return 1;
}

void org_fade(void)
{
	stat("org_fade");
}

bool org_is_playing(void)
{
	return is_playing;
}

void org_run(void)
{
	unsigned int stat[2];
	
	if (!enable_cdda) return;
	
	if (is_playing) {

		gdGdcGetDrvStat(stat);
		
		if (stat[0] == 1) {
			//staterr("GD drive status = %d\n", stat[0]);
			
			play_cdda_tracks(last_track, last_track, 15);
		}
	}

}

void org_stop(void)
{
	if (is_playing) 
	{
		is_playing = false;
		stop_cdda();
	}
}

#endif

