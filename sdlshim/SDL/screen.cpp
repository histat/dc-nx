
#include "../shim.h"
#include "bmploader.h"
#include "screen.h"
#include "screen.fdh"

#include <kos.h>
#include "libsh4.h"
#include "npvr_vertex.h"
#include "profile.h"
#include "watchdog.h"

struct polygon_list {
  unsigned int cmd;
  unsigned int mode1;
  unsigned int mode2;
  unsigned int texture;
  float alpha, red, green, blue; /* used with intensity type colour */
};

#define TA_CMD_POLYGON                    0x80000000
#define TA_CMD_MODIFIER                   0x80000000
#define TA_CMD_SPRITE			  0xa0000000
#define TA_CMD_POLYGON_TYPE_OPAQUE        (0<<24)
#define TA_CMD_MODIFIER_TYPE_OPAQUE       (1<<24)
#define TA_CMD_POLYGON_TYPE_TRANSPARENT   (2<<24)
#define TA_CMD_MODIFIER_TYPE_TRANSPARENT  (3<<24)
#define TA_CMD_POLYGON_TYPE_PUNCHTHRU     (4<<24)
#define TA_CMD_POLYGON_SUBLIST            0x00800000
#define TA_CMD_POLYGON_STRIPLENGTH_1      (0<<18)
#define TA_CMD_POLYGON_STRIPLENGTH_2      (1<<18)
#define TA_CMD_POLYGON_STRIPLENGTH_4      (2<<18)
#define TA_CMD_POLYGON_STRIPLENGTH_6      (3<<18)
#define TA_CMD_POLYGON_USER_CLIP_INSIDE   0x00020000
#define TA_CMD_POLYGON_USER_CLIP_OUTSIDE  0x00030000
#define TA_CMD_POLYGON_AFFECTED_BY_MODIFIER  0x00000080
#define TA_CMD_POLYGON_CHEAP_SHADOW_MODIFIER 0x00000000
#define TA_CMD_POLYGON_NORMAL_MODIFIER       0x00000040
#define TA_CMD_POLYGON_PACKED_COLOUR      (0<<4)
#define TA_CMD_POLYGON_FLOAT_COLOUR       (1<<4)
#define TA_CMD_POLYGON_INTENSITY          (2<<4)
#define TA_CMD_POLYGON_PREVFACE_INTENSITY (3<<4)
#define TA_CMD_POLYGON_TEXTURED           0x00000008
#define TA_CMD_POLYGON_SPECULAR_HIGHLIGHT 0x00000004
#define TA_CMD_POLYGON_GOURAUD_SHADING    0x00000002
#define TA_CMD_POLYGON_16BIT_UV           0x00000001

#define TA_POLYMODE1_Z_NEVER        (0<<29)
#define TA_POLYMODE1_Z_LESS         (1<<29)
#define TA_POLYMODE1_Z_EQUAL        (2<<29)
#define TA_POLYMODE1_Z_LESSEQUAL    (3<<29)
#define TA_POLYMODE1_Z_GREATER      (4<<29)
#define TA_POLYMODE1_Z_NOTEQUAL     (5<<29)
#define TA_POLYMODE1_Z_GREATEREQUAL (6<<29)
#define TA_POLYMODE1_Z_ALWAYS       (7<<29)
#define TA_POLYMODE1_CULL_SMALL     (1<<27)
#define TA_POLYMODE1_CULL_CCW       (2<<27)
#define TA_POLYMODE1_CULL_CW        (3<<27)
#define TA_POLYMODE1_NO_Z_UPDATE    0x04000000

#define TA_POLYMODE2_BLEND_SRC_ALPHA    (0x80<<24)
#define TA_POLYMODE2_BLEND_SRC          (0x20<<24)
#define TA_POLYMODE2_BLEND_SRC_INVALPHA (0xa0<<24)
#define TA_POLYMODE2_BLEND_DST_ALPHA    (0x10<<24)
#define TA_POLYMODE2_BLEND_DST          (0x04<<24)
#define TA_POLYMODE2_BLEND_DST_INVALPHA (0x14<<24)

#define TA_POLYMODE2_BLEND_DEFAULT  TA_POLYMODE2_BLEND_SRC

#define TA_POLYMODE2_FOG_TABLE      (0<<22)
#define TA_POLYMODE2_FOG_VERTEX     (1<<22)
#define TA_POLYMODE2_FOG_DISABLED   (2<<22)
#define TA_POLYMODE2_FOG_TABLE2     (3<<22)
#define TA_POLYMODE2_CLAMP_COLOURS  0x00200000
#define TA_POLYMODE2_ENABLE_ALPHA   0x00100000
#define TA_POLYMODE2_DISABLE_TEXTURE_TRANSPARENCY 0x00080000
#define TA_POLYMODE2_TEXTURE_FLIP_U   0x00040000
#define TA_POLYMODE2_TEXTURE_FLIP_V   0x00020000
#define TA_POLYMODE2_TEXTURE_CLAMP_U  0x00010000
#define TA_POLYMODE2_TEXTURE_CLAMP_V  0x00008000
#define TA_POLYMODE2_TRILINEAR_FILTER 0x00004000
#define TA_POLYMODE2_BILINEAR_FILTER  0x00002000
#define TA_POLYMODE2_MIPMAP_D_0_25    (1<<8)
#define TA_POLYMODE2_MIPMAP_D_0_50    (2<<8)
#define TA_POLYMODE2_MIPMAP_D_0_75    (3<<8)
#define TA_POLYMODE2_MIPMAP_D_1_00    (4<<8)
#define TA_POLYMODE2_MIPMAP_D_1_25    (5<<8)
#define TA_POLYMODE2_MIPMAP_D_1_50    (6<<8)
#define TA_POLYMODE2_MIPMAP_D_1_75    (7<<8)
#define TA_POLYMODE2_MIPMAP_D_2_00    (8<<8)
#define TA_POLYMODE2_MIPMAP_D_2_25    (9<<8)
#define TA_POLYMODE2_MIPMAP_D_2_50    (10<<8)
#define TA_POLYMODE2_MIPMAP_D_2_75    (11<<8)
#define TA_POLYMODE2_MIPMAP_D_3_00    (12<<8)
#define TA_POLYMODE2_MIPMAP_D_3_25    (13<<8)
#define TA_POLYMODE2_MIPMAP_D_3_50    (14<<8)
#define TA_POLYMODE2_MIPMAP_D_3_75    (15<<8)
#define TA_POLYMODE2_TEXTURE_REPLACE  (0<<6)
#define TA_POLYMODE2_TEXTURE_MODULATE (1<<6)
#define TA_POLYMODE2_TEXTURE_DECAL    (2<<6)
#define TA_POLYMODE2_TEXTURE_MODULATE_ALPHA (3<<6)
#define TA_POLYMODE2_U_SIZE_8         (0<<3)
#define TA_POLYMODE2_U_SIZE_16        (1<<3)
#define TA_POLYMODE2_U_SIZE_32        (2<<3)
#define TA_POLYMODE2_U_SIZE_64        (3<<3)
#define TA_POLYMODE2_U_SIZE_128       (4<<3)
#define TA_POLYMODE2_U_SIZE_256       (5<<3)
#define TA_POLYMODE2_U_SIZE_512       (6<<3)
#define TA_POLYMODE2_U_SIZE_1024      (7<<3)
#define TA_POLYMODE2_V_SIZE_8         (0<<0)
#define TA_POLYMODE2_V_SIZE_16        (1<<0)
#define TA_POLYMODE2_V_SIZE_32        (2<<0)
#define TA_POLYMODE2_V_SIZE_64        (3<<0)
#define TA_POLYMODE2_V_SIZE_128       (4<<0)
#define TA_POLYMODE2_V_SIZE_256       (5<<0)
#define TA_POLYMODE2_V_SIZE_512       (6<<0)
#define TA_POLYMODE2_V_SIZE_1024      (7<<0)

#define TA_TEXTUREMODE_MIPMAP       0x80000000
#define TA_TEXTUREMODE_VQ_COMPRESSION 0x40000000
#define TA_TEXTUREMODE_ARGB1555     (0<<27)
#define TA_TEXTUREMODE_RGB565       (1<<27)
#define TA_TEXTUREMODE_ARGB4444     (2<<27)
#define TA_TEXTUREMODE_YUV422       (3<<27)
#define TA_TEXTUREMODE_BUMPMAP      (4<<27)
#define TA_TEXTUREMODE_CLUT4        (5<<27)
#define TA_TEXTUREMODE_CLUT8        (6<<27)
#define TA_TEXTUREMODE_CLUTBANK8(n) ((n)<<25) /* 0-3  */
#define TA_TEXTUREMODE_CLUTBANK4(n) ((n)<<21) /* 0-63 */
#define TA_TEXTUREMODE_TWIDDLED     0x00000000
#define TA_TEXTUREMODE_NON_TWIDDLED 0x04000000
#define TA_TEXTUREMODE_STRIDE	    (1<<25)
#define TA_TEXTUREMODE_ADDRESS(a)   ((((unsigned long)(void*)(a))&0x7fffff)>>3)

struct packed_colour_vertex_list  {
  unsigned int cmd;
  float x, y, z, u, v;
  unsigned int colour, ocolour;
};

#define TA_CMD_VERTEX     0xE0000000
#define TA_CMD_VERTEX_EOS 0xF0000000  /* end of strip */

#define GET_PIXEL_ADDR(SURFACE, X, Y)	\
	((uint16_t *)(((uint8_t *)SURFACE->pixels) + ((Y) * SURFACE->pitch) + ((X) * 2)))

static SDL_PixelFormat SDLS_StdFormat = { NULL, 16, 2 ,
						 0,0,0,0,
						 11,6,0,
						 0xff,0xff,0xff};
SDL_Surface *SDLS_VRAMSurface = NULL;
static SDL_Surface _default;

// -----

#define FULL_SCREEN_W 640
#define FULL_SCREEN_H 480

static int screen_buffer = 0;

static float _screen_w;
static float _screen_h;
static float _screen_x;
static float _screen_y;

static bool _fullscreen = false;

static void set_scaling()
{
	float _x_scale, _y_scale;
	
	if (!_fullscreen) {

		_screen_x = 0;
		_screen_y = 20;

		_x_scale = FULL_SCREEN_W*1.0/320;;
		_y_scale = (FULL_SCREEN_H - _screen_y*2)*1.0/240;

	} else {
		_screen_x = 0;
		_screen_y = 0;

		_x_scale = 2.0;
		_y_scale = 2.0;
	}

	_screen_w = SCREEN_WIDTH  * _x_scale;
	_screen_h = SCREEN_HEIGHT * _y_scale;
}


#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)

static void tex_memcpy(void *dst, void *src, unsigned int n)
{
	unsigned int *s = (unsigned int *)src;
	unsigned int *d = (unsigned int *)(void *) \
		(0xe0000000 | (((unsigned long)dst) & 0x03ffffc0));
	
	QACR0 = 0xa4;
	QACR1 = 0xa4;

	n >>= 6;

	while (n--) {
		d[0] = *s++;
		d[1] = *s++;
		d[2] = *s++;
		d[3] = *s++;
		asm("pref @%0" : : "r" (s+16));
		d[4] = *s++;
		d[5] = *s++;
		d[6] = *s++;
		d[7] = *s++;
		asm("pref @%0" : : "r" (d));
		d += 8;
		d[0] = *s++;
		d[1] = *s++;
		d[2] = *s++;
		d[3] = *s++;
		asm("pref @%0" : : "r" (s+16));
		d[4] = *s++;
		d[5] = *s++;
		d[6] = *s++;
		d[7] = *s++;
		asm("pref @%0" : : "r" (d));
		d += 8;
	}
}

void update_polygon()
{
	struct polygon_list mypoly;

	mypoly.cmd =
		TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
		TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
	mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
	mypoly.mode2 =
		TA_POLYMODE2_BLEND_DEFAULT|TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
		TA_POLYMODE2_U_SIZE_1024|TA_POLYMODE2_V_SIZE_1024;
	mypoly.texture =
		TA_TEXTUREMODE_RGB565|TA_TEXTUREMODE_STRIDE|TA_TEXTUREMODE_NON_TWIDDLED|
		TA_TEXTUREMODE_ADDRESS(screen_tx[screen_buffer]);
	mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;

	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_OP_POLY);

	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	sqCopy32(sq, &mypoly);


	pvr_vertex32 *v = (pvr_vertex32*)sq + 32;

	float x = _screen_x;
	float y = _screen_y;
	int w = _screen_w;
	int h = _screen_h;

	pv_set_pos(v, x, y, 1.0);
	pv_set_uv(v, 0.0, 0.0);
	pv_set_cmd_submit_vertex(v);
	v++;

	pv_set_pos(v, x, y + h, 1.0);
	pv_set_uv(v, 0.0, SCREEN_HEIGHT * (1.0/1024));
	pv_set_cmd_submit_vertex(v);
	v++;

	pv_set_pos(v, x + w, y, 1.0);
	pv_set_uv(v, SCREEN_WIDTH * (1.0/1024), 0.0);
	pv_set_cmd_submit_vertex(v);
	v++;

	pv_set_pos(v, x + w, y + h, 1.0);
	pv_set_uv(v, SCREEN_WIDTH * (1.0/1024), SCREEN_HEIGHT * (1.0/1024));
	pv_set_cmd_submit_vertex_eos(v);

	pvr_list_finish();
	pvr_scene_finish();

}

// -----

SDL_Surface *SDL_SetVideoMode(int width, int height, int bitsperpixel, uint32_t flags)
{
	if (flags & SDL_FULLSCREEN)
		_fullscreen = true;
	else
		_fullscreen = false;
	
	set_scaling();

	screen_buffer = 0;

	SDLS_VRAMSurface = &_default;

	SDL_Surface *sfc = SDLS_VRAMSurface;

	sfc->pixels = vram;
	sfc->format = &SDLS_StdFormat;
	
	sfc->w = width;
	sfc->h = height;
	sfc->pitch = width * 2;
	
	sfc->use_colorkey = false;
	sfc->free_pixels = false;
	
	sfc->cliprect.x = 0;
	sfc->cliprect.y = 0;
	sfc->cliprect.w = sfc->w;
	sfc->cliprect.h = sfc->h;

	return SDLS_VRAMSurface;
}


// blit 'screen' (which is assumed to be the same size and pitch as the real screen,
// usually this is the surface returned by SDL_SetVideoMode), to the screen.
void SDL_Flip(SDL_Surface *sfc)
{
    ++screen_buffer;
    screen_buffer &= (SCREEN_BUFFER_SIZE-1);
	
    unsigned short *dst = (unsigned short *)screen_tx[screen_buffer];
    unsigned short *src = sfc->pixels;
    
    tex_memcpy(dst, src, VRAM_SIZE);
    
    update_polygon();
}


// called from SDL_Quit()
void SDLS_CloseScreen()
{
}


/*
void c------------------------------() {}
*/

// loads a Windows BMP file into an SDL_Surface.
// unlike real SDL, this function always returns a 16-bpp surface,
// regardless of the color depth of the original bitmap.
SDL_Surface *SDL_LoadBMP(const char *fname)
{
uint16_t *bitmap;
BMPHeader hdr;

	bitmap = bmp_load(fname, &hdr);
	if (!bitmap) return NULL;
	
	return SDLS_SurfaceFromRaw(bitmap, hdr.bmWidth, hdr.bmHeight, hdr.bmWidth*2, false);
}

// turn on or off use of transparent colorkey based blitting for 'surface'.
// unlike real SDL, the key is always 0 (black), and the value passed for key
// is ignored.
int SDL_SetColorKey(SDL_Surface *surface, uint32_t flag, uint32_t key)
{
	surface->use_colorkey = (flag & SDL_SRCCOLORKEY) ? true : false;
	return 0;
}


void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect_in, \
					SDL_Surface *dst, SDL_Rect *dstrect)
{
SDL_Rect srcrect, dstrectbuf;
uint16_t *srcptr, *dstptr;
int w, h;
	
	/// GET SRCRECT
	
	// srcrect isn't modified.
	// NULL means copy whole surface.
	if (srcrect_in)
	{
		// make a copy so it isn't modified
		srcrect = *srcrect_in;
	}
	else
	{	// set to entire surface
		srcrect.x = 0;
		srcrect.y = 0;
		srcrect.w = src->w;
		srcrect.h = src->h;
	}
	
	/// GET DSTRECT
	
	// dstrect will be modified to = final clipping region.
	// NULL means copy to 0,0.
	if (!dstrect)
	{
		dstrectbuf.x = 0;
		dstrectbuf.y = 0;
		dstrect = &dstrectbuf;
	}
	
	// dstrect w and h are ignored.
	dstrect->w = srcrect.w;
	dstrect->h = srcrect.h;
	
	/// CLIPPING
	
	// clip srcrect to edges of source surface
	SDL_Rect srcbounds;
	srcbounds.x = 0;
	srcbounds.y = 0;
	srcbounds.w = src->w;
	srcbounds.h = src->h;
	
	clip_rect(&srcrect, dstrect, &srcbounds);
	
	// clip dstrect to clipping region of destination
	clip_rect(dstrect, &srcrect, &dst->cliprect);
	
	/// BLIT
	
	// set up and do the blit
	w = min(srcrect.w, dstrect->w);
	h = min(srcrect.h, dstrect->h);
	
	if (w == 0 || h == 0)		// entirely offscreen
		return;
	
	srcptr = GET_PIXEL_ADDR(src, srcrect.x, srcrect.y);
	dstptr = GET_PIXEL_ADDR(dst, dstrect->x, dstrect->y);
	
	if (src->use_colorkey)
	{
		asm_do_blit_transparent(srcptr, dstptr, w, h, \
								src->pitch - (w * 2), \
								dst->pitch - (w * 2));
	}
	else
	{
		asm_do_blit_nontransparent(srcptr, dstptr, w, h, \
								src->pitch - (w * 2), \
								dst->pitch - (w * 2));
	}
}

void SDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect)
{
	if (rect)
	{
		// set left and top
		surface->cliprect.x = rect->x;
		if (surface->cliprect.x < 0) surface->cliprect.x = 0;
		
		surface->cliprect.y = rect->y;
		if (surface->cliprect.y < 0) surface->cliprect.y = 0;
		
		// set right and bottom
		int x2 = rect->x + (rect->w - 1);
		int y2 = rect->y + (rect->h - 1);
		
		if (x2 >= surface->w) x2 = (surface->w - 1);
		if (y2 >= surface->h) y2 = (surface->h - 1);
		
		surface->cliprect.w = (x2 - surface->cliprect.x) + 1;
		surface->cliprect.h = (y2 - surface->cliprect.y) + 1;
	}
	else
	{
		surface->cliprect.x = 0;
		surface->cliprect.y = 0;
		surface->cliprect.w = surface->w;
		surface->cliprect.h = surface->h;
	}
}


void SDL_FreeSurface(SDL_Surface *sfc)
{
	if (sfc->pixels)
		free(sfc->pixels);
	
	if (sfc->format != &SDLS_StdFormat)
		free(sfc->format);
	
	free(sfc);
}


// clips 'rect' to be clipped to 'clip'
// what's 'otherrect'? well, it has to do with adjusting the "other"
// rect in a srcrect/dstrect pair when the top or left of 'rect' is clipped.
//
//	dstrect->x off left side: srcrect->x must be += -dstrect->x
//
//	srcrect->x off left side of source:
//		dstrect->x += -srcrect->x
//
static void clip_rect(SDL_Rect *rect, SDL_Rect *otherrect, SDL_Rect *clip)
{
int rx2, ry2;
int cx2, cy2;
int dist;

	// get x2, y2 from width/height
	rx2 = rect->x + (rect->w - 1);
	ry2 = rect->y + (rect->h - 1);
	
	cx2 = clip->x + (clip->w - 1);
	cy2 = clip->y + (clip->h - 1);
	
	// top and left-side clipping
	if (rect->x < clip->x)
	{
		dist = (clip->x - rect->x);
		otherrect->x += dist;
		
		if (otherrect->w >= dist)
			otherrect->w -= dist;
		else
			otherrect->w = 0;
		
		rect->x = clip->x;
	}
	
	if (rect->y < clip->y)
	{
		dist = (clip->y - rect->y);
		otherrect->y += dist;
		
		if (otherrect->h >= dist)
			otherrect->h -= dist;
		else
			otherrect->h = 0;
		
		rect->y = clip->y;
	}
	
	// right and bottom-side clipping
	if (rx2 > cx2) rx2 = cx2;
	if (ry2 > cy2) ry2 = cy2;
	
	// recalculate new width/height
	if (rect->x > rx2)
		rect->w = 0;
	else
		rect->w = (rx2 - rect->x) + 1;
	
	if (rect->y > ry2)
		rect->h = 0;
	else
		rect->h = (ry2 - rect->y) + 1;
}

/*
void c------------------------------() {}
*/

// create a new surface.
// most of these parameters are ignored and are for compatibility only.
SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth, \
						uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
	return SDLS_CreateSurface(width, height, false);
}

// convert a surface to the "display format". since ALL surfaces
// in SDLS are 16-bpp anyway, this is a stupid function to call
// as all it does it return a copy of the input surface.
// provided for compatibility only.
SDL_Surface *SDL_DisplayFormat(SDL_Surface *sfc)
{
SDL_Surface *newsfc;

	newsfc = SDLS_CreateSurface(sfc->w, sfc->h, sfc->use_colorkey);
	memcpy(newsfc->pixels, sfc->pixels, (sfc->h * sfc->pitch));
	
	return newsfc;
}

uint32_t SDL_MapRGB(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b)
{
	return (uint16_t)(((r & 0xf8) << 8) | \
					  ((g & 0xfc) << 3) | \
					  ((b & 0xf8) >> 3));
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
	if (!dstrect)
	{
		uint16_t *buffer = dst->pixels;
		int npixels = (dst->w * dst->h);
		
		while(npixels--)
			*(buffer++) = color;
		
		return 0;
	}
	else
	{
		int x2, y2, cx2, cy2;
		
		// perform clipping
		x2 = dstrect->x + (dstrect->w - 1);
		y2 = dstrect->y + (dstrect->h - 1);
		cx2 = dst->cliprect.x + (dst->cliprect.w - 1);
		cy2 = dst->cliprect.y + (dst->cliprect.h - 1);
		
		if (x2 > cx2) x2 = cx2;
		if (y2 > cy2) y2 = cy2;
		
		if (dstrect->x < dst->cliprect.x) dstrect->x = dst->cliprect.x;
		if (dstrect->y < dst->cliprect.y) dstrect->y = dst->cliprect.y;
		
		// rect is entirely offscreen?
		if (dstrect->x > x2 || dstrect->y > y2)
			return 0;
		
		dstrect->w = (x2 - dstrect->x) + 1;
		dstrect->h = (y2 - dstrect->y) + 1;
		
		uint16_t *line = GET_PIXEL_ADDR(dst, dstrect->x, dstrect->y);
		int fill_length = dstrect->w;
		int fill_add = (dst->w - fill_length);
		
		int y = dstrect->h;
		while(y--)
		{
			int x = fill_length;
			while(x--)
				*(line++) = color;
			
			line += fill_add;
		}
		
		return 0;
	}
}

/*
void c------------------------------() {}
*/

// create a 16bpp surface
SDL_Surface *SDLS_CreateSurface(int width, int height, bool use_colorkey)
{
	uint16_t *pixels = (uint16_t *)malloc(width * height * 2);
	return SDLS_SurfaceFromRaw(pixels, width, height, width * 2, use_colorkey);
}

SDL_Surface *SDLS_SurfaceFromRaw(uint16_t *pixels, int width, int height, \
								int pitch, bool use_colorkey)
{
SDL_Surface *sfc;

	sfc = (SDL_Surface *)malloc(sizeof(SDL_Surface));
	
	sfc->pixels = pixels;
	sfc->format = &SDLS_StdFormat;
	
	sfc->w = width;
	sfc->h = height;
	sfc->pitch = pitch;
	
	sfc->use_colorkey = use_colorkey;
	sfc->free_pixels = true;
	
	sfc->cliprect.x = 0;
	sfc->cliprect.y = 0;
	sfc->cliprect.w = sfc->w;
	sfc->cliprect.h = sfc->h;
	
	return sfc;
}

void SDLS_DumpRect(SDL_Rect *rect)
{
	stat("<%d, %d, %d, %d>", rect->x, rect->y, rect->w, rect->h);
}

void SDL_ShowCursor(int enable)	{ }
int SDL_SetAlpha(SDL_Surface *surface, uint32_t flag, uint8_t alpha) {return -1;
}

/*
void c------------------------------() {}
*/

void SDL_GetRGB(uint32_t pixel, SDL_PixelFormat *fmt, uint8_t *r, uint8_t *g, uint8_t *b)
{
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors)
{
	return 0;
}



