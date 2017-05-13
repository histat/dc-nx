
#include <stdio.h>
#include <string.h>
#include "file.fdh"

static bool GetFullFilename(const char *fname, char *buffer)
{
//	printf("%s:%s\n",__func__, fname);
	const char *p;
	int i;
	int len;

	len = strlen(fname);

	for(i=0; i<len; ++i) {
		p = fname + i;

		if (!strncmp(p, "..", 2)) {
			i += 2;
			p = fname + i;
			strcpy(buffer, p);
			return 0;
		}
	}
	

	strcpy(buffer, fname);
	
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
	
	return fopen(buffer, mode);
}









