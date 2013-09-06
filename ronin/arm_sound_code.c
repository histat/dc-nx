#include "soundcommon.h"

#define AICA(n) ((volatile unsigned int *)(void*)(0x800000+(n)))

#undef RING_BUF

#if SAMPLE_MODE == 0
#define RING_BUF ((short *)(void *)(RING_BASE_ADDR))
#else
#define RING_BUF ((signed char *)(void *)(RING_BASE_ADDR))
#endif

#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(SOUNDSTATUS_ADDR))

#define EXPANSION ((void (*)(int , void (*)(int) ))(void*)EXPANSION_BASE_ADDR)

//static void __gccmain() { }

static int freq_exp = 0;
static int freq_mantissa = 0;

void aica_reset()
{
  int i, j;
  volatile unsigned int *hwptr = AICA(0);

  *AICA(0x2800) = 0;

  /* Reset all 64 channels to a silent state */
  for(i=0; i<64; i++) {
    hwptr[0] = 0x8000;
    hwptr[5] = 0x1f;
    hwptr[1] = 0;
    hwptr[2] = 0;
    hwptr[3] = 0;
    hwptr[4] = 0;
    for(j=6; j<32; j++)
      hwptr[j] = 0;
    hwptr += 32;
  }

  /* Enable CDDA full volume, normal panning */
  *AICA(0x2040) = 0x0f0f;
  *AICA(0x2044) = 0x0f1f;

  *AICA(0x2800) = 15;
}

void init_channel(int channel, int pan, void *data, int len)
{
  volatile unsigned int *hwptr = AICA(channel<<7);

  /* Set sample format and buffer address */
  hwptr[0] = 0x4200 | (SAMPLE_MODE<<7) | (((unsigned long)data)>>16);
  hwptr[1] = ((unsigned long)data) & 0xffff;
  /* Number of samples */
  hwptr[3] = len;
  /* Frequency */
  hwptr[6] = ((freq_exp&15)<<11)|(freq_mantissa&1023);
  /* Set volume, pan, and some other stuff */
  ((volatile unsigned char *)(hwptr+9))[4] = 0x24;
  ((volatile unsigned char *)(hwptr+9))[1] = 0xf;
  ((volatile unsigned char *)(hwptr+9))[5] = 0;
  ((volatile unsigned char *)(hwptr+9))[0] = pan;
  hwptr[4] = 0x1f;
}
/*
void do_command(int cmd)
{
  switch(cmd) {
#ifdef STEREO
   case CMD_SET_STEREO(0):
   case CMD_SET_STEREO(1):
     SOUNDSTATUS->stereo = cmd&1;
     break;
#endif
   case CMD_SET_FREQ_EXP(-8):
   case CMD_SET_FREQ_EXP(-7):
   case CMD_SET_FREQ_EXP(-6):
   case CMD_SET_FREQ_EXP(-5):
   case CMD_SET_FREQ_EXP(-4):
   case CMD_SET_FREQ_EXP(-3):
   case CMD_SET_FREQ_EXP(-2):
   case CMD_SET_FREQ_EXP(-1):
     freq_exp = (cmd&7)-8;
     SOUNDSTATUS->freq = FREQ_OF(freq_exp, freq_mantissa);
     break;
   case CMD_SET_FREQ_EXP(0):
   case CMD_SET_FREQ_EXP(1):
   case CMD_SET_FREQ_EXP(2):
   case CMD_SET_FREQ_EXP(3):
   case CMD_SET_FREQ_EXP(4):
   case CMD_SET_FREQ_EXP(5):
   case CMD_SET_FREQ_EXP(6):
   case CMD_SET_FREQ_EXP(7):
     freq_exp = (cmd&7);
     SOUNDSTATUS->freq = FREQ_OF(freq_exp, freq_mantissa);
     break;
   case CMD_SET_BUFFER(0):
   case CMD_SET_BUFFER(1):
   case CMD_SET_BUFFER(2):
   case CMD_SET_BUFFER(3):
   case CMD_SET_BUFFER(4):
   case CMD_SET_BUFFER(5):
   case CMD_SET_BUFFER(6):
   case CMD_SET_BUFFER(7):
     SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES>>(cmd&15);
     break;
   case CMD_SET_MODE(MODE_PAUSE):
     *AICA(0) = (*AICA(0) & ~0x4000) | 0x8000;
#ifdef STEREO
     if(SOUNDSTATUS->stereo)
       *AICA(0x80) = (*AICA(0x80) & ~0x4000) | 0x8000;
#endif
     SOUNDSTATUS->samplepos = 0;
     SOUNDSTATUS->mode = MODE_PAUSE;
     break;
   case CMD_SET_MODE(MODE_PLAY):
#ifdef STEREO
     if(SOUNDSTATUS->stereo) {
       init_channel(0, 0x1f, RING_BUF, SOUNDSTATUS->ring_length);
       init_channel(1, 0x0f, RING_BUF+STEREO_OFFSET, SOUNDSTATUS->ring_length);
     } else
#endif
     init_channel(0, 0x00, RING_BUF, SOUNDSTATUS->ring_length);
     SOUNDSTATUS->samplepos = 0;
     *AICA(0) |= 0xc000;
#ifdef STEREO
     if(SOUNDSTATUS->stereo)
       *AICA(0x80) |= 0xc000;
#endif
     *(unsigned char *)AICA(0x280d) = 0;
     SOUNDSTATUS->mode = MODE_PLAY;
     break;
  }
}
*/

void do_command(int cmd)
{
  switch(cmd) {
   case CMD_SET_MODE(MODE_PAUSE):
	   *AICA(0) = (*AICA(0) & ~0x4000) | 0x8000;
       *AICA(0x80) = (*AICA(0x80) & ~0x4000) | 0x8000;

     SOUNDSTATUS->samplepos = 0;
     SOUNDSTATUS->mode = MODE_PAUSE;
     break;
   case CMD_SET_MODE(MODE_PLAY):
       init_channel(0, 0x1f, RING_BUF, SOUNDSTATUS->ring_length);
       init_channel(1, 0x0f, RING_BUF+STEREO_OFFSET, SOUNDSTATUS->ring_length);

     SOUNDSTATUS->samplepos = 0;
     *AICA(0) |= 0xc000;
	 *AICA(0x80) |= 0xc000;

     *(unsigned char *)AICA(0x280d) = 0;
     SOUNDSTATUS->mode = MODE_PLAY;
     break;
  }
}

/*
void *memcpy(void *s1, const void *s2, unsigned int n)
{
  unsigned char *d = s1;
  const unsigned char *s = s2;
  while(n--)
    *d++ = *s++;
  return s1;
}
*/

int main()
{
  /* int n = 1; */

  SOUNDSTATUS->mode = MODE_PAUSE;
  SOUNDSTATUS->samplepos = 0;

  freq_exp = -1;
  freq_mantissa = 0;
  SOUNDSTATUS->freq = FREQ_OF(0, 0);
  SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES;

  aica_reset();

	//permits SH4 interrupt Bit 5  MCIEB
	*AICA(0x28b4) = 0x20;

	//resets SH4 intrrrupt MCIRE
	*AICA(0x28bc) = 0x20;

  for(;;) {

    if(SOUNDSTATUS->cmdstatus==1) {
      /*      SOUNDSTATUS[n++] = *SOUNDSTATUS; */
      if(SOUNDSTATUS->cmd < 0)
	EXPANSION(~SOUNDSTATUS->cmd, do_command);
      else
	do_command(SOUNDSTATUS->cmd);
      SOUNDSTATUS->cmdstatus = 2;
      /*
      SOUNDSTATUS[n++] = *SOUNDSTATUS;
      SOUNDSTATUS[n].mode = 12345678;
      */
    }

    if(SOUNDSTATUS->mode == MODE_PLAY) {
		//SOUNDSTATUS->samplepos = *AICA(0x2814);
	  unsigned int pos = *AICA(0x2814);

	  if(pos==(RING_BUFFER_SAMPLES>>1)) {
		  SOUNDSTATUS->samplepos = 0;

		  *AICA(0x28b8) = 0x20;
		  *AICA(0x28bc) = 0x20;
		  
	  } else if(pos==RING_BUFFER_SAMPLES-1) {

		  SOUNDSTATUS->samplepos = (RING_BUFFER_SAMPLES>>1);

		  *AICA(0x28b8) = 0x20;
		  *AICA(0x28bc) = 0x20;
	  }
	  
	}
	
  }
}
