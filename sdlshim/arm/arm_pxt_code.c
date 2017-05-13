#include <ronin/soundcommon.h>
#include "sound_pxt.h"

#define AICA(n) ((volatile unsigned int *)(void*)(0x800000+(n)))


#undef PXT_STATUS
#define PXT_STATUS ((volatile struct pxt_status *)(void *)(PXT_STATUS_ADDR))

#undef PXT_BUFFER
#define PXT_BUFFER(n) ((volatile struct pxt_buffer *)(void *)(PXT_BUFFER_ADDR+sizeof(struct pxt_buffer)*(n)))


#define FREQ_EXP      (-1)
#define FREQ_MANTISSA (0)
#define FREQ ((44100*(1024+FREQ_MANTISSA))>>(10-FREQ_EXP))

#undef SAMPLE_MODE
/* 8bit */
#define SAMPLE_MODE 0


static void init_channel(int channel, int pan, void *data, int len)
{
  volatile unsigned int *hwptr = AICA(channel<<7);

  /* Set sample format and buffer address */
  hwptr[0] = 0x4000 | (SAMPLE_MODE<<7) | (((unsigned long)data)>>16);
  hwptr[1] = ((unsigned long)data) & 0xffff;
  hwptr[2] = 0x0000;
  /* Number of samples */
  hwptr[3] = len;
  /* Frequency */
  hwptr[6] = ((FREQ_EXP&15)<<11)|(FREQ_MANTISSA&1023);
  /* Set volume, pan, and some other stuff */
  ((volatile unsigned char *)(hwptr+9))[4] = 0x24;
  ((volatile unsigned char *)(hwptr+9))[1] = 0xf;
  ((volatile unsigned char *)(hwptr+9))[5] = 0;
  ((volatile unsigned char *)(hwptr+9))[0] = pan;
  hwptr[4] = 0x1f;
}

static void start_channel(int channel)
{
  volatile unsigned int *hwptr = AICA(channel<<7);

  hwptr[0] |= 0xc000;
}

static void stop_sound(int channel)
{
  volatile unsigned int *hwptr = AICA(channel<<7);

  hwptr[0] = (hwptr[0] & ~0x4000) | 0x8000;
}

static void set_loop(int channel)
{
	volatile unsigned int *hwptr = AICA(channel<<7);
	
	hwptr[0] |= 0x200;
}

void _start(int cmd, void (*do_command)(int))
{
	int i,c,len;
	void *outbuffer;

	switch(cmd) {

	case CMD_SET_PLAYCHAN(0):
	case CMD_SET_PLAYCHAN(1):
	case CMD_SET_PLAYCHAN(2):
	case CMD_SET_PLAYCHAN(3):
	case CMD_SET_PLAYCHAN(4):
	case CMD_SET_PLAYCHAN(5):
	case CMD_SET_PLAYCHAN(6):
	case CMD_SET_PLAYCHAN(7):
	case CMD_SET_PLAYCHAN(8):
	case CMD_SET_PLAYCHAN(9):
	case CMD_SET_PLAYCHAN(10):
	case CMD_SET_PLAYCHAN(11):
	case CMD_SET_PLAYCHAN(12):
	case CMD_SET_PLAYCHAN(13):
		c = cmd&0xf;
		len = PXT_BUFFER(c)->len;
		outbuffer = (void*)PXT_BUFFER(c)->buffer;
		stop_sound((c<<1));
		stop_sound((c<<1)+1);
		stop_sound((c<<1)+15);
		stop_sound((c<<1)+16);

		init_channel((c<<1), 0x1f, outbuffer, len);
		init_channel((c<<1)+1, 0x0f, outbuffer, len);

		init_channel((c<<1)+15, 0x1f, outbuffer, len);
		init_channel((c<<1)+16, 0x0f, outbuffer, len);

		start_channel((c<<1));
		start_channel((c<<1)+1);
		start_channel((c<<1)+15);
		start_channel((c<<1)+16);

		break;

	case CMD_SET_PAUSECHAN(0):
	case CMD_SET_PAUSECHAN(1):
	case CMD_SET_PAUSECHAN(2):
	case CMD_SET_PAUSECHAN(3):
	case CMD_SET_PAUSECHAN(4):
	case CMD_SET_PAUSECHAN(5):
	case CMD_SET_PAUSECHAN(6):
	case CMD_SET_PAUSECHAN(7):
	case CMD_SET_PAUSECHAN(8):
	case CMD_SET_PAUSECHAN(9):
	case CMD_SET_PAUSECHAN(10):
	case CMD_SET_PAUSECHAN(11):
	case CMD_SET_PAUSECHAN(12):
	case CMD_SET_PAUSECHAN(13):
		c = cmd&0xf;
		stop_sound((c<<1));
		stop_sound((c<<1)+1);
		stop_sound((c<<1)+15);
		stop_sound((c<<1)+16);
		break;


	case CMD_SET_FREEPOS(0):
	case CMD_SET_FREEPOS(1):
	case CMD_SET_FREEPOS(2):
	case CMD_SET_FREEPOS(3):
	case CMD_SET_FREEPOS(4):
	case CMD_SET_FREEPOS(5):
	case CMD_SET_FREEPOS(6):
	case CMD_SET_FREEPOS(7):
	case CMD_SET_FREEPOS(8):
	case CMD_SET_FREEPOS(9):
	case CMD_SET_FREEPOS(10):
	case CMD_SET_FREEPOS(11):
	case CMD_SET_FREEPOS(12):
	case CMD_SET_FREEPOS(13):
		c = cmd&0xf;
		*(unsigned char *)AICA(0x280d) = (c<<1);
		PXT_STATUS->samplepos = *AICA(0x2814);
		break;


	case CMD_SET_MODE(MODE_PAUSE):
		for(i=0; i<64;i++) {
			stop_sound(i);
		}
		break;

	}
}
