
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>

#include "basics.h"
#include "curses.fdh"

static uchar color_matrix[16][16];
static int foreground_color = 7;
static int background_color = 0;
static int blinking = 0;
static int set_fg = 7;
static int set_bg = 0;
static int set_blinking = 0;
static int nextpair = 1;
static int update_inhibited = 0;

void set_colors()
{
	if (foreground_color == set_fg && \
		background_color == set_bg && \
		blinking == set_blinking)
	{
		return;
	}
	
	static char ega_colors[] = {
		COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
		COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
	};
	
	uchar pair = color_matrix[foreground_color & 7][background_color];
	if (pair == 255)
	{
		pair = nextpair++;
		init_pair(pair, \
				ega_colors[foreground_color & 7], \
				ega_colors[background_color]);
		
		color_matrix[foreground_color & 7][background_color] = pair;
	}
	
	attrset(COLOR_PAIR(pair) | blinking | \
		((foreground_color >= 8) ? A_BOLD: 0));
	
	set_fg = foreground_color;
	set_bg = background_color;
	set_blinking = blinking;
}

void textcolor(int c)
{
	blinking = (c > 16) ? A_BLINK : 0;
	foreground_color = c & 15;
}

void textbg(int c)
{
	background_color = c & 7;
}

/*
void c------------------------------() {}
*/

void clrscr()
{
	gotoxy(0, 0);
	clear();
	
	if (!update_inhibited)
		refresh();
}

void stat(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	set_colors();
	addstr(buf);
	addstr("\n");
	
	if (!update_inhibited)
		refresh();
}

void statnocr(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	set_colors();
	addstr(buf);
	
	if (!update_inhibited)
		refresh();
}

void staterr(const char *str, ...)
{
va_list ar;
char buf[40000];

	va_start(ar, str);
	vsprintf(buf, str, ar);
	va_end(ar);
	
	fprintf(stderr, "%s\n", buf);
	fflush(stderr);
}

void coninhibitupdate(bool enable)
{
	if (enable)
	{
		update_inhibited++;
	}
	else
	{
		if (update_inhibited > 0)
		{
			update_inhibited--;
			
			if (!update_inhibited)
				refresh();
		}
	}
}

/*
void c------------------------------() {}
*/

int conwidth()
{
	return COLS;
}

int conheight()
{
	return LINES;
}

void gotoxy(int x, int y)
{
	move(y, x);
}

int wherex()
{
int x, y;
	getyx(stdscr, y, x);
	return x;
}

int wherey()
{
int x, y;
	getyx(stdscr, y, x);
	return y;
}

/*
void c------------------------------() {}
*/

void sound(int freq, int time_ms)
{
int console_fd = -1;
bool close_it = true;

	#ifndef CLOCK_TICK_RATE
		#define CLOCK_TICK_RATE		1193180
	#endif
	
	if (freq == 0)
		return;
	
	console_fd = open("/dev/console", O_WRONLY);
	if (console_fd == -1)
	{
		staterr("-- FAILED TO OPEN /dev/console");
		console_fd = STDOUT_FILENO;	// try for failsafe
		close_it = false;
	}
	
	ioctl(console_fd, KIOCSOUND, CLOCK_TICK_RATE / freq);
	usleep(time_ms * 1000);
	ioctl(console_fd, KIOCSOUND, 0);
	
	if (close_it)
		close(console_fd);
}

int getkey()
{
	int ch = getch();
	
	// trim out escaped sequences such as left-arrow/right-arrow
	if (ch == 27)
	{
		if (getch() == 91)
		{
			while(getch() >= 0) { }
			return -1;
		}
	}
	
	return ch;
}

int waitkey()
{
	nodelay(stdscr, 0);
	int ch = getch();
	nodelay(stdscr, 1);
	
	// trim out escaped sequences such as left-arrow/right-arrow
	if (ch == 27)
	{
		if (getch() == 91)
		{
			while(getch() >= 0) { }
			return -1;
		}
	}
	
	return ch;
}

/*
void c------------------------------() {}
*/

void init_curses()
{
	initscr();
	start_color();
	nonl();
	cbreak();
	noecho();
	timeout(0);
	curs_set(0);
	scrollok(stdscr, 1);
	nodelay(stdscr, 1);
	
	memset(color_matrix, 255, sizeof(color_matrix));
}

void close_curses()
{
	endwin();
}




