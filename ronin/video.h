#ifndef _RONIN_VIDEO_H
#define _RONIN_VIDEO_H been_here_before

/* 
 * Functions used to setup and operate on the framebuffer and tile
 * accellerator.
 */
#include "common.h"

START_EXTERN_C
void dc_init_pvr();
void dc_init_video(int cabletype, int mode, int tvmode, int res,
                   int hz50, int pal, int voffset);
void dc_video_on();
int  dc_check_cable();
void dc_waitvbl();
void dc_setup_ta();
void dc_reset_screen( int hires, int lace );

void dc_set_border( unsigned int color );

void dc_draw_string(int x, int y, const char *message, int color);
void dc_draw_char12(int x, int y, int c, int color);
void dc_clrscr(int color);
void dc_multibuffer_set(int show, int draw);
void dc_tvmode(int on);
END_EXTERN_C

/* Shouldn't VGA be 0 and RGB be 2? */
#define CABLE_VGA 2
#define CABLE_RGB 1
#define CABLE_COM 3

/*
   	pixel mode (0=RGB555, 1=RGB565, 3=RGB888)
#define 
*/

#define LOWRES 0
#define HIGHRES_NOLACE 1
#define HIGHRES 2



#define C_RED   0xff0000
#define C_GREEN 0x00ff00
#define C_BLUE  0x0000ff
#define C_MAGENTA (C_RED | C_BLUE)
#define C_YELLOW (C_RED | C_GREEN)
#define C_ORANGE (C_RED | 0x7f00)
#define C_WHITE 0xffffff
#define C_BLACK 0

#define C_DARK_ORANGE 0x7f3f00

#define C_GREY   0x7f7f7f

extern unsigned short *dc_current_draw_buffer;

#define VREG_BASE 0xa05f8000

// FIXME: Replace this with the fb_devconfig or something
struct _dispvar{
  int scnbot, center, overlay;
  unsigned int mode2;
};
extern struct _dispvar dispvar;
// END FIXME

#define vaddr_t void*
struct _fb_devconfig{
  vaddr_t dc_vaddr;		/* framebuffer virtual address */
  vaddr_t dc_paddr;		/* framebuffer physical address */
  int	dc_wid;			/* width of frame buffer */
  int	dc_ht;			/* height of frame buffer */
  int	dc_depth;		/* depth, bits per pixel */
  int	dc_rowbytes;		/* bytes in a FB scan line */
  vaddr_t dc_videobase;		/* base of flat frame buffer */
  int	dc_blanked;		/* currently has video disabled */
  int	dc_dispflags;		/* display flags */
  int	dc_tvsystem;		/* TV broadcast system */
  int	dc_voffset;		/* Vertical display offset */
  int   dc_fullscreen;          /* Render content streched to full screen */
  int   dc_hz50;                /* Refresh rate */
  int   dc_vidx;		/* Screen start X */
  int   dc_vidy;		/* Screen start Y */
  int   dc_lace;		/* Interlace 1/0 */
  int   dc_stripes;             /* render nostalgia stripes */

  //	struct rasops_info rinfo;
};
extern struct _fb_devconfig fb_devconfig;

#endif //_RONIN_VIDEO_H
