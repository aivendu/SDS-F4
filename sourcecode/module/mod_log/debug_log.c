#include "mod_time.h"
#include "debug_log.h"
#include "cj01_hardware.h"
#include "cj01_io_api.h"
#include "cj01_rtc.h"
#if    DEBUG_ENABLE==1

#define DEBUG_PORT    COM1

#define DEBUG_PRINT_DATE         0
#define DEBUG_PRINT_TIME         1  //  
#define DEBUG_PRINT_FILE         0  // 


#define debug_vprintf   vprintf

int fputc(int ch, FILE * fp)
{
    IoWrite(DEBUG_PORT, &ch, 1);
    return 1;
}


static void DebugPrintf(const char *format, ...)
{
    va_list argv;
    va_start(argv, format);
    debug_vprintf(format, argv);
    va_end(argv);
}



#define DEBUG_PRINT_TIME         1  //  控制是否打印时间信息
#define DEBUG_PRINT_FILE         0  //  控制是否打印文件信息


/* 过滤条件，通过这里的配置，修改打印出的调试信息内容，返回1表示不打印 */
uint8_t FilterCondition(char *type)
{
    if ((strncmp(type, "malloc", 6) == 0) ||
        (strncmp(type, "free", 4) == 0) ||
        0)
    {
        //  条件满足
        return 1;
    }
    else
    {
        //  条件不满足
        return 0;
    }
}


/******************************************************************
** 函数名称:   DebugPrintf_1
** 功能描述:   打印调试信息，打印时间和文件信息等。支持条件过滤，字符格式化与printf一致
** 输入:
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
int DebugPrint(void *file, uint32_t line, char *type, char *format, ...)
{
    int32_t err = 0;
    uint8_t prio= OSTCBCur->OSTCBPrio;

#ifdef __DEBUG_FLASH
#if    DEBUG_ENABLE
    va_list argv;
	struct tm rtc_datetime;
	uint32_t clock_time=time(0);  //  获取时间
	s_UartStr_t debug_uart = {115200, 8, 0 , 1};
    if (FilterCondition(type))
    {
		return 0;
    }
	IoOpen(DEBUG_PORT, &debug_uart, sizeof(debug_uart));
    if ((DEBUG_PRINT_DATE) || (DEBUG_PRINT_TIME))
    {
        memcpy(&rtc_datetime, localtime(&clock_time), sizeof(struct tm));  //  转换时间
        DebugPrintf("<");
    	if (DEBUG_PRINT_DATE)
    	{
    	    /* 打印时间 */
            DebugPrintf("%04u-%02u-%02u ", rtc_datetime.tm_year, 
        	    	  rtc_datetime.tm_mon, rtc_datetime.tm_mday);
        }
    	if (DEBUG_PRINT_TIME)
    	{
            uint16_t ms;
            IoOpen(RTC_PORT, 0, 0);
            Ioctl(RTC_PORT, RTC_GETMILLISCOND, &ms);
            IoClose(RTC_PORT);
    	    /* 打印时间 */
            DebugPrintf("%02u:%02u:%02u.%03u", rtc_datetime.tm_hour,
        		          rtc_datetime.tm_min, rtc_datetime.tm_sec,
                          ms);		
    	}
        DebugPrintf(">");
    }
	if (DEBUG_PRINT_FILE)
	{
        DebugPrintf("$%s:%u$ ", (char *)file, line);  //  打印类型
	}
    if (type)	DebugPrintf(" [%u %s] ", prio, type);
	va_start(argv, format);
	err = debug_vprintf(format, argv);  //  打印内容
	va_end(argv);
	IoClose(DEBUG_PORT);
#endif
#endif
    return err;
}

/******************************************************************
** 函数名称:   DebugPrintf_1
** 功能描述:   打印调试信息，不打印时间，文件信息等。支持条件过滤，字符格式化与printf一致
** 输入:
**
** 输出:
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
int DebugPrint_1(char *type, char *format, ...)
{
    int32_t err = 0;
#ifdef __DEBUG_FLASH
#if    DEBUG_ENABLE
    va_list argv;
    s_UartStr_t debug_uart = { 115200, 8, 0 , 1 };
    if (FilterCondition(type))
    {
        return 0;
    }
    IoOpen(DEBUG_PORT, &debug_uart, sizeof(debug_uart));
    va_start(argv, format);
    err = debug_vprintf(format, argv);  //  打印内容
    va_end(argv);

    IoClose(DEBUG_PORT);
#endif
#endif
    return err;
}
#endif









