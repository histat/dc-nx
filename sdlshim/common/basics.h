
#ifndef _BASICS_H

#include <stdint.h>
typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;

typedef unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned short	ushort;

#define SWAP(a, b)		{ a ^= b; b ^= a; a ^= b; }

#define WIDTHOF(R)		((R.right - R.left) + 1)
#define HEIGHTOF(R)		((R.bottom - R.top) + 1)

#ifndef bp
	#ifdef __GNUC__
		#define bp	__asm__ ( "int3\n" );
	#else
		#define bp	_asm { int 3 }
	#endif
#endif

#define rept			for(;;)
#define stop			{ smal_bypass_cleanup = 1; staterr("programmer stop"); exit(1); }
#define stop0			{ smal_bypass_cleanup = 1; staterr("programmer stop-0"); exit(0); }
extern bool smal_bypass_cleanup;
#define here			{ staterr("---- Here ----"); }
#define TODO			{ stat("-- TODO: %s %d", __FILE__, __LINE__); }

void stat(const char *fmt, ...);
void staterr(const char *fmt, ...);

#ifndef ASSERT
#ifdef RELEASE
	#define ASSERT(x)
#else
	#define ASSERT(x) \
	{ \
		if (!(x)) \
		{	\
			printf("assertion failed: \"%s\" at %s(%d)\n", #x, __FILE__, __LINE__);	\
			fflush(stdout); \
		}	\
	}
#endif
#endif

#define ON_STARTUP(OBJECT)	\
static class OBJECT	\
{	\
public:	\
	OBJECT();	\
} auto_initer;	\
\
OBJECT::OBJECT()

#endif

