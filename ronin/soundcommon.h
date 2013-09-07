#ifndef _RONIN_SOUNDCOMMON_H
#define _RONIN_SOUNDCOMMON_H been_here_before

/*
 * This file is included both on ARM and SH
 */

/* stereo */
#define STEREO

struct soundstatus {
  int mode;
  int cmd;
  int cmdstatus;
  int samplepos;
#ifdef STEREO
  int stereo;
#endif
  int freq;
  int ring_length;
};

#define SOUNDSTATUS_ADDR (0xff80)
#define RING_BASE_ADDR (0x10000)

#define EXPANSION_BASE_ADDR (0x40000)

#define MODE_PAUSE 0
#define MODE_PLAY  1

#define CMD_SET_MODE(n) (n)

#ifdef STEREO
#define CMD_SET_STEREO(n) (0x40|(n))
#endif

#define CMD_SET_FREQ_EXP(n) (0x50|((n)&0xf))
#define CMD_SET_BUFFER(n) (0x60|(n))

#define CMD_EXPANSION(n) (~(n))

#define FREQ_172_266_EXP  (-8)
#define FREQ_344_531_EXP  (-7)
#define FREQ_689_062_EXP  (-6)
#define FREQ_1378_125_EXP (-5)
#define FREQ_2756_25_EXP  (-4)
#define FREQ_5512_5_EXP   (-3)
#define FREQ_11025_EXP    (-2)
#define FREQ_22050_EXP    (-1)
#define FREQ_44100_EXP    (0)
#define FREQ_88200_EXP    (1)
#define FREQ_176400_EXP   (2)
#define FREQ_352800_EXP   (3)
#define FREQ_705600_EXP   (4)
#define FREQ_1411200_EXP  (5)
#define FREQ_2822400_EXP  (6)
#define FREQ_5644800_EXP  (7)


#define FREQ_OF(E,M) ((44100*(1024+(M)))>>(10-(E)))


#define ADJUST_BUFFER_SIZE(n) ((n)&~31)

/* 1s buffer for menu */
//#define RING_BUFFER_SAMPLES ADJUST_BUFFER_SIZE(44100)
#define RING_BUFFER_SAMPLES ADJUST_BUFFER_SIZE(5512)


/* 16bit */
#define SAMPLE_MODE 0

#if SAMPLE_MODE == 0
#define SAMPLES_TO_BYTES(n) ((n)<<1)
#else
#if SAMPLE_MODE == 1
#define SAMPLES_TO_BYTES(n) (n)
#else
#define SAMPLES_TO_BYTES(n) ((n)>>1)
#endif
#endif

#define STEREO_OFFSET (RING_BUFFER_SAMPLES+256)

#if SAMPLE_MODE == 0
#define RING_BUF ((short *)(void *)(0xa0800000+RING_BASE_ADDR))
#else
#define RING_BUF ((signed char *)(void *)(0xa0800000+RING_BASE_ADDR))
#endif

#if SAMPLE_MODE == 0
extern short temp_sound_buffer[];
#else
extern signed char temp_sound_buffer[];
#endif

#endif /* _RONIN_SOUNDCOMMON_H */
