#ifndef __IRQ_PRIORITY__
#define __IRQ_PRIORITY__
#include "stdint.h"


/*中断优先级定义*/
typedef enum
{
    PREEMPTION_PRIORITY_0,        
} e_NVIC_IRQCHANNELPREEMPTIONPRIORITY_t;    //抢占式优先级别

typedef enum
{
    SUB_PRIORITY_RTC_WKUP,
    SUB_PRIORITY_ETH,        //网络芯片
    SUB_PRIORITY_USART1,        //串口1    
    SUB_PRIORITY_USART2,        //串口2   
    SUB_PRIORITY_USART3,        //串口3   
    SUB_PRIORITY_USART6,        //串口6    
    SUB_PRIORITY_USARTWK,        //串口WK
    SUB_PRIORITY_TIMER3,        //定时器3
    SUB_PRIORITY_PS2,        //PS2键盘
    SUB_PRIORITY_SDCARD,     //SD卡
} e_NVIC_IRQCHANNELSUBPRIORITY_t;    //副优先级别


#endif
