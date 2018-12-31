#include "pump.h"
#include "stdlib.h"
#include "string.h"
#include "chip_communication.h"
#include "sys_config.h"
#include "my_time.h"
#include "ucos_ii.h"



static uint16_t tech_relay_out;  //  工艺应用对泵的输出


#define PUMP_STATE_NOINIT             0
#define PUMP_STATE_RUNNING            1
#define PUMP_STATE_STOPPING           2
#define PUMP_STATE_OPENCIRCUIT        3
#define PUMP_STATE_SHORTCIRCUIT       4
#define PUMP_STATE_NODEFINED          15



typedef struct s_pump_ctl
{
    uint8_t  sw;         //  控制开关, 0-禁能, 1-使能
    uint8_t  active;     //  控制执行状态, 0-off, 1-short
    uint8_t  opratinon;  //  操作控制状态, 0-off, 1-short
    uint8_t  status;     //  状态, 0-未初始化, 1-打开, 2-关闭, 3-开路, 4-短路, 15-其他未定义状态
    uint32_t timer;      //  最后一次状态转换的时间
} s_pump_ctl_t;

s_pump_ctl_t  pump[16];

void UpdateRelayCtl(void)
{
    uint16_t temp;//relay_ctl ^ relay_out;
    uint16_t ctrl = 0;
    uint16_t change = 0;
    static uint16_t manual = 0;
    uint16_t manual_change = manual ^ pump_manual_ctrl;
    uint8_t i;
    uint32_t cpu_sr;
    temp = tech_relay_out;
    manual = pump_manual_ctrl;
    for (i=0; i<14; i++)
    {
        if (manual_change & (1 << i))  //  优先处理手动控制
        {
            pump[i].opratinon = ((manual>>i) & 1);
            if (pump[i].active != pump[i].opratinon)
            {
                pump[i].timer = clock();
                pump[i].active = pump[i].opratinon;
                change |= (1<<i); 
            }
        }
        else if (sys_config_ram.coil_g1.ctrl.pump_auto == 1)
        {
            pump[i].opratinon = ((temp>>i) & 1);
            if (pump[i].active != pump[i].opratinon)
            {
                //  一旦继电器打开，则必须打开到最小时间
                if (((pump[i].timer == 0) || (ComputeTickTime(pump[i].timer)/OS_TICKS_PER_SEC  
                                          >= sys_config_ram.reg_group_1.pump_open_time_min)))
                {
                    sys_config_ram.coil_g1.ctrl.communication = 1;  //  控制泵有变化，发送一条数据
                    pump[i].timer = clock();
                    pump[i].active = pump[i].opratinon;
                    change |= (1<<i); 
                }
            }  
        }
         
        if (pump[i].active)  ctrl |= (1<<i);
    }    
    PumpCtrl(change & ctrl, 1);     //  开
    PumpCtrl(change & (~ctrl), 0);  //  关
    OS_ENTER_CRITICAL();
    //  组合手动未处理的状态和当前控制状态给手动控制变量, 可以在屏上显示当前状态
    manual_change = manual ^ pump_manual_ctrl;
    pump_manual_ctrl = (pump_manual_ctrl & manual_change) | (ctrl & (~manual_change));  
    manual = ctrl;
    //manual_change = temp ^ relay_out;
    //relay_out = (relay_out & manual_change) | (ctrl & (~manual_change));  
    OS_EXIT_CRITICAL();
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
    if (ChipWriteFrame(0, 0, MAX_PUMP_CHANNEL, relay_ctl_data) != 0)
    {
        return ChipWriteFrame(0, 0, MAX_PUMP_CHANNEL, relay_ctl_data);
    }
    return 0;
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

//  
e_pump_state_t GetPumpState(uint32_t channel)
{
    if ((channel == 0) || (channel > MAX_PUMP_CHANNEL))
    {
        return PUMP_ST_NOINIT;
    }
    return pump[channel-1].active?PUMP_ST_ON:PUMP_ST_OFF;
}


int8_t PumpCtrlByTech(uint8_t channel, uint8_t open)
{
    if ((channel == 0) || (channel > MAX_PUMP_CHANNEL))
    {
        return -1;
    }
    channel -= 1;
    if (open)    tech_relay_out |= (1 << channel);    //  开启
    else         tech_relay_out &= ~(1 << channel);   //  关闭
    return 0;
}


//  成对的泵控制, 实现主泵坏了，开启从泵的功能
//  返回 0-所有泵都坏了
//       >0--已经打开的泵的通道
int8_t PairOfPumpCtrlByTech(uint8_t primary, uint8_t standby, uint8_t both)
{
    if (both == 0)  //  同时关闭
    {
        PumpCtrlByTech(primary, 0);
        PumpCtrlByTech(standby, 0);
        return 0;
    }
    if (IsPumpFault(GetPumpState(primary)))
    {
        if (IsPumpFault(GetPumpState(standby)))
        {
            return -1;
        }
        else
        {
            PumpCtrlByTech(standby, 1);
            PumpCtrlByTech(primary, 0);
            return standby;
        }
    }
    else
    {
        PumpCtrlByTech(primary, 1);
        if (both == 2)
        {
            if (IsPumpFault(GetPumpState(standby)))
            {
                PumpCtrlByTech(standby, 0);
            }
            else
            {
                PumpCtrlByTech(standby, 1);
            }
        }
        else
        {
            PumpCtrlByTech(standby, 1);
        }
        return primary;
    }
}

int8_t ClearPump(void)
{
    tech_relay_out = 0;
    pump_manual_ctrl = 0;
    return 1;
}





