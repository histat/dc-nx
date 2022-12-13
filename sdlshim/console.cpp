
#include "shim.h"
#include "console.fdh"

#include "font.cpp"
//#define ENABLE_SCROLL

static int cursorx = 0, cursory = 0;
static uint8_t cursor_visible = 1;
static uint32_t cursor_timer = 0;
static char cursor_disabled = 0;

static uint16_t *console=NULL;


bool console_init(void)
{
	cursor_timer = SDL_GetTicks();

	stat("Console initilized.");
	return 0;
}

void console_close(void)
{
}

// switch between a visible console and drawing to an offscreen buffer.
void set_console_visible(bool enable)
{
	bool enabled = (console == vram);
	if (enable == enabled) return;

	console = vram;
}


static void printchar_internal(char ch)
{
	if (ch == 8)	// backspace
	{
		if (cursorx == 0)
		{
			cursorx = SCREEN_CHARS_WIDTH-1;
			if (cursory) cursory--;
		}
		else cursorx--;

		draw_char(cursorx*FONT_WIDTH, cursory*FONT_HEIGHT, ' ');
		return;
	}

	if (ch != ' ' && ch != '\n')
		draw_char(cursorx*FONT_WIDTH, cursory*FONT_HEIGHT, ch);

	cursorx++;
	if (cursorx >= SCREEN_CHARS_WIDTH || ch == '\n')
	{
		cursorx = 0;
		cursory++;

		if (cursory >= SCREEN_CHARS_HEIGHT)
		{
			#ifdef ENABLE_SCROLL
				scroll_up();
				cursory = SCREEN_CHARS_HEIGHT - 1;
			#else
				memset(console, 0, SCREEN_PITCH*SCREEN_HEIGHT);
				cursorx = cursory = 0;
			#endif
		}
	}
}

void printchar(char ch)
{
	DrawCursor(0);
	printchar_internal(ch);

	DrawCursor(1);
}

void printstr(const char *str, bool append_cr)
{
	if(console != vram)
	  return;

	DrawCursor(0);

	while(*str)
		printchar_internal(*(str++));

	if (append_cr) printchar_internal('\n');
	DrawCursor(1);
}

void moveback(int amt)
{
int i;
	for(i=0;i<amt;i++)
	{
		if (cursorx == 0)
		{
			cursorx = SCREEN_CHARS_WIDTH-1;
			if (cursory) cursory--;
		}
		else cursorx--;
	}
}

void gotoxy(int x, int y)
{
	if (cursorx != x || cursory != y)
	{
		DrawCursor(0);
		cursorx = x;
		cursory = y;
		DrawCursor(1);
	}
}

/*
void c------------------------------() {}
*/

void DisableCursor(uint8_t newdis)
{
	if (cursor_disabled != newdis)
	{
		cursor_disabled = newdis;
		if (cursor_visible != cursor_disabled)
			DrawCursor(cursor_disabled);
	}
}

void DrawCursor(uint8_t newvis)
{
	if (cursor_disabled) newvis = 0;
	if (cursor_visible != newvis)
	{
		fillcursor(cursorx*FONT_WIDTH, cursory*FONT_HEIGHT, newvis?0xff:0);
		cursor_visible = newvis;
	}
}

void cursor_run(void)
{
uint32_t curtime = SDL_GetTicks();

	if (curtime - cursor_timer >= 500)
	{
		cursor_timer = curtime;
		DrawCursor(cursor_visible ^ 1);
	}
}

/*
void c------------------------------() {}
*/

// draws a char from fontdata[] array
void draw_char(int x, int y, char ch)
{
uint16_t *vramline = &console[(y * SCREEN_WIDTH) + x];
const uint8_t *letrdata = &fontdata[ch * 8];
int xa, ya;

	for(ya=0;ya<8;ya++)
	{
		// draw one line
		uint8_t linedata = *(letrdata++);
		uint16_t *vramptr = vramline;
		uint8_t mask = 0x80;

		for(xa=0;xa<8;xa++)
		{
			if (linedata & mask)
				*(vramptr++) = 0xffff;
			else
				*(vramptr++) = 0x0000;

			mask >>= 1;
		}

		vramline += SCREEN_WIDTH;
	}
}

static void scroll_up(void)
{
	// move lines up
	int length = (SCREEN_HEIGHT*SCREEN_PITCH) - (FONT_HEIGHT*SCREEN_PITCH);
	memmove(console, &console[FONT_HEIGHT * SCREEN_WIDTH], length);

	// clear bottom line
	uint16_t *clear_ptr = &console[(SCREEN_HEIGHT - FONT_HEIGHT) * SCREEN_WIDTH];
	memset(clear_ptr, 0, SCREEN_PITCH * FONT_HEIGHT);
}

static void fillcursor(int x, int y, uint8_t color)
{
uint16_t *ptr = &console[(y * SCREEN_WIDTH) + x];
int i;

	for(i=0;i<FONT_HEIGHT;i++)
	{
		memset(ptr, color, FONT_WIDTH*2);
		ptr += SCREEN_WIDTH;
	}
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

	printstr(buffer, true);
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

	printstr(buffer, false);
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

	printstr(buffer, true);
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
#ifndef NOSERIAL
  strcpy(logfilename, fname);
#endif
}

void writelog(const char *buf, bool append_cr)
{
#ifndef NOSERIAL
  fprintf(stderr, "%s",buf);
  if(append_cr)
    puts("\n");
#endif
}
