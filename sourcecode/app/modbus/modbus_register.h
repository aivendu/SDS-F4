#ifndef _MODBUS_REGISTER_H
#define _MODBUS_REGISTER_H

//#include "stm32f10x.h"
#include "stdint.h"
#include "modbus_slave.h"

#define REGISTER_NUM      8//MODBUS功能命令条数    
#define COILS_NUM         4

//  线圈定义结构
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
        uint16_t  init   :1;
        uint16_t  reboot :1;
        uint16_t  unused0:8; 
        uint16_t  unused00:16; 
        
        uint16_t  alarm_pump_1:1;  
        uint16_t  alarm_pump_2:1;  
        uint16_t  alarm_pump_3:1;  
        uint16_t  alarm_pump_4:1;  
        uint16_t  alarm_pump_5:1;  
        uint16_t  alarm_pump_6:1;  
        uint16_t  alarm_pump_7:1;  
        uint16_t  alarm_pump_8:1;  
        uint16_t  alarm_pump_9:1;  
        uint16_t  alarm_pump_10:1; 
        uint16_t  alarm_pump_11:1; 
        uint16_t  alarm_pump_12:1; 
        uint16_t  alarm_pump_13:1; 
        uint16_t  alarm_pump_14:1; 
        uint16_t  unused2:2;
        
        uint16_t  install_t:1;
        uint16_t  install_ph:1;
        uint16_t  install_cod:1;
        uint16_t  install_orp:1;
        uint16_t  install_nh3:1;
        uint16_t  install_DO:1;
        uint16_t  install_ss:1;
        uint16_t  install_p:1;
        uint16_t  install_flux:1;
        uint16_t  unused3:7;
        
    } ctrl;
    uint16_t coil[sizeof(struct s_ctrl) / 2];
}  u_coil_group_1_t;




typedef struct s_reg_group_1
{
    uint16_t technology_type;
    uint16_t server_port;
    uint32_t server_ip;
    uint16_t pump_open_time_min;  //  泵的最小开启时间, 单位S
} s_reg_group_1_t;

enum e_pump_st
{
    PUMP_ST_CLOSE,
    PUMP_ST_OPEN,
    PUMP_ST_WARING,
    PUMP_ST_ERROR,
};

typedef struct s_pump_st
{
    uint16_t  pump_lift;  //  提升泵
    uint16_t  pump_subm;  //  潜污泵
    uint16_t  pump_rflx;  //  回流泵
    uint16_t  pump_watr;  //  出水泵
    uint16_t  pump_dosg;  //  加药泵
    uint16_t  pump_aera;  //  曝气泵
    uint16_t  pump_wash;  //  洗膜泵
} s_pump_st_t;

typedef union u_pump_st
{
    s_pump_st_t pump_st;
    uint16_t regs[sizeof(s_pump_st_t)/2];
} u_pump_st_t;

#if 1  //  工艺参数结构体
typedef struct s_A2O_technology_argv
{
    uint16_t pump_aera1_port;
    uint16_t pump_aera2_port;
    uint16_t pump_aera3_port;
    uint16_t pump_subm1_port;
    uint16_t pump_subm2_port;
    uint16_t pump_standby_port;
    uint16_t pump_rflx1_port;
    uint16_t pump_rflx2_port;
    uint16_t pump_watr1_port;
    uint16_t pump_watr2_port;
    uint16_t pump_dosg1_port;
    uint16_t pump_dosg2_port;
    uint16_t pump_lift1_port;
    uint16_t pump_lift2_port;
    uint16_t unused1[48-14];
    //  曝气泵    
    uint32_t pump_aera1_time;
    uint32_t pump_aera2_time;
    uint32_t pump_aera3_time;
    uint16_t unused2[16-6];
    //  潜污泵
    uint16_t pump_subm_on_time;
    uint16_t pump_subm_off_time;
    uint16_t pump_subm_cycle_time;
    uint16_t unused3[16-3];
    //  回流泵
    uint16_t pump_rflx_on_time;
    uint16_t pump_rflx_off_time;
    uint16_t pump_rflx_cycle_time;
    uint16_t unused4[16-3];
    //  出水泵
    uint16_t pump_watr_cycle_time;
    uint16_t unused5[16-1];
    //  加药泵
    uint16_t pump_dosg_delay_time;
    uint16_t unused6[16-1];
    //  提升泵
    uint16_t pump_lift_cycle_time;
    uint16_t unused7[16-1];
} s_A2O_technology_argv_t;

typedef union  u_A2O_technology_argv
{
    s_A2O_technology_argv_t member;
    uint16_t  registers[sizeof(s_A2O_technology_argv_t)/2];
} u_A2O_technology_argv_t;


typedef struct s_MBR_technology_argv
{
    uint16_t pump_aera1_port;
    uint16_t pump_aera2_port;
    uint16_t pump_subm1_port;
    uint16_t pump_subm2_port;
    uint16_t pump_wmbr1_port;
    uint16_t pump_wmbr2_port;
    uint16_t pump_rflx1_port;
    uint16_t pump_rflx2_port;
    uint16_t pump_watr1_port;
    uint16_t pump_watr2_port;
    uint16_t pump_dosg1_port;
    uint16_t pump_dosg2_port;
    uint16_t pump_lift1_port;
    uint16_t pump_lift2_port;
    uint16_t unused1[48-14];
    
    //  出水泵
    uint16_t pump_watr_cycle_time;
    uint16_t pump_watr_delay_time;
    uint16_t unused5[16-2];
    //  加药泵
    uint16_t pump_dosg_delay_time;
    uint16_t unused6[16-1];
    //  提升泵
    uint16_t pump_lift_cycle_time;
    uint16_t unused7[16-1];
    //  洗膜泵
    uint16_t pump_wmbr_on_time;
    uint16_t pump_wmbr_off_time;
    uint16_t pump_wmbr_cycle_time;
    uint16_t unused8[16-3];
    //  曝气泵    
    uint32_t pump_aera1_time;
    uint32_t pump_aera2_time;
    uint32_t pump_aera3_time;
    uint16_t unused2[16-6];
    //  潜污泵
    uint16_t pump_subm_on_time;
    uint16_t pump_subm_off_time;
    uint16_t pump_subm_cycle_time;
    uint16_t unused3[16-3];
    //  回流泵
    uint16_t pump_rflx_on_time;
    uint16_t pump_rflx_off_time;
    uint16_t pump_rflx_cycle_time;
    uint16_t unused4[16-3];
} s_MBR_technology_argv_t;

typedef union  u_MBR_technology_argv
{
    s_MBR_technology_argv_t member;
    uint16_t  registers[sizeof(s_MBR_technology_argv_t)/2];
} u_MBR_technology_argv_t;


typedef struct s_SBR_technology_argv
{
    uint16_t pump_aera1_port;
    uint16_t pump_aera2_port;
    uint16_t pump_aera3_port;
    uint16_t pump_subm1_port;
    uint16_t pump_subm2_port;
    uint16_t pump_subm3_port;
    uint16_t pump_watr1_port;
    uint16_t pump_watr2_port;
    uint16_t pump_dosg1_port;
    uint16_t pump_dosg2_port;
    uint16_t pump_lift1_port;
    uint16_t pump_lift2_port;
    uint16_t unused1[48-16];
    
    //  当前阶段
    uint16_t current_phase;
    uint16_t precipitate_time;
    uint16_t unused8[16-2];
    //  曝气泵    
    uint16_t pump_aera_on_time;
    uint16_t unused2[16-1];
    //  潜污泵
    uint16_t pump_subm_cycle_time;
    uint16_t unused3[16-1];
    //  出水泵
    uint16_t pump_watr_cycle_time;
    uint16_t unused5[16-1];
    //  加药泵
    uint16_t pump_dosg_delay_time;
    uint16_t unused6[16-1];
    //  提升泵
    uint16_t pump_lift_cycle_time;
    uint16_t unused7[16-1];
} s_SBR_technology_argv_t;

typedef union  u_SBR_technology_argv
{
    s_SBR_technology_argv_t member;
    uint16_t  registers[sizeof(s_SBR_technology_argv_t)/2];
} u_SBR_technology_argv_t;
#endif

#if 1  //  传感器寄存器结构
//  传感器控制结构
typedef struct s_sensor_ctrl
{
    float max;  //  最大值
    float min;  //  最小值
    float accuracy;  //  精度
    uint16_t port;   //  端口
    uint16_t unused;  //  未使用
    
    float calib1;  //  标定1
    float sampling1;  //  采样值1
    float calib2;  //  标定2
    float sampling2;  //  采样2
} s_sensor_ctrl_t;

//  传感器数据
typedef struct s_sensors
{
    float t_value;  //  温度采样值
    float ph_value;  //  ph采样值
    float cod_value;  //  COD采样值
    float orp_value;  //  ORP采样值
    float nh3_value;  //  NH3采样值
    float DO_value;
    float ss_value;
    float p_value;
    float flux_value;
    float flux_total;
    float unused1[22]; //  占位
     
    
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
    uint16_t reg[sizeof(s_sensors_t)/2];
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

#define GetLiquidLevel(channel)    (yw_input & (1<<((channel>0?(channel-1):0))))

#endif

