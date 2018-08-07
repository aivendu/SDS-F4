#ifndef _LS595_H_
#define _LS595_H_

#include "stdint.h"

#define LS595_RESET               0  //  输出 低电平
#define LS595_SET                 1  //  输出 高电平
#define LS595_REALTIME_OUT     0x80  //  是否需要实时更新输出

extern void LS595Update(uint16_t * value);

extern void LS595OutPut(uint16_t pin, uint8_t state);

/** @defgroup ls595_pin LS595 的引脚定义
  * @{
  */
#define LS595_PWR_24V_EN     0
#define LS595_PWR_12V_EN     1
#define LS595_PWR_5V_EN      2
#define LS595_PWR_IO_EN      3
#define LS595_PWR_UART_EN    4
#define LS595_PWR_NET_EN     5
#define LS595_PWR_MEM_EN     6
#define LS595_ST_LED         7

#define LS595_IO_OUT_01      8
#define LS595_IO_OUT_02      9
#define LS595_IO_OUT_03      10
#define LS595_IO_OUT_04      11
#define LS595_IO_OUT_05      12
#define LS595_IO_OUT_06      13
#define LS595_IO_OUT_07      14
#define LS595_IO_OUT_08      15
/**
  * @}
  */

#endif
