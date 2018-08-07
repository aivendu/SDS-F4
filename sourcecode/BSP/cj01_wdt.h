#ifndef _WDT_H
#define _WDT_H
#include "stdint.h"
#if (__DEBUG_FLASH == 0)            //Release模式下的看门狗设置
#define WDT_ENABLE				0	//	1--看门狗使能，0--看门狗禁止
#else                               //Debug模式下的看门狗设置
#define WDT_ENABLE				0	//	1--看门狗使能，0--看门狗禁止
#endif


#define WDT_INTERRUPT_EN		0	//	1--看门狗溢出产生中断，0--溢出产生芯片复位
#define WDT_CHANGE_TIME         1


extern int8_t Wdt_Init(int32_t port, void *config);

extern int32_t Wdt_Read(int32_t port, void *buf, uint32_t buf_len);
extern int8_t Wdt_Close(int32_t port);
extern int8_t Wdt_Write(int32_t port, void *buf, uint32_t buf_len);
extern int8_t Wdt_Open(int32_t port, const void *config, uint8_t len);
extern int8_t Wdt_Ioctl(int32_t port, uint32_t cmd, va_list argp);

extern void WDT_Exception(void);

#endif

