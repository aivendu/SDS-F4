#include "..\bsp_includes.h"
#include "cj01_exti.h"




/*****************************************************************************************
** 函数名称 ：	EXTI9_5_IRQHandler
** 函数功能 ：	外部中断处理函数
** 输    入 ：	void				
** 输    出 ：	void    			
** 全局变量 :
** 调用模块 :   PS2_KEY_IRQHandler
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void EXTI9_5_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        PS2_KEY_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line8); //清除中断标志位
    }

#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}

/*****************************************************************************************
** 函数名称 ：	EXTI4_IRQHandler
** 函数功能 ：	外部中断处理函数
** 输    入 ：	void
** 输    出 ：	void
** 全局变量 :
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void EXTI4_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
//        spi_to_uart_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line4); //清除中断标志位
    }

#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}
