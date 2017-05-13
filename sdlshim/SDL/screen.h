
#ifndef _SCREEN_H
#define _SCREEN_H

struct SDL_Rect
{
	int16_t x, y;
	uint16_t w, h;
};

struct SDL_Color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t unused;
};

struct SDL_Palette
{
  int ncolors;
  SDL_Color *colors;
};

struct SDL_PixelFormat
{
    SDL_Palette *palette;
	uint8_t BitsPerPixel;
	uint8_t BytesPerPixel;
	uint8_t Rloss, Gloss, Bloss, Aloss;
	uint8_t Rshift, Gshift, Bshift, Ashift;
	uint32_t Rmask, Gmask, Bmask, Amask;
};

struct SDL_Surface
{
    uint32_t flags;
	uint16_t *pixels;
	int w, h;
	
	SDL_PixelFormat *format;
	SDL_Rect cliprect;
	uint16_t pitch;
	
	bool use_colorkey;
	bool free_pixels;
};

#define SDL_SWSURFACE       0x00000000  /* Not used */
#define SDL_SRCALPHA        0x00010000
#define SDL_SRCCOLORKEY     0x00020000
#define SDL_ANYFORMAT       0x00100000
#define SDL_HWPALETTE       0x00200000
#define SDL_DOUBLEBUF       0x00400000
#define SDL_FULLSCREEN      0x00800000
#define SDL_RESIZABLE       0x01000000
#define SDL_NOFRAME         0x02000000
#define SDL_OPENGL          0x04000000
#define SDL_HWSURFACE       0x08000001  /* Not used */
#define SDL_ASYNCBLIT       0x08000000  /* Not used */
#define SDL_RLEACCELOK      0x08000000  /* Not used */
#define SDL_HWACCEL         0x08000000  /* Not used */


extern "C"
{
	void asm_do_blit_transparent(uint16_t *src, uint16_t *dest, \
								int cpywidth, int cpyheight, \
								int srcpitch, int dstpitch);
	
	void asm_do_blit_nontransparent(uint16_t *src, uint16_t *dest, \
								int cpywidth, int cpyheight, \
								int srcpitch, int dstpitch);
	
	void *asm_fast_memcpy(void *dest, const void *src, size_t length);
	void *asm_fast_memcpy_2(void *dest, const void *src, size_t length);
};

extern SDL_Surface *SDLS_VRAMSurface;

#ifndef	max
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef	min
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#endif
