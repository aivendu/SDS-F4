#ifndef _USART_H_
#define _USART_H_
#include "stdint.h"
#include "stdarg.h"
#define EN_USART1_RX     1


extern int8_t  UartIoctl(int32_t port, uint32_t cmd, va_list args);
extern int8_t  UartOpen(int32_t port, const void *config, uint8_t len);
extern int8_t  UartClose(int32_t port);
extern int32_t UartRead(int32_t port, void *buf, uint32_t buf_len);
extern int8_t  UartWrite(int32_t port, void *buf, uint32_t buf_len);


#endif
