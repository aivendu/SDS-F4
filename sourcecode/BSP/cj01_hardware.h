#ifndef __HARDWARE__
#define __HARDWARE__
#include "stdint.h"


/*IO口类型定义*/
typedef struct
{
    uint8_t dir;               //要配置的IO口方向,0--输出，1--输入
    uint8_t data;              //要配置的IO口的输出状态
    void (*hook)(uint32_t);
} s_GpioStr_t;

/*Uart口类型定义*/
typedef struct
{
    uint32_t baud      : 20;  //要配置的波特率：1～115200
    uint32_t data_b    : 4;   //要配置的数据位,有效值为：5，6，7，8
    uint32_t parity    : 3;   //奇偶校验位，0-无校验NONE,1--奇校验ODD,2--偶校验EVEN
    uint32_t stop_b    : 2;   //要配置的停止位,有效值为：1，2
    uint32_t unused    : 3;   //
} s_UartStr_t;


//I2C口初始化类型定义
typedef   struct
{
    uint32_t  i2c_speed : 22;         //通信速率 0-400000
    uint32_t  mode      : 1;          //工作模式，1:主，0:从
    uint32_t  int_flag  : 1;          //是否使用中断方式， 1:使用，0:不使用
    uint32_t  i2c_adr   : 8;          //设置为从模式时，Adr表示从地址
} s_I2CStr_t;


//I2C读写要配置的参数
typedef   struct
{
    uint8_t  sla;               //器件从地址
    uint8_t  suba_type;         //子地址结构    1－单字节地址  2－双字节地址    3－8+X结构
    uint32_t suba;              //器件内部物理地址 CAT1025地址范围:0－2047,FM14V10地址范围:0-131071
    uint8_t  *w_r_data;         //将要写入或读出的数据的指针
    uint32_t w_r_num;           //将要写入或读出的数据的个数
} s_I2cWriteStr_t;

typedef struct
{
    uint32_t mode      : 1;
    uint32_t addr      : 7;
    uint32_t speed     : 24;
} s_spi_t;




#endif
