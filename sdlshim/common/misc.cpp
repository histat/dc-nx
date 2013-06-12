
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>

#include "basics.h"
#include "misc.fdh"

void stat(const char *fmt, ...);


unsigned short fgeti(FILE *fp)
{
unsigned short value;
	fread(&value, 2, 1, fp);
	return value;
}

unsigned int fgetl(FILE *fp)
{
unsigned int value;
	fread(&value, 4, 1, fp);
	return value;
}

void fputi(unsigned short word, FILE *fp)
{
	fwrite(&word, 2, 1, fp);
}

void fputl(unsigned int word, FILE *fp)
{
	fwrite(&word, 4, 1, fp);
}

// read a string from a file until a null is encountered
void freadstring(FILE *fp, char *buf, int max)
{
int i;

	--max;
	for(i=0;i<max;i++)
	{
		buf[i] = fgetc(fp);
		if (!buf[i])
		{
			return;
		}
	}
	
	buf[i] = 0;
}

// write a string to a file and null-terminate it
void fputstring(char *buf, FILE *fp)
{
	if (buf[0]) fprintf(fp, "%s", buf);
	fputc(0, fp);
}

// write a string to a file-- does NOT null-terminate it
void fputstringnonull(char *buf, FILE *fp)
{
	if (buf[0]) fprintf(fp, "%s", buf);
}

// reads strlen(str) bytes from file fp, and returns true if they match "str"
bool fverifystring(FILE *fp, const char *str)
{
bool result = 1;

	for(int i=0;str[i];i++)
	{
		if (fgetc(fp) != str[i])
			result = 0;		// don't stop reading so file cursor is correct
	}
	
	return result;
}

// read data from a file until CR
void fgetline(FILE *fp, char *str, int maxlen)
{
int k;
	str[0] = 0;
	fgets(str, maxlen - 1, fp);
	
	// trim the CRLF that fgets appends
	for(k=strlen(str)-1;k>=0;k--)
	{
		if (str[k] != 13 && str[k] != 10) break;
		str[k] = 0;
	}
}

// returns the size of an open file.
int filesize(FILE *fp)
{
int cp, sz;

	cp = ftell(fp);
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, cp, SEEK_SET);
	
	return sz;
}

// return a random number between min and max inclusive
int randrange(int min, int max)
{
int range, val;

	range = (max - min);
	
	if (range < 0)
	{
		//error("random(): warning: max < min [%d, %d]\n", min, max);
		min ^= max;
		max ^= min;
		min ^= max;
		range = (max - min);
	}
	
	if (range >= RAND_MAX)
	{
		//error("random(): range > RAND_MAX\n", min, max);
		return 0;
	}
	
	val = rand() % (range + 1);
	return val + min;
}


void strtoupper(char *str)
{
int i;
	for(i=strlen(str)-1;i>=0;i--) str[i] = toupper(str[i]);
}

void strtolower(char *str)
{
int i;
	for(i=strlen(str)-1;i>=0;i--) str[i] = tolower(str[i]);
}

char *GetStaticStr(void)
{
static int counter = 0;
static struct
{
	char str[1024];
} bufs[50];

	if (++counter >= 50) counter = 0;
	return bufs[counter].str;
}

char *stprintf(const char *fmt, ...)
{
va_list ar;
char *str = GetStaticStr();

	va_start(ar, fmt);
	vsprintf(str, fmt, ar);
	va_end(ar);
	
	return str;
}

bool file_exists(const char *filename)
{
FILE *fp;

	fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		return true;
	}
	
	return false;
}

// given a full path to a file, return onto the name of the file.
// the pointer returned is within the original string.
const char *GetFileSpec(const char *file_and_path)
{
	char *ptr = (char *)strrchr(file_and_path, '/');
	if (ptr)
		return ptr+1;
	else
		return file_and_path;
}

// a strncpy that works as you might expect
void maxcpy(char *dst, const char *src, int maxlen)
{
int len = strlen(src);

	if (len >= maxlen)
	{
		if (maxlen >= 2) memcpy(dst, src, maxlen - 2);
		if (maxlen >= 1) dst[maxlen - 1] = 0;
	}
	else
	{
		memcpy(dst, src, len + 1);
	}
}


bool strbegin(const char *bigstr, const char *smallstr)
{
int i;
	for(i=0;smallstr[i];i++)
		if (bigstr[i] != smallstr[i]) return false;
	return true;
}


void hexdump(const uint8_t *data, int len)
{
int i;
int off = 0;
const uint8_t *linedata;
char line[800];
char *ptr;

	do
	{
		linedata = &data[off];
		sprintf(line, "  %02X: ", off);
		
		// print 16 chars of hex data
		for(i=0;i<16;i++)
		{
			if (off+i < len)
				sprintf(line, "%s%02x", line, linedata[i]);
			else
				strcat(line, "  ");
			
			if (i & 1) strcat(line, " ");
		}
		
		strcat(line, "    ");
		
		// print the same chars again, as ASCII data
		ptr = &line[strlen(line)];
		for(i=0;i<16;i++)
		{
			if (off+i >= len) break;
			*(ptr++) = ((linedata[i] > 30 && linedata[i] < 129) ? linedata[i] : '.');
		}
		
		//*(ptr++) = '\n';
		*ptr = 0;
		
		stat("%s", line);
		off += 0x10;
	}
	while(off < len);
}

