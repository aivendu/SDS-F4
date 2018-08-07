#ifndef __PS2KEY__
#define __PS2KEY__
#include "stdint.h"



#if 1	//改动PS2时需要改动的参数

#define PS2_GPIO_PORT_CLK							GPIOB //时钟GPIO端口
#define PS2_GPIO_PORT_DAT							GPIOB //数据GPIO端口
#define PS2_GPIO_PIN_CLK							GPIO_PIN_8//发送引脚PIN
#define PS2_GPIO_PIN_DAT							GPIO_PIN_9 //接收引脚PIN

#define PS2_EXTI_LINE                               EXTI_Line8                     
#define PS2_IRQ_N								    EXTI9_5_IRQn//中断通道
#define PS2_IRQ_CHANNEL_PREEMPTION_PRIORITY			PREEMPTION_PRIORITY_0//抢占优先级
#define PS2_IRQ_CHANNEL_SUB_PRIORITY				SUB_PRIORITY_PS2//子优先级
        

#define PS2_GPIO_CLK_CLK                            RCC_AHB1Periph_GPIOB
#define PS2_GPIO_DAT_CLK                            RCC_AHB1Periph_GPIOB

#define PS2_EXTI_PORTSOURCE                         EXTI_PortSourceGPIOB
#define PS2_EXTI_PINSOURCE                     EXTI_PinSource8

#endif








#define	KEY_NUM					1//防抖次数


typedef struct
{
	uint32_t value   :7;   // 按键值
	uint32_t press   :1;   // 0--放开，1--按下
	uint32_t count   :24;  // 按下或者松开的时间，单位ms
	void (*depress_func)(void);  //  按下时触发函数
	void (*upspring_func)(void); //  松开时触发函数
} s_keyboard_t;
extern void PS2Timer(void);
extern void PS2_KEY_IRQHandler(void);




extern int32_t Ps2key_Read(int32_t port, void *buf, uint32_t buf_len);
extern int8_t Ps2key_Ioctl(int32_t port, uint32_t cmd, va_list argp);
extern int8_t Ps2key_Close(int32_t port);
extern int8_t Ps2key_Write(int32_t port, void *buf, uint32_t buf_len);
extern int8_t Ps2key_Open(int32_t port, const void *config, uint8_t len);




#endif
