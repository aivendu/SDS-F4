#ifndef _MOD_TIME_H_
#define _MOD_TIME_H_
#include "stdint.h"


#if !((defined __time_h) || (defined TM_S))
#define TM_S
typedef unsigned int clock_t;    /* cpu time type */
typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
struct tm
{
    uint16_t tm_year; /* years since 0 */
    uint8_t  tm_mon;  /* months since January, 1 to 12 */
    uint8_t  tm_mday; /* day of the month, 1 to 31 */
    uint8_t  tm_hour; /* hours since midnight, 0 to 23 */
    uint8_t  tm_min;  /* minutes after the hour, 0 to 59 */
    uint8_t  tm_sec;  /* seconds after the minute, 0 to 59 */
    uint8_t  tm_wday; /* days since Sunday, 0 to 6 */
    uint16_t tm_msec; /* millisecond */
};
#endif

#define BASE_OF_TIME    2000

#define TIME_API_TYPE      1  //  0--使用clock_t类型配置；1--使用struct tm类型配置
#define GetLocalTime(t)    {IoOpen(RTC_PORT, 0, 0); IoRead(RTC_PORT, t, sizeof(struct tm)); IoClose(RTC_PORT);}
#define SetLocalTime(t)    {IoOpen(RTC_PORT, 0, 0); IoWrite(RTC_PORT, t, sizeof(struct tm)); IoClose(RTC_PORT);}


extern const uint8_t day_Of_month[12];


extern uint8_t Week(uint16_t year, uint8_t mon, uint8_t dom);

/*提供的秒数不能大于*/
extern struct tm *LocalTime(const time_t *t/*timer*/);

extern time_t MKTime(struct tm *t/*timeptr*/);

extern time_t DiffTime(struct tm *t1, struct tm *t2);


//  获取本地时间，基于time.h中对时间的定义
extern clock_t clock(void);

//  更新本地时间，基于time.h中对时间的定义
extern time_t time(time_t *t);

extern uint8_t CharToTime(char *format, char *time, struct tm *time_r);


//倒计时处理，输入开始计时时间，输出从开始计时已经过了多长时间
extern uint32_t ComputeTickTime(uint32_t time);

#define localtime(t)    LocalTime(t)
#define mktime(t)       MKTime(t)

#endif

