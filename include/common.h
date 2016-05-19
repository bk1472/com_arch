#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<termios.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<string.h>
#include	<errno.h>
#include	<ar.h>
#include	<time.h>
#include	<unistd.h>

/*-----------------------------------
 * Type definitions
 */
typedef unsigned char		UINT08;
typedef unsigned short		UINT16;
typedef unsigned long		UINT32;
typedef unsigned long long	UINT64;

typedef signed char			SINT08;
typedef signed short		SINT16;
typedef signed long			SINT32;
typedef signed long long	SINT64;

typedef unsigned char		UCHAR;
typedef unsigned short		USHORT;
typedef unsigned long		ULONG;

typedef unsigned char		BYTE;
typedef unsigned char		BOOL;

#define BUSWIDTH			UINT16

extern	int	print0n(const char *format , ... );
extern	int	print1n(const char *format , ... );

#ifdef __cplusplus
}
#endif

#endif/*__COMMON_HEADER__*/
