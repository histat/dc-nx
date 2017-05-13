
#include "shim.h"
#include "support.fdh"

WCHAR *QuickToUnicode(const char *str)
{
int i;

	if (!str) return NULL;
	
	WCHAR *out = (WCHAR *)malloc(strlen(str) * 2);
	for(i=0;str[i];i++)
		out[i] = str[i];
	
	out[i] = 0;
	return out;
}

void MSGBOX(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsnprintf(buf, sizeof(buf), str, ar);
	va_end(ar);
	
	puts(buf);
	fflush(stdout);
	
	WCHAR *unicode = QuickToUnicode(buf);
	MessageBox(NULL, unicode, TEXT(""), MB_OK);
	free(unicode);
}

/*
void c------------------------------() {}
*/

// it's malloc. but the block of memory it returns is guaranteed to
// be 4-byte aligned so you can DWORD-copy from it.
void *malloc_aligned(size_t size)
{
	uint8_t *ptr = (uint8_t *)malloc(size + 4);
	if (!ptr) return NULL;

	int offs = 3 - ((uint32_t)ptr & 3);
	ptr[offs] = offs;
	return (ptr + offs + 1);
}

// memory allocated with malloc_aligned() had dang well BETTER be
// freed using this special function.
void free_aligned(void *ptr)
{
	uint8_t *vptr = (uint8_t *)ptr;
	vptr--;
	vptr -= *vptr;
	free(vptr);
}
























