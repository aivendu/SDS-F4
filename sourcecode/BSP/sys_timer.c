//#include "os_includes.h"
#include "sys_timer.h"
#include "stm32f4xx.h"
#include "os_cfg.h"
#include "includes.h"
#include "cj01_rtc.h"
//#include "cpal_conf.h"
#define SYSTEM_SUPPORT_OS      1

#define delay_ostickspersec OS_TICKS_PER_SEC
//初始化延迟函数
//当使用ucos的时候,此函数会初始化ucos的时钟节拍
//SYSTICK的时钟固定为AHB时钟的1/8
//SYSCLK:系统时钟频率
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS                       //如果需要支持OS.
    u32 reload;
#endif
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);//SYSTICK使用外部时钟源
    //  fac_us=SYSCLK/8;                        //不论是否使用OS,fac_us都需要使用
#if SYSTEM_SUPPORT_OS                       //如果需要支持OS.
    reload = SYSCLK / 8;                        //每秒钟的计数次数 单位为K
    reload *= 1000000 / delay_ostickspersec;    //根据delay_ostickspersec设定溢出时间
    //reload为24位寄存器,最大值:16777216,在72M下,约合1.86s左右
    //  fac_ms=1000/delay_ostickspersec;        //代表OS可以延时的最少单位
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; //开启SYSTICK中断
    SysTick->LOAD = reload;                     //每1/OS_TICKS_PER_SEC秒中断一次
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开启SYSTICK
#else
    fac_ms = (u16)fac_us * 1000;                //非OS下,代表每个ms需要的systick时钟数
#endif
}


void SysTimeInit(void)
{
    delay_init(168);       //延时初始化
}

extern void Reset_Handler(void);

void Reset(void)
{
    //  Reset_Handler();
    NVIC_SystemReset();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	INT8U error;
//	static uint8_t kkkkkkkk = 0;
//	if (kkkkkkkk)
//	{
//		kkkkkkkk = 0;
//		GPIO_SetBits(GPIOC, GPIO_PIN_0);
//	}
//	else
//	{
//		kkkkkkkk = 1;
//		GPIO_ResetBits(GPIOC, GPIO_PIN_0);
//	}
    extern uint8_t modbus_datatimeout;
    if (modbus_datatimeout)   modbus_datatimeout--;
    if (rtc_ms < 999)    rtc_ms++;
 	if(OSRunning==1)					//OS开始跑了,才执行正常的调度处理
	{
		OSIntEnter();						//进入中断
		OSTimeTick();       				//调用ucos的时钟服务程序               
		OSIntExit();       	 				//触发任务切换软中断
		OSFlagPost(bsp_os_flag, OS_FLAG_TIMER_5MS, OS_FLAG_SET, &error);
	}
}





#define DEBUG_PORT      0xff
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义 _sys_exit() 以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
void _ttywrch(int ch)
{
	ch = ch ;
}

//int _sys_open(void)
//{
//    return 0;
//}

#endif
