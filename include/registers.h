#ifndef __REGISTERS_HEADER__
#define __REGISTERS_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct __regs__
{
	UINT16 r[8];
	UINT16 pc[1];
	UINT16 psr[1];
} REGISTER_T;

#ifdef __cplusplus
}
#endif

#endif/*__REGISTERS_HEADER__*/
