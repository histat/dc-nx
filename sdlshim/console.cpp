
#include "shim.h"
#include "console.fdh"

bool console_init(void)
{
	stat("Console initilized.");
	return 0;
}

void console_close(void)
{
}

// switch between a visible console and drawing to an offscreen buffer.
void set_console_visible(bool enable)
{
}


/*
void c------------------------------() {}
*/

void stat(const char *str, ...)
{
#ifndef NOSERIAL
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);

	puts(buffer);
	fflush(stdout);
#endif
}

void statnocr(const char *str, ...)
{
#ifndef NOSERIAL
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);

	puts(buffer);
	fflush(stdout);
#endif
}

void staterr(const char *str, ...)
{
#ifndef NOSERIAL  
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);


	puts(" error << ");
	puts(buffer);
	puts(" >>\n");

	fflush(stdout);
#endif
}

/*
void c------------------------------() {}
*/

void SetLogFilename(const char *fname)
{
}

void writelog(const char *buf, bool append_cr)
{
}



