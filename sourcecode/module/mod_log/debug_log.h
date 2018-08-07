#ifndef __DEBUG_LOG__
#define __DEBUG_LOG__
#include <stdint.h>
#include "stdint.h"
#include "stdio.h"
#include "string.h"


#define DEBUG_ENABLE      1

#if DEBUG_ENABLE==1
extern int DebugPrint(void *file, uint32_t line, char *type, char *format, ...);
extern int DebugPrint_1(char *type, char *format, ...);

#define Printf_D(type,format,...)     DebugPrint(__MODULE__, __LINE__, type,format, ##__VA_ARGS__)
#define Printf_D_1(type,format,...)   DebugPrint_1(type,format, ##__VA_ARGS__)
#else
#define Printf_D(type,format,...)
#define Printf_D_1(type,format,...)
#endif




#endif
