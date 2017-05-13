
#ifdef __SDCARD__

#include "scrnsave.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../libsd/ff.h"


extern uint16_t *vram;

static FATFS Fatfs;
static FILINFO _fi;

#ifdef _USE_LFN
static char lfn[_MAX_LFN + 1];
#endif

int sd_init(void)
{
	f_mount(0, &Fatfs);

#ifdef _USE_LFN	
	_fi.lfname = lfn;
	_fi.lfsize = sizeof(lfn);
#endif

	return 1;
}

int sd_exit(void) {

	f_mount(0, NULL);

	return 1;
}

bool screensave(void)
{
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;
	uint8_t *img;
	char fname[32];
	time_t long_time;
	struct tm *now_time;
	FRESULT res;
	FIL fil;
	unsigned int writesize;

    time(&long_time);
    now_time = localtime(&long_time);

    sprintf(fname, "%d%d%d%d%d%d.BMP"
	       ,now_time->tm_year + 1900
	       ,now_time->tm_mon + 1
	       ,now_time->tm_mday
	       ,now_time->tm_hour
	       ,now_time->tm_min
	       ,now_time->tm_sec);

	int w = 320;
	int h = 240;
	
	res = f_open (&fil, fname, FA_WRITE|FA_CREATE_ALWAYS);

	if(res != FR_OK)
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

	f_write (&fil, &bmf, sizeof(BITMAPFILEHEADER), &writesize);
	f_write (&fil, &bmi, sizeof(BITMAPINFOHEADER), &writesize);

	f_write (&fil, img, size, &writesize);

	free(img);

	f_close (&fil);

	return true;
}
#endif
