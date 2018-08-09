#include "..\bsp_includes.h"
#include "cj01_wdt.h"

/*****************************************************************************************
** 函数名称 ：	Wdt_Open
** 函数功能 ：  打开看门狗
** 输    入 ：	int32_t port		无效	
** 输    入 ：	void * config		无效	
** 输    入 ：	uint8_t len			无效
** 输    出 ：	int8_t  			无效
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Wdt_Open(int32_t port, const void *config, uint8_t len)
{
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    if (WDT_ENABLE)
    {
        OS_ENTER_CRITICAL();

        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对IWDG->PR IWDG->RLR的写  
        IWDG_SetPrescaler(4); //设置IWDG分频系数
        IWDG_SetReload(1000);   //设置IWDG装载值 ，大概两秒
        IWDG_ReloadCounter(); //reload       
        IWDG_Enable();       //使能看门狗

        OS_EXIT_CRITICAL();
    }

    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Wdt_Close
** 函数功能 ：  禁用看门狗，STM32独立看门狗不能被关闭
** 输    入 ：	int32_t port			无效
** 输    出 ：	int8_t  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Wdt_Close(int32_t port)
{
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    if (WDT_ENABLE)
    {
        OS_ENTER_CRITICAL();
        //独立看门狗不能被关闭

        OS_EXIT_CRITICAL();
    }
    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Wdt_Write
** 函数功能 ：  喂狗
** 输    入 ：	int32_t port			无效
** 输    入 ：	void * buf			    无效
** 输    入 ：	uint16_t buf_len		无效	
** 输    出 ：	int8_t  			    无效
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Wdt_Write(int32_t port, void *buf, uint32_t buf_len)
{
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    if (WDT_ENABLE)
    {
        OS_ENTER_CRITICAL();
        IWDG_ReloadCounter();//喂狗
        OS_EXIT_CRITICAL();
    }
    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Wdt_Read
** 函数功能 ：  读取看门狗定时器的当前值。
** 输    入 ：	int32_t port			IO端口号
** 输    入 ：	void * buf			    要读的数据存放地址
** 输    入 ：	uint16_t buf_len		要读的数据长度	
** 输    出 ：	int16_t  			 0：操作成功  -1：操作失败
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int32_t Wdt_Read(int32_t port, void *buf, uint32_t buf_len)
{
//    uint32_t tmp;
//    uint32_t *temp;
    //	temp = (uint32_t *)buf;
    //	tmp = WDTV;
    //	*temp = tmp;
    return 0;
}


/*****************************************************************************************
** 函数名称 ：	Wdt_Ioctl
** 函数功能 ：  对WDT口操作控制
** 输    入 ：	int32_t port			
** 输    入 ：	uint32_t cmd			
** 输    入 ：	va_list argp			
** 输    出 ：	int8_t  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Wdt_Ioctl(int32_t port, uint32_t cmd, va_list argp)
{
    int8_t ret=0;
//    uint32_t temp = va_arg(argp, uint32_t);
    //	switch (cmd)
    //	{
    //	case WDT_CHANGE_TIME:
    //		WDTC = (temp / 10) * (Fpclk / 4 / 100);
    //		ret = 0;
    //		break;
    //	default:
    //		ret = -1;
    //		break;
    //	}
    return ret;
}


/*****************************************************************************************
** 函数名称 ：	WDT_Exception
** 函数功能 ：  WDT中断服务程序，STM32独立看门狗没有中断
** 输    入 ：	void			
** 输    出 ：	void  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void WDT_Exception(void)
{
    //STM32独立看门狗没有中断
//	OS_ENTER_CRITICAL();
//	if (WDT_ENABLE)
//	{
//		//	异常处理代码
//	}
//	else
//	{
//		//	禁止看门狗
//		WDMOD = 0;
//	}
//	//VICVectAddr = 0;
//	OS_EXIT_CRITICAL();
}
