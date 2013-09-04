#include <string.h> //FIXME: External dependecy.
#include "soundcommon.h"
#include "sound.h"
#include "report.h"

#include "arm_sound_code.h"

void *memcpy4(void *s1, const void *s2, unsigned int n)
{
  unsigned int *p1 = s1;
  const unsigned int *p2 = s2;
  n+=3;
  n>>=2;
  while(n--)
    *p1++ = *p2++;
  return s1;
}

void *memcpy4s(void *s1, const void *s2, unsigned int n)
{
  unsigned int *p1a = s1;
  unsigned int *p1b = (void*)(((char *)s1)+SAMPLES_TO_BYTES(STEREO_OFFSET));
  const unsigned int *p2 = s2;
  n+=3;
  n>>=2;
  while(n--) {
#if SAMPLE_MODE == 0
    unsigned int a = *p2++;
    unsigned int b = *p2++;
    *p1a++ = (a & 0xffff) | ((b & 0xffff)<<16);
    *p1b++ = ((a & 0xffff0000)>>16) | (b & 0xffff0000);
#else
#error 8 bit stereo not implemented...
#endif
  }
  return s1;
}

void *memset4(void *s, int c, unsigned int n)
{
  unsigned int *p = s;
  n+=3;
  n>>=2;
  while(n--)
    *p++ = c;
  return s;  
}

void init_arm()
{
  int i;

#if 0  
  *(volatile unsigned int *)(0xa05f6934) = 0x00000002;
#endif

  //TODO: Providing your own ARM code as an argument might be nice.
  *((volatile unsigned long *)(void *)0xa0702c00) |= 1;
  memset4((void*)0xa0800000, 0, 2*1024*1024);
  memcpy4((void*)0xa0800000, arm_sound_code, sizeof(arm_sound_code));
  *((volatile unsigned long *)(void *)0xa0702c00) &= ~1;
  //FIXME: Some sort of sleep here would be cleaner...
  for(i=0; i<0x200000; i++);
}

//int fillpos;

int read_sound_int(volatile int *p)
{
  while((*((volatile int *)(void *)0xa05f688c))&32);
  return *p;
}

static void write_sound_int(volatile int *p, int v)
{
  while((*((volatile int *)(void *)0xa05f688c))&32);
  *p = v;
}

static void wait_sound_command(int n)
{
  while(read_sound_int(&SOUNDSTATUS->cmdstatus)!=n);
}

void do_sound_command(int cmd)
{
  wait_sound_command(0);
  write_sound_int(&SOUNDSTATUS->cmd, cmd);
  write_sound_int(&SOUNDSTATUS->cmdstatus, 1);
  wait_sound_command(2);
  write_sound_int(&SOUNDSTATUS->cmdstatus, 0);
  wait_sound_command(0);
}

void start_sound()
{
  while(read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY) {
    memset4((void*)RING_BUF, 0, SAMPLES_TO_BYTES(RING_BUFFER_SAMPLES+1));
    memset4((void*)RING_BUF+STEREO_OFFSET, 0, SAMPLES_TO_BYTES(RING_BUFFER_SAMPLES+1));

    do_sound_command(CMD_SET_MODE(MODE_PLAY));
  }
}

void stop_sound()
{
  while(read_sound_int(&SOUNDSTATUS->mode) != MODE_PAUSE)
    do_sound_command(CMD_SET_MODE(MODE_PAUSE));
}

