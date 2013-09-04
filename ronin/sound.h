#ifndef _RONIN_SOUND_H
#define _RONIN_SOUND_H been_here_before

/*
 * Sound
 */
#include "common.h"
#define DC_SOUND_BS 9216
//#define DC_SOUND_BS 524288
//#define DC_SOUND_BS 6144

struct buffer
{
  int read_pos;
  int write_pos;
  int full;

  short data[ DC_SOUND_BS ]; // enough, I hope.
};
//FIXME: This raises the namespace pollution to new levels.
extern struct buffer buff;

#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(0xa0800000+SOUNDSTATUS_ADDR))

extern int sound_device_open;
extern int fillpos;

START_EXTERN_C
void init_arm();
void write_samples( const short *samples, int nbytes );
void do_sound_command(int cmd);
int read_sound_int(volatile int *p);
void stop_sound();
void start_sound();
END_EXTERN_C


#endif /* _RONIN_SOUNDCOMMON_H */
