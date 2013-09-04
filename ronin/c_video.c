#include "video.h"
#include "ta.h"
#include "report.h"

/* Initialize the PVR subsystem to a known state */

static unsigned int three_d_params[] = {
	0x80a8, 0x15d1c951,	/* M (Unknown magic value) */
	0x80a0, 0x00000020,	/* M */
	0x8008, 0x00000000,	/* TA out of reset */
	0x8048, 0x00000009,	/* alpha config */
	0x8068, 0x02800000,	/* pixel clipping x */
	0x806c, 0x01e00000,	/* pixel clipping y */
	0x8110, 0x00093f39,	/* M */
	0x8098, 0x00800408,	/* M */
	0x804c, 0x000000a0,	/* display align (640*2)/8 */
	0x8078, 0x3f800000,	/* polygon culling (1.0f) */
	0x8084, 0x00000000,	/* M */
	0x8030, 0x00000101,	/* M */
	0x80b0, 0x007f7f7f,	/* Fog table color */
	0x80b4, 0x007f7f7f,	/* Fog vertex color */
	0x80c0, 0x00000000,	/* color clamp min */
	0x80bc, 0xffffffff,	/* color clamp max */
	0x8080, 0x00000007,	/* M */
	0x8074, 0x00000000,	/* cheap shadow */
	0x807c, 0x0027df77,	/* M */
	0x8008, 0x00000001,	/* TA reset */
	0x8008, 0x00000000,	/* TA out of reset */
	0x80e4, 0x00000000,	/* stride width */
	0x6884, 0x00000000,	/* Disable all interrupt events */
	0x6930, 0x00000000,
	0x6938, 0x00000000,
	0x6900, 0xffffffff,	/* Clear all pending int events */
	0x6908, 0xffffffff,
#if 0
	0x6930, 0x00000000,	/* Re-enable some events */
	0x6938, 0x00000000,
#endif
	0x80b8, 0x0000ff07,	/* fog density */
	0x80b4, 0x007f7f7f,	/* fog vertex color */
	0x80b0, 0x007f7f7f,	/* fog table color */
	0x8108, 0x00000003      /* 32bit palette  */
};

static unsigned int scrn_params[] = {
	0x80e8, 0x00160000,	/* screen control */
	0x8044, 0x00800000,	/* pixel mode (vb+0x11) */
	0x805c, 0x00000000,	/* Size modulo and display lines (vb+0x17) */
	0x80d0, 0x00000100,	/* interlace flags */
	0x80d8, 0x020c0359,	/* M */
	0x80cc, 0x001501fe,	/* M */
	0x80d4, 0x007e0345,	/* horizontal border */
	0x80dc, 0x00240204,	/* vertical position */
        0x80e0, 0x07d6c63f,	/* sync control */
	0x80ec, 0x000000a4,	/* horizontal position */
	0x80f0, 0x00120012,	/* vertical border */
	0x80c8, 0x03450000,	/* set to same as border H in 80d4 */
	0x8068, 0x027f0000,	/* (X resolution - 1) << 16 */
	0x806c, 0x01df0000,	/* (Y resolution - 1) << 16 */
	0x804c, 0x000000a0,	/* display align */
	0x8118, 0x00008040,	/* M */
	0x80f4, 0x00000400,	/* anti-aliasing */
	0x8048, 0x00000009,	/* alpha config */
	0x7814, 0x00000000,	/* More interrupt control stuff (so it seems)*/
	0x7834, 0x00000000,
	0x7854, 0x00000000,
	0x7874, 0x00000000,
	0x78bc, 0x4659404f,
	0x8040, 0x00000000	/* border color */
};

static void set_regs(unsigned int *values, int cnt)
{
  volatile unsigned char *regs = (volatile unsigned char *)(void *)0xa05f0000;
  unsigned int r, v;
  
  while(cnt--) {
    r = *values++;
    v = *values++;
    *(volatile unsigned int *)(regs+r) = v;
  }
}

void dc_init_pvr()
{
  volatile unsigned int *vbl = (volatile unsigned int *)(void *)0xa05f810c;

  set_regs(three_d_params, sizeof(three_d_params)/sizeof(three_d_params[0])/2);
  while (!(*vbl & 0x01ff));
  while (*vbl & 0x01ff);
  set_regs(scrn_params, sizeof(scrn_params)/sizeof(scrn_params[0])/2);
}

void dc_set_border( unsigned int color )
{
  *(volatile unsigned int *)
    ((volatile unsigned char *)(void *)0xa05f0000+0x8040) = color;
}


/* Set up video to use TV-mode (darker stripes every other line) for
   displays with a vertical resolution of 480 pixels. */
void dc_tvmode(int on)
{
  fb_devconfig.dc_stripes = on;
}

/* Set up video registers to the desired
   video mode
  
   in:
   	cabletype  (0=VGA, 2=RGB, 3=Composite)
   	pixel mode (0=RGB555, 1=RGB565, 3=RGB888)
   	tvmode     (0 = off, 1 = on)
   	res        (0 = 320 x 240, 1 = 640 x 240, 2 = 640 x 480) 
        hz50       (0 = 60Hz, 1 = 50 Hz)
        pal        (0 = NTSC, 1 = PAL, 2 = PALM, 3 = PALN)
        voffset    (vertical offset of screen in TV mode. Added to the
                    base offset.)
*/
void dc_init_video(int cabletype, int mode, int tvmode, int res,
                   int hz50, int pal, int voffset)
{
  static int bppshifttab[]= { 1,1,0,2 };
  static int videobase=0xa05f8000;
  static int cvbsbase=0xa0702c00;
  static int hpos=0xa4;
  static int hvcounter31=0x020c0359; /* 60Hz Lace */
  static int hvcounter15=0x01060359; /* 60Hz Nonlace */
  
  //Wrong order lace...
   static int hvcounter3150=0x0270035f; /* 50Hz Lace Modern */
   static int hvcounter1550=0x0138035f; /* 50Hz Nonlace */


// static int hvcounter3150=0x0271035f; /* 50Hz Lace 0.9.4 */
  //Makes the top part of the screen shake like crazy but seems to
  //eliminate the wrong order lace problem. Also makes it NTSC...
  static int hborder=0x007e0345;
  int laceoffset=0;

  int shift, lines, hvcounter, modulo, words_per_line, vpos;
  unsigned int tmp, videoflags, attribs;

  volatile unsigned int *vbl = (volatile unsigned int *)(void *)0xa05f810c;

  //  fb_devconfig.dc_stripes = tvmode;    

  while (!(*vbl & 0x01ff));
  while (*vbl & 0x01ff);

  *(int *)(videobase+0x44)=0;
  *(int *)(videobase+0xd0)=0;

  if(!(cabletype & CABLE_VGA))
    hz50 = pal = 0;

  if(res==LOWRES || res==HIGHRES_NOLACE)
    hvcounter=(hz50? hvcounter1550 : hvcounter15);
  else
    hvcounter=(hz50? hvcounter3150 : hvcounter31);

  if(hz50)
    hborder = 0x008d034b;
    
  // Look up bytes per pixel as shift value
  mode=mode&3; //&3 is safety left over from asm.
  shift=bppshifttab[mode]; 
  // Get video HW address
  *(int *)(videobase+8)=0;	// Reset???
  *(int *)(videobase+0x40)=0;	// Set border colour to black
  // Set pixel clock and colour mode
  mode = (mode<<2)/*+1*/;
  lines = 240;			// Non-VGA screen has 240 display lines
  if(!(cabletype & CABLE_VGA))		// VGA
  {
    if(res<HIGHRES /*&& !tvmode*/) {
      mode+=2;
      laceoffset++;
    }

    hvcounter=hvcounter31;

    lines = lines<<1;		// Double # of display lines for VGA
    mode  = mode|0x800000;	// Set double pixel clock
  } else
    tvmode=0;	// Running TV-mode on a TV isn't really helpful.

  *(int *)(videobase+0x50)=0;	// Set video base address
  // Video base address for short fields should be offset by one line
  if(res==HIGHRES && (cabletype & CABLE_VGA))
    *(int *)(videobase+0x54)=640<<shift;// Set short fields video base address
  else
    *(int *)(videobase+0x54)=0;// Set short fields video base address

  // Set screen size, modulo, and interlace flag
  videoflags=1<<8;			// Video enabled
  if(res==LOWRES)
    words_per_line=(320/4)<<shift;	// Two pixels per words normally
  else
    words_per_line=(640/4)<<shift;
  modulo=1;

  fb_devconfig.dc_lace = 0;

  if(!(cabletype & CABLE_VGA))	// VGA => No interlace
  {
#if 0
    if(res<HIGHRES && !tvmode)
      modulo+=words_per_line;	//Render black on every other line.
#endif
  } else {
    if(res!=HIGHRES_NOLACE)
      modulo+=words_per_line;	//Skip the black part (lores) or
                                //add one line to offset => display
				//every other line (hires)
    if(res==HIGHRES) {
      videoflags|=1<<4; //enable LACE 
      fb_devconfig.dc_lace = 1;
    }
    
#if 0
    if(!pal)
      videoflags|=1<<6; //enable NTSC (doesn't matter on jp systems,
	                // european systems seems to be able to produce 
                        // it, US systems are unknown)

    if(hz50)
      videoflags|=1<<7;	//50Hz
#else
    /*     videoflags|=(pal&3)<<6; */
    if(cabletype & CABLE_VGA)
      videoflags|=0x40/*(pal? 0xc0 : 0x40)*/;
#endif
  }

  //modulo, height, width
  *(int *)(videobase+0x5c)=(((modulo<<10)+lines-1)<<10)+words_per_line-1;

  // Set vertical pos and border

  if(!(cabletype & CABLE_VGA)) //VGA
    voffset += 40;
  else
    voffset += (hz50? 45 : 18);

  if(res==HIGHRES && pal)       // PAL Lace 
    laceoffset = 0;
#if 0    
  else if(res==HIGHRES && hz50) // NTSC Lace 50Hz (tested on EU,JP machine. strange)
    laceoffset = 0;
#endif

  if(res<HIGHRES && (cabletype & CABLE_VGA))
    laceoffset=-1;

  fb_devconfig.dc_vidx = hpos;
  fb_devconfig.dc_vidy = voffset;

  vpos=(voffset<<16)|(voffset+laceoffset);
    
  *(int *)(videobase+0xf0)=vpos;	// V start
  *(int *)(videobase+0xdc)=vpos+lines;	// start and end border
  *(int *)(videobase+0xec)=hpos;	// Horizontal pos
  *(int *)(videobase+0xd8)=hvcounter;	// HV counter
  *(int *)(videobase+0xd4)=hborder;	// Horizontal border
  *(int *)(videobase+0xc8)=hborder<<16; // H sync
  if(!(cabletype & CABLE_VGA))
    attribs=0x1f;
  else if(hz50)
    attribs=0x3d;
  else
    attribs=0x16;
  if(res==LOWRES)
    attribs=((attribs<<8)+1)<<8;		//X-way pixel doubler
  else
    attribs=attribs<<16;

  *(int *)(videobase+0xe8)=attribs;	// Screen attributes

#if 0
  if(!(cabletype & CABLE_VGA))
    *(int *)(videobase+0xe0)=(0x0f<<22)|(793<<12)|(3<<8)|0x3f;
  else {
    attribs = 0x3f;
    if(hz50)
      attribs |= 5<<8;
    else
      attribs |= (res==HIGHRES? 6:3)<<8;
    if(hz50)
      attribs |= (res==HIGHRES? 362:799)<<12;
    else
      attribs |= (res==HIGHRES? 364:793)<<12;
    attribs |= 0x1f<<22;
    *(int *)(videobase+0xe0)=attribs;
  }
#else
  *(int *)(videobase+0xe0)=((cabletype & CABLE_VGA)? (hz50? 0x7d6a53f : 0x7d6c63f) : 0x3f1933f);
#endif

#if 0
  vpos = (hz50? 310:260);
  if(!(cabletype & CABLE_VGA))
    vpos = 510;
#endif

  /* Set up vertical blank event */
#if 0
  vpos = 242+voffset;
  if(!(cabletype & CABLE_VGA))
    vpos = 482+voffset;
#endif
  *(int *)(videobase+0xcc)=((voffset-2)<<16)|(voffset+lines+2);

  // Select RGB/CVBS
  if(cabletype&CABLE_RGB) //!rgbmode
    tmp=3;
  else
    tmp=0;
  tmp=tmp<<8;
  *(int *)cvbsbase = tmp;  

  *(int *)(videobase+0x44)=mode;// Set bpp
  *(int *)(videobase+0xd0)=videoflags;	//60Hz, NTSC, lace

  //  *(volatile unsigned int *)(void*)0xa05f6900 = 0x08;

  return;
}

void dc_video_on()
{
  volatile unsigned int *vbl = (volatile unsigned int *)(void *)0xa05f810c;

  while (!(*vbl & 0x01ff));
  while (*vbl & 0x01ff);

  *(int *)(0xa05f8044)|=1;	//video enable
}

static void pvr_check_tvsys()
{
  fb_devconfig.dc_tvsystem = (*(__volatile unsigned char *)0xa021a004) & 3;
  
  switch(fb_devconfig.dc_tvsystem) {
   case 1: // PAL
     fb_devconfig.dc_hz50 = 1; break;
   case 3: // PAL N
     fb_devconfig.dc_hz50 = 1; break;
  }
}

/* Quick memory clear */

#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)

void store_q_clear(void *ptr, int cnt)
{
  unsigned int *d = (unsigned int *)(void *)
    (0xe0000000 | (((unsigned long)ptr) & 0x03ffffc0));
  /* Set store queue memory area as desired */
  QACR0 = ((((unsigned int)ptr)>>26)<<2)&0x1c;
  QACR1 = ((((unsigned int)ptr)>>26)<<2)&0x1c;
  /* Fill both store queues with zeroes */
  d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] =
    d[8] = d[9] = d[10] = d[11] = d[12] = d[13] = d[14] = d[15] = 0;
  /* Write them as many times necessary */
  cnt>>=5;
  while(cnt--) {
    asm("pref @%0" : : "r" (d));
    d += 8;
  }
  /* Wait for both store queues to complete */
  d = (unsigned int *)0xe0000000;
  d[0] = d[8] = 0;
}

void dc_reset_screen( int hires, int lace )
{
  extern void store_q_clear(void *ptr, int cnt);
  int i, tvmode=fb_devconfig.dc_stripes, cable = dc_check_cable();

  fb_devconfig.dc_wid = 640 >> (hires?0:1);
  fb_devconfig.dc_ht  = 480 >> (lace?0:1);

  dispvar.scnbot = 480;
  dispvar.center = 0;
  dispvar.overlay = 0;
  dispvar.mode2 =
    TA_POLYMODE2_BLEND_DEFAULT|TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_MIPMAP_D_1_00|TA_POLYMODE2_TEXTURE_REPLACE|
    TA_POLYMODE2_U_SIZE_512|TA_POLYMODE2_V_SIZE_512;

  if(cable != 0)
    tvmode = /*fb_devconfig.dc_stripes =*/ 0;

  if(lace)
    tvmode = 0;

  if(tvmode) {
    /* Use texture based tvmode... */
    dispvar.overlay = 1;
    lace = 1;
    tvmode = 0;
  }

  if(fb_devconfig.dc_fullscreen) {
    lace=1; hires=1;
    if(fb_devconfig.dc_fullscreen<2)
      dispvar.mode2 |= TA_POLYMODE2_BILINEAR_FILTER;
  } else {
    dispvar.center = 1;
    if(!lace) {
      /* No need to run "noninterlace" on VGA, it just
	 makes the overlays more ugly... */
      if(cable == 0)
	lace = 1;
      else
	dispvar.scnbot/=2;
    }
    hires = 1;
  }

  /* When does this happen? if(tvmode) tvmode=0; above should prevent
     this? */
  if(tvmode && lace) {
    report("FATAL: This should never happend.\n");
    exit(17);

    dispvar.scnbot /= 2;
    lace = 0;
  }

  ta_disable_irq();

  reportf("Changing res to %d\n", hires+lace);
  
  /* Use fb_devconfig.dc_stripes, NOT tvmode here. Otherwise the call
     will destroy the value of fb_devconfig.dc_stripes set by external
     calls to dc_init_video. */
  dc_init_video(cable, 1, fb_devconfig.dc_stripes, hires+lace,
                fb_devconfig.dc_hz50, fb_devconfig.dc_tvsystem, 
                fb_devconfig.dc_voffset);

  for(i=0; i<2; i++) {
    struct ta_buffers *b = &ta_buffers[i];

    b->ta_tilew = 640/32;
    b->ta_tileh = (dispvar.scnbot+31)/32;
    b->ta_clipw = 640;
    b->ta_cliph = dispvar.scnbot;
    b->ta_lists = TA_LIST_OPAQUEPOLY|TA_LIST_TRANSPOLY;
    b->ta_extra_segments = 1024;
    b->fb_modulo = (tvmode? 1280*2 : 1280);
    b->fb_lines = b->ta_tileh*32;
    b->fb_pixfmt = TA_PIXFMT_RGB565|TA_PIXFMT_DITHER;
  }

  ta_init_renderstate();

  store_q_clear(ta_buffers[0].fb_base,
		ta_buffers[0].fb_modulo*ta_buffers[0].fb_lines);
  store_q_clear(ta_buffers[1].fb_base,
		ta_buffers[1].fb_modulo*ta_buffers[1].fb_lines);

  ta_enable_irq();
}

void dc_setup_ta()
{
  *(volatile unsigned int *)(void*)0xa05f8008 = 3;

  pvr_check_tvsys(&fb_devconfig);
  store_q_clear((void *)0xa4000000, 8*1024*1024);
  dc_init_pvr();
  dc_reset_screen(1, 1);
  dc_video_on();
}
