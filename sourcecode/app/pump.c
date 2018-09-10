#include "pump.h"
#include "stdlib.h"
#include "string.h"
#include "chip_communication.h"
#include "sys_config.h"

#define PUMP_STATE_NOINIT             0
#define PUMP_STATE_RUNNING            1
#define PUMP_STATE_STOPPING           2
#define PUMP_STATE_OPENCIRCUIT        3
#define PUMP_STATE_SHORTCIRCUIT       4
#define PUMP_STATE_NODEFINED          15



typedef struct s_pump_ctl
{
    uint8_t  sw;         //  控制开关, 0-禁能, 1-使能
    uint8_t  active;     //  控制执行状态, 0-run, 1-off
    uint8_t  opratinon;  //  操作控制状态, 0-run, 1-off
    uint8_t  status;     //  状态, 0-未初始化, 1-打开, 2-关闭, 3-开路, 4-短路, 15-其他未定义状态
    uint32_t timer;      //  当前状态的时间(定时器)
} s_pump_ctl_t;

s_pump_ctl_t  pump[16];

int8_t SinglePumpCtrl(uint8_t channel, uint8_t open)
{
    uint8_t  relay_ctl_data[MAX_PUMP_CHANNEL];
    if ((sys_config_ram.coil_g1.ctrl.manual) || 
        (channel == 0) || (channel > MAX_PUMP_CHANNEL))
    {
        return -1;
    }
    memset(relay_ctl_data, 0, sizeof(relay_ctl_data));
    if (open)    open = 1;    //  开启
    else         open = 2;    //  关闭
    relay_ctl_data[channel-1] = open;
    return ChipWriteFrame(0, 0, MAX_PUMP_CHANNEL, relay_ctl_data);
}

//  可以同时控制多个通道
int8_t PumpCtrl(uint32_t channel, uint8_t open)
{
    uint8_t  relay_ctl_data[MAX_PUMP_CHANNEL];
    uint8_t i;
    memset(relay_ctl_data, 0, sizeof(relay_ctl_data));
    if (open)    open = 1;    //  开启
    else         open = 2;    //  关闭
    for (i=0; i<MAX_PUMP_CHANNEL; i++)
    {
        if (channel & (1<<i))
        {
            relay_ctl_data[i] = open;
        }
    }
    return ChipWriteFrame(0, 0, MAX_PUMP_CHANNEL, relay_ctl_data);
}


//  只能读单个通道的状态
int8_t GetPumpCtrlState(uint32_t channel)
{
    uint8_t i;
    int8_t status;
    for (i=0;i<MAX_PUMP_CHANNEL;i++)
    {
        if (channel & (1<<i))
        {
            break;
        }
    }
    if (i >= MAX_PUMP_CHANNEL)
    {
        return -1;
    }
    if (ChipReadFrame(1, CH2_RELAY_ST_ADDR+i, 1, &status) < 0)
    {
        return -1;
    }
    return status;
}

