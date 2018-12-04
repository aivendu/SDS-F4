#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "ucos_ii.h"
#include "my_time.h"
#include "cj01_rtc.h"
#include "ucos_ii.h"
#include "cj01_hardware.h"
#include "cj01_io_api.h"



#include "cj01_io_api.h"
#define TIME_API_TYPE      1  //  0--使用clock_t类型配置；1--使用struct tm类型配置
#define GetLocalTime(t)    {IoOpen(RTC_PORT, 0, 0); IoRead(RTC_PORT, t, sizeof(struct tm)); IoClose(RTC_PORT);}
#define SetLocalTime(t)    {IoOpen(RTC_PORT, 0, 0); IoWrite(RTC_PORT, t, sizeof(struct tm)); IoClose(RTC_PORT);}
#define SystemTick()       OSTimeGet()

//  获取从当前年到0年的闰年数量, 包括当年
#define GetLeapYearNumber(a)  (((a) / 4) - ((a) / 100) + ((a) / 400))
//  判断当前年是否是闰年
#define IsLeapYear(a)         ((((a) % 4 == 0) && ((a) % 100 != 0)) || ((a) % 400 == 0))

const uint8_t day_Of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint8_t Week(uint16_t year, uint8_t mon, uint8_t dom)
{
    //  计算星期几
    if(mon == 1 || mon == 2)
    {
        mon += 12;
        year--;
    }

    return ((dom + 2 * mon + 3 * (mon + 1) / 5 + year + year / 4 - year / 100 + year / 400 + 1) % 7);
}

/*提供的秒数不能大于*/
struct tm *LocalTime(const time_t *t/*timer*/)
{
    static struct tm t_r;
    uint32_t t_temp = *t / 86400, i;
    t_r.tm_sec = 0;
    t_r.tm_min = 0;
    t_r.tm_hour = 0;
    t_r.tm_mday = 1;
    t_r.tm_mon = 1;
    t_r.tm_year = t_temp / 365 + BASE_OF_TIME;
    //  计算闰年数量
    i = GetLeapYearNumber(t_r.tm_year - 1) - GetLeapYearNumber(BASE_OF_TIME - 1);
    //  计算当年的天数
    t_temp = t_temp % 365;

    if(t_temp < i)
    {
        //  剩余天数小于润年数，前面计算的年份多了一年
        t_r.tm_year--;

        if(IsLeapYear(t_r.tm_year))
        {
            t_temp = 366 + t_temp - i;
        }
        else
        {
            t_temp = 365 + t_temp - i;
        }
    }
    else
    {
        t_temp = t_temp - i;
    }

    //  初步计算当年月数
    i = 0;

    if(IsLeapYear(t_r.tm_year))
    {
        if(t_temp >= 59)
        {
            i = 1;
        }
    }

    if(t_temp < 31)        //  月份等于1
    {
        t_r.tm_mon = 1;
        t_r.tm_mday += t_temp;
    }
    else if((t_temp - i) < 59)       //  月份等于2
    {
        t_r.tm_mon = 2;
        t_r.tm_mday += t_temp - 31;
    }
    else        //  月份大于等于3
    {
        t_temp = t_temp - 59 - i;
        i = 2;

        while((i < 12) && (t_temp >= day_Of_month[i]))
        {
            t_temp -= day_Of_month[i];
            i++;
        }

        t_r.tm_mon += i;
        t_r.tm_mday += t_temp;
    }

    //  计算时间
    t_temp = *t % 86400;
    t_r.tm_hour = t_temp / 3600;
    t_r.tm_min = t_temp % 3600 / 60;
    t_r.tm_sec = t_temp % 60;

    //  计算星期几
    t_r.tm_wday = Week(t_r.tm_year, t_r.tm_mon, t_r.tm_mday);
    return &t_r;
}

time_t MKTime(struct tm *t/*timeptr*/)
{
    uint8_t  iflag = 0;
    uint32_t Continueddaynow = 0;
    if (t->tm_year < BASE_OF_TIME)
    {
        return 0;
    }
    //  计算当年的天数
    if(t->tm_mon >= 3)
    {
        //  判断闰年
        if(IsLeapYear(t->tm_year))
        {
            iflag = 1;
        }
        else
        {
            iflag = 0;
        }

        Continueddaynow = 13 * (t->tm_mon + 1) / 5  - 7 + (t->tm_mon - 1) * 28 + t->tm_mday + iflag;
    }
    else if(t->tm_mon == 1)
    {
        Continueddaynow = t->tm_mday;
    }
    else if(t->tm_mon == 2)
    {
        Continueddaynow = 31 + t->tm_mday;
    }

    //  计算润年年数
    iflag = GetLeapYearNumber(t->tm_year - 1) - GetLeapYearNumber(BASE_OF_TIME - 1);

    Continueddaynow = Continueddaynow +  365 * (t->tm_year - BASE_OF_TIME) + iflag - 1;
    Continueddaynow = ((Continueddaynow * 24 + t->tm_hour) * 60 + t->tm_min) * 60 + t->tm_sec;
    return Continueddaynow;
}

time_t DiffTime(struct tm *t1, struct tm *t2)
{
    return MKTime(t1) - MKTime(t2);
}

//  获取mcu tick，基于time.h中对时间的定义
clock_t clock(void)
{
    return SystemTick();
}


//  更新本地时间，基于time.h中对时间的定义
time_t time(time_t *t)
{

#if (TIME_API_TYPE==0)
    time_t  t_temp;

    if(t != NULL)
    {
        t_temp = *t;
        SetLocalTime(&t_temp);
        return t_temp;
    }
    else
    {
        GetLocalTime(&t_temp);
        return t_temp;
    }

#else
    struct tm rtc;

    if(t != NULL)
    {
        rtc = *LocalTime(t);
        SetLocalTime(&rtc);
        return *t;
    }
    else
    {
        GetLocalTime(&rtc);
        return MKTime(&rtc);
    }

#endif
}


//  描述:   时间的字符串格式转整数格式
//  输入:   format -- 时间的字符串格式，YYYY-MM-DD hh:mm:ss, 分割符可以是除[Y,M,D,h,m,s,\0]以外的任何字符
//          time -- time的数据必须是数字字符，分割符必须与format匹配, 字符前可以有空格
//          time_r -- 时间的整数格式
//  输出:   0 -- 转换成功
//          1 -- 转换失败
//const static char time_format_capitalchar[] = "YYYY-MM-DD hh:mm:ss";
uint8_t CharToTime(char *format, char *time, struct tm *time_r)
{
    struct tm time_temp;
    uint8_t i, j;

    /*if ((strstr(time_format_capitalchar , format) == NULL)
        || (strstr(time_format_lowerchar , format) == NULL))
    {
        //    判断字符串格式是否正确
        return 1;
    }*/
    while(1)
    {
        //  去除时间字符串前面的空格
        if((*time >= '0') && (*time <= '9'))
        {
            //    发现正确字符，跳出开始转换
            break;
        }
        else if(*time == ' ')
        {
            //    空格删除
            time++;
        }
        else
        {
            //    发现错误字符
            return 1;
        }
    }

    memset(&time_temp, 0, sizeof(time_temp));
    i = 0;
    j = 0;

    while(1)
    {
        switch(format[i])
        {
            case 'Y':
                j++;

                if(j > 4)
                {
                    //    字符太多
                    return 1;
                }

                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_year = time_temp.tm_year * 10 + time[i] - '0';
                break;

            case 'M':
                j++;

                if(j > 2)
                {
                    //    字符太多
                    return 1;
                }

                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_mon = time_temp.tm_mon * 10 + time[i] - '0';

                break;

            case 'D':
                j++;

                if(j > 2)
                {
                    //    字符太多
                    return 1;
                }

                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_mday = time_temp.tm_mday * 10 + time[i] - '0';
                break;

            case 'h':
                j++;

                if(j > 2)
                {
                    //    字符太多
                    return 1;
                }

                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_hour = time_temp.tm_hour * 10 + time[i] - '0';
                break;

            case 'm':
                j++;

                if(j > 2)
                {
                    //    字符太多
                    return 1;
                }

                //  解析分钟
                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_min = time_temp.tm_min * 10 + time[i] - '0';
                break;

            case 's':
                j++;

                if(j > 2)
                {
                    //    字符太多
                    return 1;
                }

                if((time[i] < '0') || (time[i] > '9'))
                {
                    //  字符错误
                    return 1;
                }

                time_temp.tm_sec = time_temp.tm_sec * 10 + time[i] - '0';
                break;

            case '\0':
                *time_r = time_temp;
                return 0;

            default
                    :
                if(format[i] == time[i])
                {
                    j = 0;
                    break;
                }
                else
                {
                    return 1;
                }
        }

        i++;
    }
}

//倒计时处理，输入开始计时时间，输出从开始计时已经过了多长时间
uint32_t ComputeTickTime(uint32_t time)
{
    uint32_t time_t = clock();

    if (time_t >= time)
    {
        return time_t - time;
    }
    else
    {
        return 0xFFFFFFFF - time + time_t;
    }
}





