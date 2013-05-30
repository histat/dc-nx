
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "stat.fdh"

char LogFilename[1024] = { 0 };

void SetLogFilename(const char *f)
{
	strcpy(LogFilename, f);
	remove(f);
}

void stat(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	puts(buf);
	fflush(stdout);
	
	if (LogFilename[0])
	{
		FILE *fp = fopen(LogFilename, "a+");
		if (fp)
		{
			fprintf(fp, "%s %s\n", GetTimestamp(), buf);
			fclose(fp);
		}
	}
}

void statnocr(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	fprintf(stdout, "%s", buf);
	fflush(stdout);
	
	if (LogFilename[0])
	{
		FILE *fp = fopen(LogFilename, "a+");
		if (fp)
		{
			fprintf(fp, "%s", buf);
			fclose(fp);
		}
	}
}

void staterr(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	fprintf(stdout, "%s\n", buf);
	fflush(stdout);
	
	if (LogFilename[0])
	{
		FILE *fp = fopen(LogFilename, "a+");
		if (fp)
		{
			fprintf(fp, "%s << %s >>\n", GetTimestamp(), buf);
			fclose(fp);
		}
	}
}

const char *GetTimestamp(void)
{
static char buffer[1024];
	
	time_t ts = time(NULL);
	struct tm *tm = localtime(&ts);
	strftime(buffer, sizeof(buffer)-1, "%m/%d/%y %I:%M:%S%p", tm);
	
	return buffer;
}

