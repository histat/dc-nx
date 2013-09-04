#ifndef _RONIN_DC_TIME_H
#define _RONIN_DC_TIME_H been_here_before

/*
 * Time and timer-related functions.
 */
#include <sys/types.h>
#include <time.h>
#include "common.h"
#define USEC_TO_TIMER(x) (((x)*100)>>11)
#define TIMER_TO_USEC(x) (((x)<<11)/100)
START_EXTERN_C
unsigned long Timer( );
void usleep( unsigned int usecs );
END_EXTERN_C

#endif //_RONIN_DC_TIME_H
