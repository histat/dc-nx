/*
 * Copyright 2004-2007, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef _SUPPORT_DEFS_H
#define _SUPPORT_DEFS_H

#include <stdint.h>

/* this must be located before the include of sys/types.h */
#if !defined(_SYS_TYPES_H) && !defined(_SYS_TYPES_H_)
typedef unsigned long			ulong;
typedef unsigned int			uint;
typedef unsigned short			ushort;
#endif

//#include <BeBuild.h>
//#include <Errors.h>

//#include <sys/types.h>


/* Shorthand type formats */
typedef	int8_t			int8;
typedef uint8_t			uint8;
typedef volatile int8_t		vint8;
typedef volatile uint8_t	vuint8;

typedef	int16_t			int16;
typedef uint16_t 		uint16;
typedef volatile int16_t	vint16;
typedef volatile uint16_t	vuint16;

typedef	int32_t			int32;
typedef uint32_t		uint32;
typedef volatile int32_t	vint32;
typedef volatile uint32_t	vuint32;

typedef	int64_t			int64;
typedef uint64_t		uint64;
typedef volatile int64_t	vint64;
typedef volatile uint64_t	vuint64;

typedef volatile long		vlong;
typedef volatile int		vint;
typedef volatile short		vshort;
typedef volatile char		vchar;

typedef volatile unsigned long	vulong;
typedef volatile unsigned int	vuint;
typedef volatile unsigned short	vushort;
typedef volatile unsigned char	vuchar;

typedef unsigned char		uchar;
typedef unsigned short          unichar;


/* Descriptive formats */
typedef int32			status_t;
typedef int64			bigtime_t;
typedef uint32			type_code;
typedef uint32			perform_code;


/* Empty string ("") */
#ifdef __cplusplus
extern const char *B_EMPTY_STRING;
#endif

#if 0
#define INT8_MAX	0x7f
#define INT16_MAX	0x7fff
#define INT32_MAX	0x7fffffff
#define INT64_MAX	0x7fffffffffffffff
#define UINT8_MAX	0xff
#define UINT16_MAX	0xffff
#define UINT32_MAX	0xffffffffffffffff
#endif

/* min and max comparisons */
#ifndef __cplusplus
#	ifndef min
#		define min(a,b) ((a)>(b)?(b):(a))
#	endif
#	ifndef max
#		define max(a,b) ((a)>(b)?(a):(b))
#	endif
#endif

/* min() and max() won't work in C++ */
#define min_c(a,b) ((a)>(b)?(b):(a))
#define max_c(a,b) ((a)>(b)?(a):(b))


/* Grandfathering */
//#ifndef __cplusplus
//#	include <stdbool.h>
//#endif

#ifndef NULL
#	define NULL (0)
#endif


/* Obsolete or discouraged API */

/* use 'true' and 'false' */
#ifndef FALSE
#	define FALSE	0
#endif
#ifndef TRUE
#	define TRUE		1
#endif

#endif	/* _SUPPORT_DEFS_H */
