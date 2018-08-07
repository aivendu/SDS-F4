#include "modbus_register.h"
#include "string.h"
#include "modbus_slave.h"
#include "sys_config.h"


extern void UpdataStatusInfo(void);
extern uint8_t device_addr;


const uint8_t  Device_info[32] = {""}; 

uint16_t relay_out;
uint16_t yw_input;
uint16_t senser_ad[4];
uint16_t relay_state[14];

u_pump_st_t  pump_state;

/******************************************************************
** ��������:   SetFirmwareUpdataState
** ��������:   ���ĳ�����µ�״̬
** ����:	   func_code��������
**			   *dat��д�������
** ���:	   ��
** ȫ�ֱ���:
** ����ģ��:
** ��ע:	   
** ����:	   arjun
** ����:	   20160316
******************************************************************/
uint32_t ModbusResetSystem(uint8_t func_code, const uint8_t *dat)
{
    if (dat[3] == 2)
    {
        reboot_flag = 1;
        return 0;
    }
    return 2;
}    
s_modbus_coils_t sds_coils[COILS_NUM] =
{
    {
        //  �̵������
        0x0000,
        0x000E,
        (1 << MODBUS_FUNC_CODE_05) + 
        (1 << MODBUS_FUNC_CODE_0F),
        (uint8_t *)&relay_out,
        0
    },
    {
        //  bool �������������ź�
        0x0400,
        0x0008,
        (1 << MODBUS_FUNC_CODE_01),
        (uint8_t *)&yw_input,
        0
    },
    {
        //  ���ܿ����������
        0x0800,
        0x0030,
        (1 << MODBUS_FUNC_CODE_01) +
        (1 << MODBUS_FUNC_CODE_05) +
        (1 << MODBUS_FUNC_CODE_0F),
        (uint8_t *)coil_group_1.coil,
        0
    }
};

s_modbus_register_t parkinglock_register[REGISTER_NUM] =
{
    {
        // 1  �豸��ʶ
        0x0005,
        0x0001,
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06),
        (uint16_t *)&process_technology_type,
        0
    },
    {
        // �õ�״̬
        0x000F,
        sizeof(pump_state.regs),
        (1 << MODBUS_FUNC_CODE_03),
        pump_state.regs,
        0
    },
    {
        // 1  ������
        0x00FF,
        sizeof(sensor_reg),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        sensor_reg.reg,
        0
    },
    {
        // ���ղ���
        0x03FF,
        sizeof(technology_argv.a2o),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        technology_argv.registers,
        0
    },
};


