#ifndef _RONIN_SERIAL_H
#define _RONIN_SERIAL_H been_here_before

/*
 * Funktions for managing the DC serial port. Should not normally be
 * used in the final product, but only for debugging.
 */
#include "common.h"

START_EXTERN_C
void serial_init(int baudrate);
void serial_puts(const char *message);
void serial_putc(int c);
int  serial_getc();
int  serial_getc_blocking();
void serial_flush();
END_EXTERN_C

#endif //_RONIN_SERIAL_H
