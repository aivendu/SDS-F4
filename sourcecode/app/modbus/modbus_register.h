#ifndef _MODBUS_REGISTER_H
#define _MODBUS_REGISTER_H

//#include "stm32f10x.h"
#include "stdint.h"
#include "modbus_slave.h"

#define REGISTER_NUM      4//MODBUS������������    
#define COILS_NUM         3

//  ��Ȧ����ṹ
typedef union u_coil_rw
{
    struct s_ctrl
    {
        uint16_t  net    :1;
        uint16_t  sd     :1;
        uint16_t  usb    :1; 
        uint16_t  save   :1;
        uint16_t  calib  :1;
        uint16_t  manual :1;
        uint16_t  unused0:10; 
        
        uint16_t  alarm_sw_pump_lift:1;  //  ������
        uint16_t  alarm_sw_pump_subm:1;  //  Ǳ�۱�
        uint16_t  alarm_sw_pump_rflx:1;  //  ������
        uint16_t  alarm_sw_pump_watr:1;  //  ��ˮ��
        uint16_t  alarm_sw_pump_dosg:1;  //  ��ҩ��
        uint16_t  alarm_sw_pump_aera:1;  //  ������
        uint16_t  alarm_sw_pump_wash:1;  //  ϴĤ��
        uint16_t  unused2:9;
        
        uint16_t  install_T:1;
        uint16_t  install_PH:1;
        uint16_t  install_COD:1;
        uint16_t  install_ORP:1;
        uint16_t  install_NH3:1;
        uint16_t  install_DO:1;
        uint16_t  install_SS:1;
        uint16_t  install_P:1;
        uint16_t  install_FR:1;
        uint16_t  unused3:7;
        
    } ctrl;
    uint16_t coil[3];
}  u_coil_group_1_t;



enum e_pump_st
{
    PUMP_ST_CLOSE,
    PUMP_ST_OPEN,
    PUMP_ST_WARING,
    PUMP_ST_ERROR,
};

typedef struct s_pump_st
{
    uint16_t  pump_lift;  //  ������
    uint16_t  pump_subm;  //  Ǳ�۱�
    uint16_t  pump_rflx;  //  ������
    uint16_t  pump_watr;  //  ��ˮ��
    uint16_t  pump_dosg;  //  ��ҩ��
    uint16_t  pump_aera;  //  ������
    uint16_t  pump_wash;  //  ϴĤ��
} s_pump_st_t;

typedef union u_pump_st
{
    s_pump_st_t pump_st;
    uint16_t regs[sizeof(s_pump_st_t)];
} u_pump_st_t;

#if 1  //  ���ղ����ṹ��
typedef struct s_A2O_technology_argv
{
    uint16_t pump_aera1_port;
    uint16_t pump_aera2_port;
    uint16_t pump_aera3_port;
    uint16_t pump_subm1_port;
    uint16_t pump_subm2_port;
    uint16_t pump_subm3_port;
    uint16_t pump_rflx1_port;
    uint16_t pump_rflx2_port;
    uint16_t pump_watr1_port;
    uint16_t pump_watr2_port;
    uint16_t pump_dosg1_port;
    uint16_t pump_dosg2_port;
    uint16_t pump_lift1_port;
    uint16_t pump_lift2_port;
    uint16_t unused1[48-14];
    
    uint32_t pump_aera1_time;
    uint32_t pump_aera2_time;
    uint32_t pump_aera3_time;
    uint16_t unused2[16-6];
    
    uint16_t pump_subm_on_time;
    uint16_t pump_subm_off_time;
    uint16_t pump_subm_cycle_time;
    uint16_t unused3[16-3];
    
    uint16_t pump_rflx_on_time;
    uint16_t pump_rflx_off_time;
    uint16_t pump_rflx_cycle_time;
    uint16_t unused4[16-3];
    
    uint16_t pump_watr_cycle_time;
    uint16_t unused5[16-1];
    
    uint16_t pump_dosg_delay_time;
    uint16_t unused6[16-1];
    
    uint16_t pump_lift_cycle_time;
    uint16_t unused7[16-1];
} s_A2O_technology_argv_t;

typedef union  u_technology_argv
{
    s_A2O_technology_argv_t a2o;
    uint16_t  registers[256];
} u_technology_argv_t;
#endif

#if 1  //  �������Ĵ����ṹ
//  ���������ƽṹ
typedef struct s_sensor_ctrl
{
    float max;  //  ���ֵ
    float min;  //  ��Сֵ
    float accuracy;  //  ����
    uint16_t port;   //  �˿�
    uint16_t unused;  //  δʹ��
    
    uint16_t calib1;  //  �궨1
    uint16_t sampling1;  //  ����ֵ1
    uint16_t calib2;  //  �궨2
    uint16_t sampling2;  //  ����2
} s_sensor_ctrl_t;

//  ����������
typedef struct s_sensors
{
    float t_value;  //  �¶Ȳ���ֵ
    float ph_value;  //  ph����ֵ
    float cod_value;  //  COD����ֵ
    float orp_value;  //  ORP����ֵ
    float nh3_value;  //  NH3����ֵ
    float DO_value;
    float ss_value;
    float p_value;
    float flux_value;
    float unused1[0x40];
    
    s_sensor_ctrl_t t;
    s_sensor_ctrl_t ph;
    s_sensor_ctrl_t cod;
    s_sensor_ctrl_t orp;
    s_sensor_ctrl_t nh3;
    s_sensor_ctrl_t DO;
    s_sensor_ctrl_t ss;
    s_sensor_ctrl_t p;
    s_sensor_ctrl_t flux;
} s_sensors_t;
    
typedef union u_sensor_reg_rw
{ 
    s_sensors_t ctrl;
    uint16_t reg[sizeof(s_sensors_t)];
} u_sensor_reg_t;
#endif


extern  s_modbus_register_t parkinglock_register[REGISTER_NUM];
extern s_modbus_coils_t sds_coils[COILS_NUM];
void Updata_Device_info(void);
void Updata_Realtime_info(void);
void UpdataControlRegister(void);


extern uint16_t relay_out;
extern uint16_t yw_input;
extern uint16_t senser_ad[4];
extern uint16_t relay_state[14];
extern u_pump_st_t  pump_state;



#endif
