#include "MBmagCP.h"
#include "spi_uart.h"
#include "math.h"
#include "utility.h"
#include "mod_time.h"
#include "ucos_ii.h"

#define mbmagcpRead()
#define mbmagcpWrite()

/*
正向总量定义
D4、D3、D2、D1、D0需要先将压缩BCD码规格化为十进制格式，具体如下：
			D4 = ( D4 >> 4) *10 + (D4 & 0x0F)
			D3 = ( D3 >> 4) *10 + (D3 & 0x0F)
			D2 = ( D2 >> 4) *10 + (D2 & 0x0F)
D1 = ( D1 >> 4) *10 + (D1 & 0x0F)
			D0 = ( D0 >> 4) *10 + (D0 & 0x0F)

其中 符号 >> 为C语言中的逻辑右移操作。

	总量 = 100000000 * D4 + 1000000 * D3 + 10000 * D2 + 100 * D1 + D0;
D5字节:
D5	0	1	2	3	4	5	6	7
单位	0.001L	0.01L	0.1L	1L	0.001m3	0.01m3	0.1m3	1m3
D5	8	9	10	11	12	13	14	15
单位	0.001kg	0.01kg	0.1kg	1kg	0.001t	0.01t	0.1t	1t
5）反向总量定义
D4、D3、D2、D1、D0需要先将压缩BCD码规格化为十进制格式，具体如下：
			D4 = ( D4 >> 4) *10 + (D4 & 0x0F)
			D3 = ( D3 >> 4) *10 + (D3 & 0x0F)
			D2 = ( D2 >> 4) *10 + (D2 & 0x0F)
D1 = ( D1 >> 4) *10 + (D1 & 0x0F)
			D0 = ( D0 >> 4) *10 + (D0 & 0x0F)

其中 符号 >> 为C语言中的逻辑右移操作。

	总量 = 100000000 * D4 + 1000000 * D3 + 10000 * D2 + 100 * D1 + D0;
D5字节:
D5	0	1	2	3	4	5	6	7
单位	0.001L	0.01L	0.1L	1L	0.001m3	0.01m3	0.1m3	1m3
D5	8	9	10	11	12	13	14	15
单位	0.001kg	0.01kg	0.1kg	1kg	0.001t	0.01t	0.1t	1t


*/

uint8_t * MBmagCPFraming(uint8_t addr, uint8_t code)
{
    int8_t len = 0; 
    uint8_t eor = 0;
    uint32_t time = clock();
    uint8_t buffer[16];
    uint8_t * ret_ptr;
    buffer[0] = 0x2A;
    buffer[1] = addr;
    buffer[2] = code;
    buffer[3] = 0x2E;
    
    SpiUart2Flush();
    SpiUart2Write(0, buffer, 4);
    while (1)
    {
        len += SpiUart2Read(0, buffer+len, 10-len);
        if (len >= 10)
        {
            if ((buffer[0] != addr) || buffer[1] != code)
            {
                return 0;
            }
            for (len = 0; len < 6; len++)
            {
                eor ^= buffer[2+len];
            }
            if (eor != buffer[8])
            {
                return 0;
            }
            ret_ptr = &buffer[2];
            return ret_ptr;
        }
        else if (ComputeTickTime(time) > 500)
        {
            return 0;
        }
        OSTimeDly(10);
    }
}

int8_t MBmagCPReadFlux(float * ret_value)
{
    uint8_t *buffer = MBmagCPFraming(1, 0);
    int8_t exponent;
    uint8_t dir;
    if (buffer)
    {
        exponent = buffer[3]-5;
        dir = buffer[5];
        *ret_value = BCD2HEX(buffer[0]) + BCD2HEX(buffer[1]) * 100 + BCD2HEX(buffer[2]) * 10000;
        *ret_value = *ret_value * pow(10, exponent);
        if (dir)
        {
            *ret_value = 0 - *ret_value;
        }
        return 1;
    }
    return -1;
}

int8_t MBmagCPReadTotalFlux(float * ret_value)
{
    uint8_t *buffer = MBmagCPFraming(1, 4);
    uint8_t units;
    if (buffer)
    {
        units = buffer[5];
        *ret_value = BCD2HEX(buffer[0]) + BCD2HEX(buffer[1]) * 100 + BCD2HEX(buffer[2]) * 10000 +
                     BCD2HEX(buffer[3]) * 1000000 + BCD2HEX(buffer[4]) * 100000000;
        switch (units)
        {
        case 0: *ret_value /= 1000; break;
        case 1: *ret_value /= 100; break;
        case 2: *ret_value /= 10; break;
        case 3: *ret_value /= 1; break;
        case 4: *ret_value /= 10; break;
        case 5: *ret_value /= 100; break;
        case 6: *ret_value /= 1000; break;
        case 7: *ret_value /= 10000; break;
        default: return -2;
        }
        return 1;
    }
    return -1;
}


