
#include <stdio.h>
#include <ronin/ronin.h>
#include "file.fdh"

/*
void c------------------------------() {}
*/

FILE *SDLS_fopen(const char *fname, const char *mode)
{
	return fopen(fname, mode);
}








