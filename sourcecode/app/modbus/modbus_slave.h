#ifndef _MODBUS_H
#define _MODBUS_H
#include "stdint.h"
//#include "stm32f10x.h"

#define ModbusLocalAddr()     device_addr
#define Delay_ms(time)       OSTimeDly(time)      


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
};


#define SUPPORT_CODE_NUMBER   19

typedef struct s_modbus_register
{
    uint16_t start_addr;
    int16_t  length;
    uint32_t cmd_support[(SUPPORT_CODE_NUMBER + 31) / 32];
    uint16_t *buffer;
    //  第一个参数是功能码，第二个参数是接收到的功能码以后的数据
    //  返回小于0x100表示错误代码，0--没有错误，大于0x100表示需要返回的数据
    uint32_t (*func)(uint8_t, const uint8_t *);
} s_modbus_register_t;


typedef struct
{
    uint16_t file_no;         //  文件号
    uint16_t max_record_num;  //  最大的记录号
    void    **file_p;         //  文件指针
    uint32_t (*func)(void **, const uint8_t *);  //  操作函数 第一个参数文件指针,第二个参数接收的数据
} s_modbus_record_t;

typedef struct
{
    uint16_t start_addr;
    int16_t coils_num;
    uint32_t cmd_support[(SUPPORT_CODE_NUMBER + 31) / 32];
    uint8_t  *coils;
    uint32_t (*func)(uint8_t , const uint8_t *);  //  操作函数 第一个参数文件指针,第二个参数接收的数据
} s_modbus_coils_t;

extern uint8_t reboot_flag;


extern void HalfWordBigEndianCopy(void *dst, void *src, uint32_t len);

extern void TaskModbus(void);
extern void CreateRegister(s_modbus_register_t *reg, uint8_t num);
void ModbusSlaveReceive(uint8_t *dat);

#define BOOT_FILE_ADDR     BOOTLOADER_ADDR
#define BOTT_FILE_SIZE     BOOTLOADER_SIZE
#define APP_FILE_ADDR      PROGRAM_ADDR_START
#define APP_FILE_SIZE      PROGRAM_SIZE


#endif

