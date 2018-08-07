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
    //  ��һ�������ǹ����룬�ڶ��������ǽ��յ��Ĺ������Ժ������
    //  ����С��0x100��ʾ������룬0--û�д��󣬴���0x100��ʾ��Ҫ���ص�����
    uint32_t (*func)(uint8_t, const uint8_t *);
} s_modbus_register_t;


typedef struct
{
    uint16_t file_no;         //  �ļ���
    uint16_t max_record_num;  //  ���ļ�¼��
    void    **file_p;         //  �ļ�ָ��
    uint32_t (*func)(void **, const uint8_t *);  //  �������� ��һ�������ļ�ָ��,�ڶ����������յ�����
} s_modbus_record_t;

typedef struct
{
    uint16_t start_addr;
    int16_t coils_num;
    uint32_t cmd_support[(SUPPORT_CODE_NUMBER + 31) / 32];
    uint8_t  *coils;
    uint32_t (*func)(uint8_t , const uint8_t *);  //  �������� ��һ�������ļ�ָ��,�ڶ����������յ�����
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

