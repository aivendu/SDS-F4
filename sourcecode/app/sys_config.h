﻿#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "stdint.h"
#include "modbus_register.h"

enum
{
    CM_GPRS,
    CM_4G,
};


typedef struct 
{
    uint32_t device_type;
    uint16_t version;
    uint16_t crc;
    //uint16_t tech_type;
    s_reg_group_1_t reg_group_1;
    u_A2O_technology_argv_t a2o_argv;
    u_MBR_technology_argv_t mbr_argv;
    u_SBR_technology_argv_t sbr_argv;
    u_sensor_reg_t  sensor;
    u_coil_group_1_t  coil_g1;
    char username[64];
    char password[64];
    uint8_t com_mode;
    uint8_t dhcp;
    char gprs_apn[64];
    char gprs_apn_username[64];
    char gprs_apn_passwd[64];
    uint32_t local_ip;
    uint32_t local_submask;
    uint32_t local_gateway;
    uint32_t local_dns1;
    uint32_t local_dns2;
} s_sys_config_t;

//  端口定义: 0:  未使用                                				
//            1-0x3F: MODBUS#1-#63  MODBUS接口的地址1到地址63                                				
//            0x40-0x5F: ADC#1-#32   ADC1-ADC32口                            				
//            0x60:   COM1: 接COM1口                            				
//            0x70:   COM2: 接COM2口                            				
//            0x80-0x8F:   RS485-1#1-#16: RS485-1的其他协议地址1到地址16                            				
//            0x90-0x9F:   RS485-2#1-#16: RS485-2的其他协议地址1到地址16 
typedef enum e_sensor_port
{
    SENSOR_PORT_NONE = 0,  //  端口未使用
    SENSOR_PORT_MODBUS = 0x01,  //  使用 RS485 MODBUS
    SENSOR_PORT_ADC = 0x40,     //  使用 ADC
    SENSOR_PORT_COM1 = 0x60,    //  使用 COM1
    SENSOR_PORT_COM2 = 0x70,    //  使用 COM2
}e_sensor_port_t;

//  检测传感器端口是否正确，正确返回1，不正确返回0
extern int8_t CheckSenorPort (e_sensor_port_t type, uint8_t port);

#define SensorPortMODBUS(a)    (SENSOR_PORT_MODBUS+(a)-1)
#define SensorPortADC(a)       (SENSOR_PORT_ADC+(a)-1)
#define SensorPortCOM1(a)      (SENSOR_PORT_COM1+(a)-1)
#define SensorPortCOM2(a)      (SENSOR_PORT_COM2+(a)-1)

extern s_sys_config_t  sys_config_ram;
extern const s_sys_config_t sys_config_rom;

#define process_technology_type     sys_config_ram.reg_group_1.technology_type
#define a2o_technology_argv         sys_config_ram.a2o_argv
#define mbr_technology_argv         sys_config_ram.mbr_argv
#define sbr_technology_argv         sys_config_ram.sbr_argv
#define sensor_reg                  sys_config_ram.sensor
#define coil_group_1                sys_config_ram.coil_g1


extern int16_t InitFileSystem(void);
extern int16_t SaveSystemCfg(uint32_t index, uint8_t *buff, uint32_t len);

#define SaveSysConfig(a)   \
         SaveSystemCfg((uint32_t)&(a) - (uint32_t)&sys_config_ram, (uint8_t *)&(a), sizeof(a))
#define SaveSensorCfg()  SaveSysConfig(sys_config_ram.sensor)

#endif
