
#include "shim.h"
#include "console.fdh"


bool console_init(void)
{
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

static char logfilename[64] = { 0 };


void stat(const char *str, ...)
{
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);
	
	if (logfilename[0])
		writelog(buffer, true);
}

void statnocr(const char *str, ...)
{
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);
	
	if (logfilename[0])
		writelog(buffer, false);
}

void staterr(const char *str, ...)
{
va_list ar;
char buffer[256];

	va_start(ar, str);
	vsnprintf(buffer, sizeof(buffer), str, ar);
	va_end(ar);
	
	if (logfilename[0])
	{
		writelog(" error << ", false);
		writelog(buffer, false);
		writelog(" >>\n", false);
	}
}

/*
void c------------------------------() {}
*/

void SetLogFilename(const char *fname)
{
	maxcpy(logfilename, fname, sizeof(logfilename));
	remove(logfilename);
	
	stat("Log set %d", SDL_GetTicks());
}

void writelog(const char *buf, bool append_cr)
{
#if 0
	FILE *fp;

	fp = SDLS_fopen(logfilename, "a+");
	if (fp)
	{
		fputs(buf, fp);
		if (append_cr) fputc('\n', fp);
		
		fclose(fp);
	}
#else
	fprintf(stderr, "%s\n", buf);
#endif
}



