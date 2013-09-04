/* Output to the serial slave. Mainly used for debuggging. */

#include <stdarg.h>
#include "serial.h"
#include "report.h"
#include "dc_time.h"

//! Returns the int @[x] converted to a string.
//FIXME: Move it to some string lib or something instead.
char *itoa(int x) 
{ 
  static char buf[30];
  int minus = 0;
  int ptr=29;
  buf[29]=0;

  if(!x) return "0";
  if( x < 0 )  {  minus=1;  x = -x; }
  while(x > 0) 
  { 
    buf[--ptr] = x%10 + '0'; 
    x/=10; 
  }
  if( minus ) buf[--ptr] = '-';
  return buf+ptr; 
}

//! Prints @[str] to the serial port and flushes it. Turned into an
//! empty function if NOSERIAL is defined.
//!
//! Use this function for debug output.
void report(const char *str)
{
#ifndef NOSERIAL
  serial_puts(str); 
  serial_flush();
  usleep(1000); //FIXME: Make serial_flush sleep depending on bps set instead.
#endif
}

//! Outputs a string to the serial port based on the format string
//! @[fmt]. Turned into an empty function if NOSERIAL is defined.
//!
//! Use this function for debug output.
void reportf(const char *fmt, ...)
{
#ifndef NOSERIAL
  int p;
  va_list va;
  va_start(va, fmt);
  while((p = *fmt++))
    if(p=='%')
      switch(*fmt++) 
      {
       case '\0': --fmt;    break;

       case 's': serial_puts( va_arg(va, char *) );   break;
       case '%': serial_putc('%'); break;
       case 'd': serial_puts( itoa(va_arg(va, int)) ); break;
       case 'p': serial_puts("(void *)0x");
       case 'x': 
       {
         char buf[9];
         int i, d;
         int n = va_arg( va, int );
         for(i=0; i<8; i++) 
         {
           d = n&15;
           n>>=4;
           if(d<10)
             buf[7-i]='0'+d;
           else
             buf[7-i]='a'+(d-10);
         }
         buf[8]=0;
         serial_puts(buf);
         break;
       }
       case 'b':
       {
         char bits[33];
         int i, d = va_arg( va, int);
         bits[32]=0;
         for( i = 0; i<31; i++ )
           if( d & (1<<i) )
             bits[31-i] = '1';
           else
             bits[31-i] = '0';
         report( bits );
	 break;
       }
      }
    else
      serial_putc(p);
  va_end(va);
  serial_flush();
  usleep(1000);
#endif /* SERIAL */
}
