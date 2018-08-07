#include "stdint.h"
#include "string.h"
#include "utility.h"

/*
**  描述:   IP 字符串转换为整形的ip 模式，ip 间隔只能是空格和'.', 否则判定为转换失败
**  输入:   ip -- ip的字符串格式xxx.xxx.xxx.xxx,xxx取值范围0-255
**          ip_return -- ip的整数格式
**  返回:   0 -- 转换成功，
**          1 -- 转换失败, 中间出现非法字符，或者转换出来的整数大于255
**  说明:   字符串自能有'"'、'.'、空格、数字组成
**  调用:    memset() -- 对数据进行赋值操作
**  作者:   杜其俊/aiven
**  日期:   2014-4-21
*/
int8_t CharToIP(const char *ip, s_ipv4_t *const ip_return)
{
    uint16_t i, ip_temp[4], flag;
    memset(ip_temp, 0, sizeof(ip_temp));

    for (i = 0; i < 4; i++)
    {
        //  查找字段的开始，滤除空格和'.'
        flag = 0;

        while ((*ip < '0') || (*ip > '9'))
        {
            //  字符不是数字
            if ((*ip == ' ') || (*ip == '\"'))
            {
                //  空格忽略
                ip++;
            }
            else if (*ip == '.')
            {
                ip++;

                if (flag == 0)
                {
                    flag = 1;
                }
                else if (flag == 1)
                {
                    //  有超过两个'.'
                    flag = 2;
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }

        if (((i == 0) && (flag == 0))            //  字符串开头不能是'.'
            || ((i != 0) && (flag == 1)))   //  第二个字段前必须要有'.'
        {
            while ((*ip >= '0') && (*ip <= '9'))
            {
                //  字符正确
                ip_temp[i] = ip_temp[i] * 10 + *ip - '0';
                ip++;
            }

            if (ip_temp[i] > 255)
            {
                //  ip地址数字不正确，格式错误
                return -1;
            }
        }
        else
        {
            //    间隔符不只有空格和'.',返回错误
            return -1;
        }
    }

    ip_return->ip1 = (uint8_t)ip_temp[0];
    ip_return->ip2 = (uint8_t)ip_temp[1];
    ip_return->ip3 = (uint8_t)ip_temp[2];
    ip_return->ip4 = (uint8_t)ip_temp[3];
    return 0;
}

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
int8_t strtoint(uint8_t scale, uint8_t s_len, const char *str, uint8_t *ret)
{
    uint8_t temp_int = 0, temp_8 = 0;

    if (strlen(str) && ((scale == 2) || (scale == 8) || (scale == 10) || (scale == 16)))
    {
        // 参数正确，可以转换
    }
    else
    {
        return -1; // 参数不正确，不可以转换
    }

    while ((*str) && ((s_len == 0) || (s_len && (temp_8 < s_len))))
    {
        if ((*str >= '0') && (*str <= '1'))
        {
            temp_int = scale * temp_int + (*str - '0');
        }
        else if ((*str >= '0') && (*str <= '7') && (scale >= 8))
        {
            temp_int = scale * temp_int + (*str - '0');
        }
        else if ((*str >= '0') && (*str <= '9') && (scale >= 10))
        {
            temp_int = scale * temp_int + (*str - '0');
        }
        else if ((*str >= 'A') && (*str <= 'F') && (scale == 16))
        {
            temp_int = scale * temp_int + (*str - 'A') + 10;
        }
        else if ((*str >= 'a') && (*str <= 'f') && (scale == 16))
        {
            temp_int = scale * temp_int + (*str - 'a') + 10;
        }
        else
        {
            return -1;
        }

        str++;
        temp_8++;

        if ((temp_8 % 2) == 0)
        {
            *ret = temp_int;
            ret++;
            temp_int = 0;
        }
    }

    return 0;
}


