#ifndef _SYS_TIMER_H
#define _SYS_TIMER_H

#include "stdint.h"

extern void SysTimeInit(void);
extern void Reset(void);
extern void delay_us(uint32_t nus);
#endif
