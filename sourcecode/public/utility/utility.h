#ifndef _UTILITY_H
#define _UTILITY_H

#include "stdint.h"

#define  _STR(a)           #a
#define  STR(a)            _STR(a)
#define  JOINT_STR(a,b)    a##b

typedef union u_size_conv
{
    uint8_t  ui8[4];
    int8_t   i8[4];
    uint16_t ui16[2];
    int16_t  i16[2];
    uint32_t ui32;
    int32_t  i32;
} u_size_conv_t;

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
};
#endif

/*  ip的数据为大端模式  */
typedef struct s_ipv4
{
    uint8_t ip1;   //  ip 的第一个数字
    uint8_t ip2;   //  ip 的第二个数字
    uint8_t ip3;   //  ip 的第三个数字
    uint8_t ip4;   //  ip 的第四个数字
} s_ipv4_t;
typedef union u_ipv4
{
    s_ipv4_t s;    //  ip 的结构
    uint8_t  ip[4];//  ip 数字数组
    uint32_t ip_w; //  ip 的整数形式
} u_ipv4_t;
/*
**  描述:   IP 字符串转换为整形的ip 模式，ip 间隔只能是空格和'.', 否则判定为转换失败
**  输入:   ip -- ip的字符串格式xxx.xxx.xxx.xxx,xxx取值范围0-255
**          ip_return -- ip的整数格式
**  返回:   0 -- 转换成功，
**          1 -- 转换失败, 中间出现非法字符，或者转换出来的整数大于255
**  说明:   字符串自能有'.'、空格、数字组成
**  调用:    memset() -- 对数据进行赋值操作
**  作者:   杜其俊/aiven
**  日期:   2014-4-21
*/
extern int8_t CharToIP(const char *ip, s_ipv4_t *const ip_return);
/*  转换ip的大小端模式  */
#define ChangeIPByteOrder(a) ((s_ipv4_t)(((a)->ip1 << 24) + ((a)->ip2 << 16) + ((a)->ip2 << 8) + ((a)->ip2)))



/**********************************************************************
// 原型: int8_t str2int(uint8_t scale, uint8_t strlen, const char *str, uint8_t * ret)
//
// 功能: 字符串转整形数
//
// 参数: scale --  进制，只能为2,8,10,16
//   strlen -- 字符串长度,当为0时,长度有字符串str确定，str必须有结束符，
//   str -- 待转换的字符串
//   ret -- 转换完成的整数
//
// 返回: -1 -- 转换失败
//   0 -- 转换成功
//
// 资源: 局部变量: 5byte

// 说明: 该函数只能转换二进制32位数，
   八进制10位数，十进制9位，十六进制8位。
************************************************************************/
extern int8_t strtoint(uint8_t scale, uint8_t s_len, const char *str, uint8_t *ret);

#endif

