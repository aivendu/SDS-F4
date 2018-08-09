#include "..\bsp_includes.h"
#include "cj01_rtc.h"
#include "cj01_irq_priority.h"

uint32_t rtc_ms;
uint8_t  rtc_init_flag;
#define PREDIV_COUNT     0x3FF



static int8_t RTCInit(void)
{
    u16 retry = 0X1FFF;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
    PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 

    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)		//是否第一次配置?
    {
        RCC_LSEConfig(RCC_LSE_ON);//LSE 开启    
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
        {
            retry++;
            OSTimeDly(OS_TICKS_PER_SEC/100);
        }
        if (retry == 0)return 1;		//LSE 开启失败. 

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
        RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟 

        struct tm t_time;
        t_time.tm_year = 2017;
        t_time.tm_mon = 1;
        t_time.tm_mday = 1;
        t_time.tm_wday = 7;
        t_time.tm_hour = 0;
        t_time.tm_min = 0;
        t_time.tm_sec = 0;
        RtcWrite(RTC_PORT, &t_time, sizeof(t_time));   //设置初始日期日时间

        RTC_WriteBackupRegister(RTC_BKP_DR0, 0x5050);	//标记已经初始化过了
    }
    RTC_InitTypeDef RTC_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
	EXTI_InitTypeDef 	EXTI_InitStructure;
    
    RTC_InitStructure.RTC_AsynchPrediv = 0x1F;//RTC异步分频系数(1~0X7F)
    RTC_InitStructure.RTC_SynchPrediv = PREDIV_COUNT;//RTC同步分频系数(0~7FFF)
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;//RTC设置为,24小时格式
    RTC_Init(&RTC_InitStructure);


    //wakeup NVIC 中断配置
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;//串口中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0; //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SUB_PRIORITY_RTC_WKUP;      //子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
    
    EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line=EXTI_Line22;
	EXTI_InitStructure.EXTI_LineCmd =ENABLE;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);
    
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    RTC_WakeUpCmd(DISABLE);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(0);
    RTC_WakeUpCmd(ENABLE);
    RTC_ClearITPendingBit(RTC_IT_WUT);

    return 0;
}


/*****************************************************************************************
** 函数名称 ：	RtcOpen
** 函数功能 ：	打开rtc操作，第一次开启RTC的话，会进行RTC设置，并初始化时间值
** 输    入 ：	int32_t port		 port 系统分配的端口号
** 输    入 ：	void * config		 buf  无效参数
** 输    入 ：	uint8_t len			 buf_len 无效参数
** 输    出 ：	int8_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t RtcOpen(int32_t port, const void *config, uint8_t len)
{
    if (rtc_init_flag == 0)
    {
        rtc_init_flag = 1;
        return RTCInit();
    }
    return 0;
}


/*****************************************************************************************
** 函数名称 ：	RtcClose
** 函数功能 ：
** 输    入 ：	int32_t port
** 输    出 ：	int8_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t RtcClose(int32_t port)
{
    return 0;
}


/*****************************************************************************************
** 函数名称 ：	RtcWrite
** 函数功能 ：	设置rtc时间参数
** 输    入 ：	int32_t port		系统分配的端口号
** 输    入 ：	void * buf			写入的数据地址，为(tm *)类型
** 输    入 ：	uint16_t buf_len	读取的数据长度，此函数为无效参数
** 输    出 ：	int8_t    			 0：写成功 -1:学失败
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t RtcWrite(int32_t port, void * buf, uint32_t buf_len)
{
    struct tm *t_temp = buf;
    ErrorStatus Error;

    RTC_DateTypeDef RTC_DateTypeInitStructure = { 0 };
    if (t_temp->tm_year >= 2000)
    {
        RTC_DateTypeInitStructure.RTC_Year = t_temp->tm_year - 2000;//年
    }
    else
    {
        RTC_DateTypeInitStructure.RTC_Year = t_temp->tm_year;//年
    }

    RTC_DateTypeInitStructure.RTC_Month = t_temp->tm_mon;//月
    RTC_DateTypeInitStructure.RTC_Date = t_temp->tm_mday;//日
    RTC_DateTypeInitStructure.RTC_WeekDay = t_temp->tm_wday;//星期
    Error = RTC_SetDate(RTC_Format_BIN, &RTC_DateTypeInitStructure);

    if (Error == SUCCESS)
    {
        RTC_TimeTypeDef RTC_TimeTypeInitStructure = { 0 };
        RTC_TimeTypeInitStructure.RTC_Hours = t_temp->tm_hour;//时
        RTC_TimeTypeInitStructure.RTC_Minutes = t_temp->tm_min;//分
        RTC_TimeTypeInitStructure.RTC_Seconds = t_temp->tm_sec;//秒
//      RTC_TimeTypeInitStructure.RTC_H12=ampm;
        Error = RTC_SetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure);
    }

    if (Error == ERROR)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


/*****************************************************************************************
** 函数名称 ：	RtcRead
** 函数功能 ：	读取Rtc时间
** 输    入 ：	int32_t port			  系统分配的端口号
** 输    入 ：	void * buf				  读到的数据返回地址，类型为( tm *)
** 输    入 ：	uint16_t buf_len		  读取的数据长度，此函数为无效参数
** 输    出 ：	int16_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int32_t RtcRead(int32_t port, void *buf, uint32_t buf_len)
{
    struct tm *t_temp = buf;

    RTC_TimeTypeDef RTC_TimeTypeInitStructure;
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure);

    t_temp->tm_hour = RTC_TimeTypeInitStructure.RTC_Hours;
    t_temp->tm_min = RTC_TimeTypeInitStructure.RTC_Minutes;
    t_temp->tm_sec = RTC_TimeTypeInitStructure.RTC_Seconds;

    RTC_DateTypeDef  RTC_DateStruct;
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

    t_temp->tm_year = RTC_DateStruct.RTC_Year + 2000;
    t_temp->tm_mon = RTC_DateStruct.RTC_Month;
    t_temp->tm_mday = RTC_DateStruct.RTC_Date;
    t_temp->tm_wday = RTC_DateStruct.RTC_WeekDay;

    return 0;
}


/*****************************************************************************************
** 函数名称 ：	RtcIoctl
** 函数功能 ：	无功能，始终返回-1，应用层不能调用该函数，只是为了适应架构
** 输    入 ：	int32_t port
** 输    入 ：	uint32_t cmd
** 输    入 ：	va_list args
** 输    出 ：	int8_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t RtcIoctl(int32_t port, uint32_t cmd, va_list args)
{
    uint16_t * ms = va_arg(args, uint16_t *);
    switch (cmd)
    {
        case RTC_GETMILLISCOND:
            *ms = rtc_ms;
            break;
        case RTC_REINIT:
            RTCInit();
            break;
    }
    return -1;
}

void RTC_WKUP_IRQHandler(void)
{
    OSIntEnter();
    if(RTC_GetITStatus(RTC_IT_WUT))
	{
		RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI_ClearITPendingBit(EXTI_Line22);
        rtc_ms = 0;
    }
    OSIntExit();
}


/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/

