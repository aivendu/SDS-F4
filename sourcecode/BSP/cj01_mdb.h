#ifndef _MDB_H
#define _MDB_H
#include "stdint.h"

#if 1	//改动MDB时需要改动的参数

#define MDB_GPIO_PORT_TX							GPIOB //发送GPIO端口
#define MDB_GPIO_PORT_RX							GPIOB   //接收GPIO端口
#define MDB_GPIO_PIN_SOURCE_TX						GPIO_PinSource10//发送复用端口
#define MDB_GPIO_PIN_SOURCE_RX						GPIO_PinSource11 //接收复用端口
#define MDB_GPIO_AF_USART							GPIO_AF_USART3//复用串口
#define MDB_GPIO_PIN_TX								GPIO_PIN_10//发送引脚PIN
#define MDB_GPIO_PIN_RX								GPIO_PIN_11 //接收引脚PIN
#define MDB_USART_PORT								USART3//串口端口
#define MDB_USART_IRQ_N								USART3_IRQn//中断通道
#define MDB_IRQ_CHANNEL_PREEMPTION_PRIORITY			PREEMPTION_PRIORITY_0//抢占优先级
#define MDB_IRQ_CHANNEL_SUB_PRIORITY				SUB_PRIORITY_USART3//子优先级

#define MDB_RECV_QUENE								((s_mdb_queue_t *)UART3RecvBuf)
#define MDB_SEND_QUENE								((s_mdb_queue_t *)UART3SendBuf)
#define MDB_RECV_QUEUE_BUFF_LEN						(sizeof(UART3RecvBuf))
#define MDB_SEND_QUEUE_BUFF_LEN						(sizeof(UART3SendBuf))

#define	MDB_ENABLE_GPIO_RCC()						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)//使能GPIO时钟
#define	MDB_ENABLE_USART_RCC()						RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)//使能USART时钟
#endif




extern int16_t MdbRead(int32_t port, void *buf, uint16_t buf_len);
extern int8_t MdbIoctl(int32_t port, uint32_t cmd, va_list argp);
extern int8_t MdbClose(int32_t port);
extern int8_t MdbWrite(int32_t port, void *buf, uint16_t buf_len);
extern int8_t MdbOpen(int32_t port, void *config, uint8_t len);

/*****************************************************************************************
** 函数名称 ：	MDB_IRQHandler
** 函数功能 ：	MDB中断服务程序需要放进相应的串口中断函数内处理
** 输    入 ：	void
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/08/12
*****************************************************************************************/
extern void MDB_IRQHandler(void);



#define MDB_MODE_SET     0x300
#define MDB_MODE_CLR     0x400

#endif

