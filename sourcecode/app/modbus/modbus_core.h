#ifndef _MODBUS_CORE_H_
#define _MODBUS_CORE_H_

#include  "stdint.h"

enum e_modbus_func_code_index
{
    MODBUS_FUNC_CODE_01,
    MODBUS_FUNC_CODE_02,
    MODBUS_FUNC_CODE_03,
    MODBUS_FUNC_CODE_04,
    MODBUS_FUNC_CODE_05,
    MODBUS_FUNC_CODE_06,
    MODBUS_FUNC_CODE_07,
    MODBUS_FUNC_CODE_08,
    MODBUS_FUNC_CODE_0B,
    MODBUS_FUNC_CODE_0C,
    MODBUS_FUNC_CODE_0F,
    MODBUS_FUNC_CODE_10,
    MODBUS_FUNC_CODE_11,
    MODBUS_FUNC_CODE_14,
    MODBUS_FUNC_CODE_15,
    MODBUS_FUNC_CODE_16,
    MODBUS_FUNC_CODE_17,
    MODBUS_FUNC_CODE_18,
    MODBUS_FUNC_CODE_2B,
//    E_SUPPORT_CODE_NUMBER,
};

#define SUPPORT_CODE_NUMBER   19

//  定义命令格式结构，对每条命令的数据格式进行表格化处理，方便接收时数据的处理
typedef struct
{
    uint8_t code;     //  功能码
    uint8_t length;   //  基础数据长度
    uint8_t length_index;  //  数据长度的索引
    uint8_t data_align    : 3;   //  数据对齐的byte 数
    uint8_t length_type   : 3;   //  数据长度占用的字节数
    uint8_t sub_enable    : 1;   //  有没有子功能 1--有
    //  第一个参数为子功能码，用于有子功能码的接收处理
    uint32_t (*receive_func)(uint8_t);
    //  第一个参数为该功能码的索引编号，第二个字节为接收到的数据
    uint32_t (*response_func)(uint8_t, uint8_t *);
} s_support_code_t;

extern void CopyCoilFromBuffer(uint16_t start, uint16_t size, uint32_t reg, uint8_t *data);
extern void CopyCoilToBuffer(uint16_t start, uint16_t size, uint32_t reg, uint8_t *data);
extern void HalfWordBigEndianCopy(void *dst, void *src, uint32_t len);

#endif
