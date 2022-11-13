
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "file.fdh"

static bool GetFullFilename(const char *fname, char *buffer)
{
  //printf("%s:%s\n",__func__, fname);

	const char *p;
	int i;
	int len;
	//int pos;
	char *path = buffer;

	len = strlen(fname);
	
	//pos = 0;
	for(i=0; i<len; i++) {
	  p = fname + i;

	  if (!strncmp(p, "..", 2)) {
	    fname += 2;
	    break;
	  }
	}
	sprintf(buffer, "/cd/%s", fname);
	
	return 0;
	
}

/*
void c------------------------------() {}
*/

FILE *SDLS_fopen(const char *fname, const char *mode)
{
	char buffer[256];
	if (GetFullFilename(fname, buffer))
		return NULL;

	//printf("%s:%s\n",__func__, buffer);
	
	return fopen(buffer, mode);
}









