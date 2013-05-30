
#ifndef _BMPLOADER_H
#define _BMPLOADER_H

#define BMP_HEADER_LEN		0x36

struct BMPHeader
{
	uint16_t type;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t data_offset;
	
	uint32_t info_size;
	
	uint32_t bmWidth, bmHeight;
	uint16_t planes;
	uint16_t bpp;
	
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biColorsUsed;
	uint32_t biColorsImportant;
};





#endif
