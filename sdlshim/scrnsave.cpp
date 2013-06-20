
#include "shim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


typedef struct tagBITMAPFILEHEADER {
  unsigned short bfType;
  unsigned long  bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long  bfOffBits;
} __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImporant;
} __attribute__ ((packed)) BITMAPINFOHEADER;

bool screensave()
{
	FILE *out;
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;
	uint8_t *img;
	char fname[32];
	time_t long_time;
	struct tm *now_time;

    time(&long_time);
    now_time = localtime(&long_time);

    sprintf(fname, "%d%d%d%d%d%d.BMP"
	       ,now_time->tm_year + 1900
	       ,now_time->tm_mon + 1
	       ,now_time->tm_mday
	       ,now_time->tm_hour
	       ,now_time->tm_min
	       ,now_time->tm_sec);

	int w = SCREEN_WIDTH;
	int h = SCREEN_HEIGHT;
	
	out = fopen(fname, "wb");

	if(!out)
		return false;

	memset(&bmf, 0, sizeof(BITMAPFILEHEADER));
	memset(&bmi, 0, sizeof(BITMAPINFOHEADER));

	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = w;
	bmi.biHeight = h;
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biCompression = 0;
	bmi.biSizeImage = w * h * 3;
	
	int offset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	int size = bmi.biSizeImage;
	int fsize = offset + size;
	
	bmf.bfType = 'MB';
	bmf.bfSize = fsize;
	bmf.bfOffBits = offset;
	bmf.bfReserved1 = 0;
	bmf.bfReserved2 = 0;

	img = (uint8_t*)malloc(size);
	
	if(!img)
		return false;

	memset(img, 0, size);

	uint16_t *s = (uint16_t*)vram;
	uint8_t *d;
	
	for(int j=0; j<h; j++) {

		d = img + (w * 3) * ((h - 1) - j);
		
		for(int i=0; i<w; i++) {
			
			uint16_t pix = s[i+j*w];
			
			d[0] = ((pix & 0x1f) >> 0) << 3;
			d[1] = ((pix & 0x7e0) >> 5) << 2;
			d[2] = ((pix & 0xf800) >> 11) << 3;

			d += 3;
		}
	}

	fwrite(&bmf, 1, sizeof(BITMAPFILEHEADER), out);
	fwrite(&bmi, 1, sizeof(BITMAPINFOHEADER), out);

	fwrite(img, 1, size, out);

	free(img);

	fclose(out);

	return true;
}


