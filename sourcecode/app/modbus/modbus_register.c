#include "modbus_register.h"
#include "string.h"
#include "modbus_slave.h"
#include "sys_config.h"
#include "modbus_core.h"
#include "mod_time.h"

extern void UpdataStatusInfo(void);
extern uint8_t device_addr;



const uint8_t  Device_info[32] = {""}; 

uint16_t relay_out;
uint16_t yw_input;
uint16_t senser_ad[4];
uint16_t relay_state[14];
char     userpassword[100];
uint16_t permission[256/16] = {1};

u_pump_st_t  pump_state;

uint32_t RTCSet(struct s_modbus_register *reg, const uint8_t *data)
{
    uint16_t buffer[12];
    uint16_t addr = (data[1]<<8) + data[2];
    uint16_t len = (data[3]<<8) + data[4];
    struct tm new_time;
    uint32_t time_tt;
    if ((len != 6) || (addr != reg->start_addr))
    {
        return 2;
    }
    HalfWordBigEndianCopy(buffer, (uint8_t *)&data[6], 12);
    new_time.tm_year = buffer[0];
    new_time.tm_mon = buffer[1];
    new_time.tm_mday = buffer[2];
    new_time.tm_hour = buffer[3];
    new_time.tm_min = buffer[4];
    new_time.tm_sec = buffer[5];
    time_tt = mktime(&new_time);
    time(&time_tt);
    return 0;
}


uint32_t PermissionVerify(struct s_modbus_register *reg, const uint8_t *data)
{
    char * userpassword = (char *)&data[6+8];  //  mcgs屏幕发送的字符串有抬头:"mcgsstr:"
//    uint16_t len = ((data[3]<<8) + data[4]) * 2;  //  一个字节2个byte
    char *name, *password;
    
    name = strtok_r(userpassword, ",", &userpassword);
    password = strtok_r(0, ",", &userpassword);
    if ((strncmp(name, sys_config_ram.username, sizeof(sys_config_ram.username)) == 0) &&
        (strncmp(password, sys_config_ram.password, sizeof(sys_config_ram.password)) == 0))
    {
        permission[0] = 1;
    }
    return 0;
}

/******************************************************************
** 函数名称:   SetFirmwareUpdataState
** 功能描述:   更改程序更新的状态
** 输入:	   func_code：功能码
**			   *dat：写入的数据
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:	   
** 作者:	   arjun
** 日期:	   20160316
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
        //  继电器输出
        0x0001,
        0x000E,
        (1 << MODBUS_FUNC_CODE_01) +
        (1 << MODBUS_FUNC_CODE_05) + 
        (1 << MODBUS_FUNC_CODE_0F),
        (uint8_t *)&relay_out,
        0
    },
    {
        //  bool 量传感器输入信号
        0x0400,
        0x0008,
        (1 << MODBUS_FUNC_CODE_01) + 
        (1 << MODBUS_FUNC_CODE_02),
        (uint8_t *)&yw_input,
        0
    },
    {
        //  功能控制输入输出
        0x0800,
        sizeof(coil_group_1.coil) * 8,
        (1 << MODBUS_FUNC_CODE_01) +
        (1 << MODBUS_FUNC_CODE_05) +
        (1 << MODBUS_FUNC_CODE_0F),
        (uint8_t *)coil_group_1.coil,
        0
    },
    {
        //  功能控制输入输出
        0xFE00,
        sizeof(permission) * 8,
        (1 << MODBUS_FUNC_CODE_01) +
        (1 << MODBUS_FUNC_CODE_05) +
        (1 << MODBUS_FUNC_CODE_0F),
        (uint8_t *)permission,
        0
    }
};

#define GetRegisterLen(a)     (sizeof(a)>>1)

s_modbus_register_t parkinglock_register[REGISTER_NUM] =
{
    {
        // 1  设备标识
        0x0000,
        0x0006,
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_10),
        0,
        RTCSet
    },
    {
        // 1  设备标识
        0x0006,
        GetRegisterLen(s_reg_group_1_t),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) + 
        (1 << MODBUS_FUNC_CODE_10),
        (uint16_t *)&process_technology_type,
        0
    },
    {
        // 泵的状态
        0x0010,
        GetRegisterLen(pump_state.regs),
        (1 << MODBUS_FUNC_CODE_03),
        pump_state.regs,
        0
    },
    {
        // 1  传感器
        0x0100,
        GetRegisterLen(sensor_reg),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        sensor_reg.reg,
        0
    },
    {
        // 工艺参数
        0x0400,
        GetRegisterLen(a2o_technology_argv.member),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        a2o_technology_argv.registers,
        0
    },
    {
        // 工艺参数
        0x0500,
        GetRegisterLen(mbr_technology_argv.member),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        mbr_technology_argv.registers,
        0
    },
    {
        // 工艺参数
        0x0600,
        GetRegisterLen(sbr_technology_argv.member),
        (1 << MODBUS_FUNC_CODE_03) + 
        (1 << MODBUS_FUNC_CODE_06) +
        (1 << MODBUS_FUNC_CODE_10) ,
        sbr_technology_argv.registers,
        0
    },
    {
        // 工艺参数
        0xFE00,
        GetRegisterLen(userpassword),
        (1 << MODBUS_FUNC_CODE_10) ,
        0,//(uint16_t *)userpassword,
        PermissionVerify
    },
};


