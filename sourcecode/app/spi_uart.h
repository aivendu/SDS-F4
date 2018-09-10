#ifndef _SPI_UART_H_
#define _SPI_UART_H_
#include "stdint.h"


typedef struct 
{
    uint16_t uart1_in_num;
    uint16_t uart1_out_empty;
    uint16_t uart2_in_num;
    uint16_t uart2_out_empty;
    uint16_t uart3_in_num;
    uint16_t uart3_out_empty;
} s_uart_st_t;
typedef struct s_channel_2
{
    uint8_t relay_stu[42];
    s_uart_st_t  uart_st;
} s_channel_2_t;

extern void InitSpiUart(uint32_t prio);
extern int32_t SpiUart1Write(int32_t port, void *buf, uint32_t buf_len);
extern int32_t SpiUart1Read(int32_t port, void *buf, uint32_t buf_len);
extern int32_t SpiUart2Write(int32_t port, void *buf, uint32_t buf_len);
extern int32_t SpiUart2Read(int32_t port, void *buf, uint32_t buf_len);
extern int32_t SpiUart3Write(int32_t port, void *buf, uint32_t buf_len);
extern int32_t SpiUart3Read(int32_t port, void *buf, uint32_t buf_len);


#endif
