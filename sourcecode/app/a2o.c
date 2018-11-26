﻿/*  A2O.c
 *  实现A2O(商达)工艺的处理
 *  设备1、曝气泵
        3台（端口3-5），三台泵分别单独时间控制，默认0：00~23:00开启，23:01~23:59停止，时间可调。
        三个泵单独设置，可以按照某一个小时工作。
    设备2、潜污泵(原提升泵)  
        周期A+B的时间始终不变。如果在A区间内停止，停止时间一直要到周期结束。  
        两个泵相互切换工作，切换时间可配置（A+B的倍数）。
        如果1号泵坏， 只使用2号泵，3号泵只受L5控制  1,2 号泵是进循环系统， 3号泵直接排出系统。
        3台（端口6-8），一用二备，液位L4高液位（调节池液位）启动1台主泵或备用泵1，液位L4低液位停止。
        同时可时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。主泵和备用泵1每２４ｈ自动切换运行，
        如主泵故障，则用自动启备用泵1，同理，备用泵故障，自动切换到主泵运行。液位L5高液位（调节池超高液位），
        启动备用泵2，L5低液位，备用泵2停止。
    设备3、回流泵（切换时间可调）
        2台（端口9-10），一用一备，时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
        2台水泵２４ｈ自动切换运行。如主泵故障，则用启用备用泵，同理，备用泵故障，自动切换到主泵运行。
    设备4、出水泵（切换时间可调）
        2台（端口11-12），一用一备，液位L6（沉淀池液位）高液位启动主泵，液位L6低液位，主泵停止。
        2台水泵２４ｈ自动切换运行。如主泵故障，则启用用备用泵，同理，备用泵故障，自动切换到主泵运行。
    设备5、加药泵（加药的时间和潜污泵开启时间一致，整个过程延迟A分钟）（L7和L8必须要同时达到高液位才动作）周期和潜污泵保持一致。
        2台（端口13-14），分别加PAC及次氯酸钠。和潜污泵联动并受液位L7和L8（加药桶液位）控制，
        潜污泵开启后A（可调，A默认值为10）分钟且L7和L8为高液位，两台加药泵同时启动，
        若L7和L8中有一个为低液位，则加药泵停止。若L7和L8为高液位，潜污泵停止，则加药泵延迟A分钟停止。
    设备6、提升（非原提升泵）周期可调
        2台（端口1-2），一用一备，液位L1（提升机液位）高液位且L3（调节池液位）低液位启动主泵，液位L1低液位，主泵停止，
        2台水泵２４ｈ自动切换运行。如主泵故障，则启用备用泵，同理，备用泵故障，自动切换到主泵运行。液位L2高液位且L3低液位同时启动主泵和备用泵。

 */
#include "a2o.h"
#include "pump.h"
#include "mod_time.h"
#include "sys_config.h"

typedef struct s_subm_open
{
    struct s_subm_open * next;
    uint32_t start;  //  单位S
    uint32_t time : 28;  //  单位S
    uint32_t type : 4;   // 0--结束时间; 1--起始时间; 
                         // 2--起始持续时间; 3--结束持续时间; 4--时间标志
} s_subm_open_t;

s_subm_open_t subm_open[60];
s_subm_open_t *subm_open_head = 0;
s_subm_open_t *subm_open_free_head = subm_open;
s_subm_open_t subm_open_current;

void AddSubmOpenTimestamp(s_subm_open_t *curr)
{
    int32_t i;
    s_subm_open_t * temp;
    curr->next = 0;
    if (subm_open_head == 0)
    {
        subm_open[0] = *curr;
        subm_open_head = subm_open;
        subm_open_free_head = &subm_open[1];
        temp = subm_open_free_head;
        for (i=2; i<(sizeof(subm_open)/sizeof(s_subm_open_t)); i++)
        {
            temp->next = &subm_open[i];
            temp = temp->next;
        }
        temp->next = 0;
    }
    else
    {
        temp = subm_open_head;
        while (temp->next)  temp = temp->next;  //  找最后一个数据
        if (subm_open_free_head)
        {
            temp->next = subm_open_free_head;  //  获取新的空间
            subm_open_free_head = subm_open_free_head->next;  //  更新空闲头
            *temp->next = *curr;  //  赋值            
        }
    }
}


s_subm_open_t *GetSubmOpenTimestamp(void)
{
    s_subm_open_t * temp = 0, *next;
    if (subm_open_head)
    {
        
        if (subm_open_free_head > subm_open_head)
        {
            temp = subm_open_head->next;
            subm_open_head->next = subm_open_free_head;
            subm_open_free_head = subm_open_head;
            subm_open_head = temp;
            temp = subm_open_free_head;
        }
        else
        {
            next = subm_open_free_head;
            while (next->next && (next->next < subm_open_head))  next = next->next;
            temp = next->next;
            next->next = subm_open_head;
            next = subm_open_head->next;
            subm_open_head->next = temp;
            temp = subm_open_head;
            subm_open_head = next;
        }
    }
    return temp;
}

/*
液位说明：
1、L1、L2分别为提升井中的高液位和超高液位
2、L3、L4为调节池高液位、超高液位
3、L5为出水井液位
4、L6和L7为加药桶液位
5、L8备用液位
 */
#define A2O_LLP_LIFTING_HIGH           1
#define A2O_LLP_LIFTING_ULTRAHIGH      2
#define A2O_LLP_SUBM_HIGH              3
#define A2O_LLP_SUBM_ULTRAHIGH         4
#define A2O_LLP_WATER_HIGH             5
#define A2O_LLP_DOSING1_HIGH           6
#define A2O_LLP_DOSING2_HIGH           7
#define A2O_LLP_STANDBY                8


enum e_a2o_pump_state
{
    A2O_PUMP_ST_INIT,
    A2O_PUMP_ST_PRIMARY_A,
    A2O_PUMP_ST_PRIMARY_B,
    A2O_PUMP_ST_MINOR_A,
    A2O_PUMP_ST_MINOR_B
} ;

extern void PumpCtl(uint8_t channel, uint8_t open);

//  曝气泵处理
int8_t GetPumpState(uint32_t channel)
{
    if ((channel == 0) || (channel > MAX_PUMP_CHANNEL))
    {
        return 0;
    }
    return (relay_out & (1 << (channel-1)))?1:0;
}

#define IsAerationPumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_aera1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_aera2_port) || \
             GetPumpState(a2o_technology_argv.member.pump_aera3_port))

//  曝气泵控制
//设备1、曝气泵
//3台（端口3-5），三台泵分别单独时间控制，默认0：00~23:00开启，23:01~23:59停止，时间可调。
void A2OAerationPumpCtrl(void)
 {
    uint8_t state = 0;
    uint32_t now_ts = time(0);
    struct tm now = *localtime(&now_ts);
    if (a2o_technology_argv.member.pump_aera1_port)  //  泵是否使能, 配置了端口表示使能
    {
        //  判断对应时间是否需要开启
        if ((1<< now.tm_hour) & a2o_technology_argv.member.pump_aera1_time)
        {
            //  当前小时需要开启
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera1_port, 1);
            state = 1;
        }
        else
        {
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera1_port, 0);
        }
    }
    
    if (a2o_technology_argv.member.pump_aera2_port)  //  泵是否使能, 配置了端口表示使能
    {
        //  判断对应时间是否需要开启
        if ((1<< now.tm_hour) & a2o_technology_argv.member.pump_aera2_time)
        {
            //  当前小时需要开启
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera2_port, 1);
            state = 1;
        }
        else
        {
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera2_port, 0);
        }
    }
    
    
    if (a2o_technology_argv.member.pump_aera3_port)  //  泵是否使能, 配置了端口表示使能
    {
        //  判断对应时间是否需要开启
        if ((1<< now.tm_hour) & a2o_technology_argv.member.pump_aera3_time)
        {
            //  当前小时需要开启
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera3_port, 1);
            state = 1;
        }
        else
        {
            SinglePumpCtrl(a2o_technology_argv.member.pump_aera3_port, 0);
        }
    }
    if (IsAerationPumpOpen())
    {
        pump_state.pump_st.pump_aera = 1;
    }
    else
    {
        pump_state.pump_st.pump_aera = 0;
    }
}

/**  @breif  设备2、潜污泵
 **  @notice 3台（端口6-8），一用二备，液位L4高液位（调节池液位）启动1台主泵或备用泵1，液位L4低液位停止。
 **          同时可时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
 **          主泵和备用泵1每２４ｈ自动切换运行，如主泵故障，则用自动启备用泵1，
 **          同理，备用泵故障，自动切换到主泵运行。
 **          液位L5高液位（调节池超高液位），启动备用泵2，L5低液位，备用泵2停止。
 **/

void A2OSubmCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == a2o_technology_argv.member.pump_subm1_port)
    {
        standby = a2o_technology_argv.member.pump_subm2_port;
    }
    else if (channel == a2o_technology_argv.member.pump_subm2_port)
    {
        standby = a2o_technology_argv.member.pump_subm1_port;
    }
    else  //  参数错误
    {
        return;
    }
    if (GetLiquidLevel(A2O_LLP_SUBM_ULTRAHIGH) && value)
    {
        SinglePumpCtrl(standby, 1);
        SinglePumpCtrl(channel, 1);
    }
    else if (GetLiquidLevel(A2O_LLP_SUBM_HIGH) && value)
    {
        SinglePumpCtrl(standby, 0);
        SinglePumpCtrl(channel, 1);
    }
    else
    {
        SinglePumpCtrl(channel, 0);
        SinglePumpCtrl(standby, 0);
    }
}

#define IsSubmersibleSewagePumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_subm1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_subm2_port)\
            )

void A2OSubmersibleSewagePumpCtrl(void)
{
    uint8_t pump_st = 0;
    static uint8_t state = A2O_PUMP_ST_INIT;
    static uint32_t time_change = 0;
    static uint32_t time_cycle = 0;
    static uint32_t open_time;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (state)
    {
        case A2O_PUMP_ST_INIT:
            time_change = time_now;
            time_cycle = time_now;
            state = A2O_PUMP_ST_PRIMARY_A;
            A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);
            //open_time = 0;
            pump_st = 1;
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_on_time)  //  开启时间超时
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 0);  //  关闭泵
                time_change = time_now;
                state = A2O_PUMP_ST_PRIMARY_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_subm1_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);
                state = A2O_PUMP_ST_MINOR_A;
                pump_st = 1;
            }
            A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);
            break;
        case A2O_PUMP_ST_PRIMARY_B:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_off_time)  //  关闭时间超时
            {
                //open_time = 0;
                time_change = time_now;
                if ((time_now - time_cycle) > a2o_technology_argv.member.pump_subm_cycle_time)  //  周期切换时间超时
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);
                    state = A2O_PUMP_ST_MINOR_A;
                    time_cycle = time_now;
                }
                else
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1); 
                    state = A2O_PUMP_ST_PRIMARY_A;
                }
                pump_st = 1;
            }
            break;
        case A2O_PUMP_ST_MINOR_A:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_on_time)  //  开启时间超时
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 0);  //  关闭泵
                time_change = time_now;
                state = A2O_PUMP_ST_MINOR_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_subm2_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);
                state = A2O_PUMP_ST_MINOR_A;
                pump_st = 1;
            }
            A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);
            break;
        case A2O_PUMP_ST_MINOR_B:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_off_time)  //  关闭时间超时
            {
                //open_time = 0;
                if ((time_now - time_cycle) > a2o_technology_argv.member.pump_subm_cycle_time)  //  周期切换时间超时
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);  //  开启主泵
                    state = A2O_PUMP_ST_PRIMARY_A;
                    time_cycle = time_now;
                }
                else
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);  //  开启备用泵
                    state = A2O_PUMP_ST_MINOR_A;
                }
                pump_st = 1;
                time_change = time_now;
            }
            break;
        default:
            state = A2O_PUMP_ST_INIT;
            break;
    }
    if (IsSubmersibleSewagePumpOpen())
    {
        pump_state.pump_st.pump_subm = 1;
        if (open_time == 0)   open_time = time(0);
        if (subm_open_current.start == 0)
        {
            if ((state == A2O_PUMP_ST_PRIMARY_A) || (state == A2O_PUMP_ST_MINOR_A))
            {
                subm_open_current.start = time(0);
                subm_open_current.time = 0;
            }
        }
    }
    else
    {
        pump_state.pump_st.pump_subm = 0;
        if (open_time) 
        {
            subm_open_current.time += time(0) - open_time;
            open_time = 0;
        }
        if (subm_open_current.start)
        {
            if ((state != A2O_PUMP_ST_PRIMARY_A) && (state != A2O_PUMP_ST_MINOR_A))
            {
                subm_open_current.type = 2;
                AddSubmOpenTimestamp(&subm_open_current);
                subm_open_current.start = 0;
            }
        }
    }
}

/**  @breif  设备3、回流泵
 **  @notice 2台（端口9-10），一用一备，时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则用启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **/
void A2OReflexCtrl(uint32_t channel, uint8_t value)
{
    if (value)
    {
        SinglePumpCtrl(channel, 1);
        pump_state.pump_st.pump_rflx = 1;
    }
    else
    {
        SinglePumpCtrl(channel, 0);
    }
}

#define IsReflexPumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_rflx1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_rflx2_port))

void A2OReflexPumpCtrl(void)
{   
    uint8_t pump_st = 0;
    static uint8_t refx_state = A2O_PUMP_ST_INIT;
    static uint32_t refx_time_change = 0;
    static uint32_t refx_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (refx_state)
    {
        case A2O_PUMP_ST_INIT:
            refx_time_change = time_now;
            refx_time_cycle = time_now;
            refx_state = A2O_PUMP_ST_PRIMARY_A;
            A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 1);
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - refx_time_change) > a2o_technology_argv.member.pump_rflx_on_time)  //  开启时间超时
            {
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 0);  //  关闭主泵
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 0);  //  关闭备用泵
                refx_time_change = time_now;
                refx_state = A2O_PUMP_ST_PRIMARY_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_rflx1_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 0);
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 1);
                refx_state = A2O_PUMP_ST_MINOR_A;
            }
            break;
        case A2O_PUMP_ST_PRIMARY_B:
            if ((time_now - refx_time_change) > a2o_technology_argv.member.pump_rflx_off_time)  //  关闭时间超时
            {
                refx_time_change = time_now;
                if ((time_now - refx_time_cycle) > a2o_technology_argv.member.pump_rflx_cycle_time)  //  周期切换时间超时
                {
                    A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 1);  //  开启备用泵
                    refx_state = A2O_PUMP_ST_MINOR_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 1);  //  开启主泵
                    refx_state = A2O_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case A2O_PUMP_ST_MINOR_A:
            if ((time_now - refx_time_change) > a2o_technology_argv.member.pump_rflx_on_time)  //  开启时间超时
            {
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 0);  //  关闭主泵
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 0);  //  关闭备用泵
                refx_time_change = time_now;
                refx_state = A2O_PUMP_ST_MINOR_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_rflx2_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 0);
                A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 1);
                refx_state = A2O_PUMP_ST_MINOR_A;
            }
            break;
        case A2O_PUMP_ST_MINOR_B:
            if ((time_now - refx_time_change) > a2o_technology_argv.member.pump_rflx_off_time)  //  关闭时间超时
            {
                if ((time_now - refx_time_cycle) > a2o_technology_argv.member.pump_rflx_cycle_time)  //  周期切换时间超时
                {
                    A2OReflexCtrl(a2o_technology_argv.member.pump_rflx1_port, 1);  //  开启主泵
                    refx_state = A2O_PUMP_ST_PRIMARY_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    A2OReflexCtrl(a2o_technology_argv.member.pump_rflx2_port, 1);  //  开启备用泵
                    refx_state = A2O_PUMP_ST_MINOR_A;
                }
                refx_time_change = time_now;
            }
            break;
        default:
            refx_state = A2O_PUMP_ST_INIT;
            break;
    }
    if (IsReflexPumpOpen())
    {
        pump_state.pump_st.pump_rflx = 1;
    }
    else
    {
        pump_state.pump_st.pump_rflx = 0;
    }
}

/**  @breif  设备4、出水泵
 **  @notice 2台（端口11-12），一用一备，液位L6（沉淀池液位）高液位启动主泵，液位L6低液位，主泵停止。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用用备用泵，同理，备用泵故障，自动切换到主泵运行。

 **/

void A2OWaterCtrl(uint32_t channel, uint8_t value)
{
    if ((GetLiquidLevel(A2O_LLP_WATER_HIGH)) && value)
    {
        SinglePumpCtrl(channel, 1);
        //pump_state.pump_st.pump_watr = 1;
    }
    else
    {
        SinglePumpCtrl(channel, 0);
    }
    if ((GetLiquidLevel(A2O_LLP_WATER_HIGH)) == 0)
    {
        //pump_state.pump_st.pump_watr = 0;
    }
}

#define IsWaterPumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_watr1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_watr2_port))
void A2OWaterPumpCtrl(void)
{
//    uint8_t pump_st = 0;
    static uint8_t watr_state = A2O_PUMP_ST_INIT;
    static uint32_t watr_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (watr_state)
    {
        case A2O_PUMP_ST_INIT:
            watr_time_cycle = time_now;
            watr_state = A2O_PUMP_ST_PRIMARY_A;
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if (GetPumpState(a2o_technology_argv.member.pump_watr1_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                watr_state = A2O_PUMP_ST_MINOR_A;
                break;
            }
            else if ((time_now - watr_time_cycle) > a2o_technology_argv.member.pump_watr_cycle_time)  //  周期切换时间超时
            {
                watr_state = A2O_PUMP_ST_MINOR_A;
                watr_time_cycle = time_now;
            }
            A2OWaterCtrl(a2o_technology_argv.member.pump_watr1_port, 1);
            A2OWaterCtrl(a2o_technology_argv.member.pump_watr2_port, 0);
            break;
        
        case A2O_PUMP_ST_MINOR_A:
            if (GetPumpState(a2o_technology_argv.member.pump_watr2_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                watr_state = A2O_PUMP_ST_PRIMARY_A;
                break;
            }
            else if ((time_now - watr_time_cycle) > a2o_technology_argv.member.pump_watr_cycle_time)  //  周期切换时间超时
            {
                watr_state = A2O_PUMP_ST_PRIMARY_A;
                watr_time_cycle = time_now;
            }
            A2OWaterCtrl(a2o_technology_argv.member.pump_watr1_port, 0);
            A2OWaterCtrl(a2o_technology_argv.member.pump_watr2_port, 1);
            break;
        default:
            watr_state = A2O_PUMP_ST_INIT;
            break;
    }
    if (IsWaterPumpOpen())
    {
        pump_state.pump_st.pump_watr = 1;
    }
    else
    {
        pump_state.pump_st.pump_watr = 0;
    }
}
/**  @breif  设备5、加药泵
 **  @notice 2台（端口13-14），分别加PAC及次氯酸钠。
 **          和潜污泵联动并受液位L7和L8（加药桶液位）控制，潜污泵开启后A（可调，A默认值为10）分钟且L7和L8为高液位，
 **          两台加药泵同时启动，若L7和L8中有一个为低液位，则加药泵停止。若L7和L8为高液位，潜污泵停止，则加药泵延迟A分钟停止。
 **/

#define IsDosingPumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_dosg1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_dosg2_port))

void A2ODosingPumpCtrl(void)
{
    uint8_t pump_st = 0;
    static uint8_t dosg_state = A2O_PUMP_ST_INIT;
    static uint32_t watr_time_delay = 0;
    static s_subm_open_t subm_open_temp;
    s_subm_open_t *subm_open_ptr;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (dosg_state)
    {
        case A2O_PUMP_ST_INIT:
            if ((subm_open_ptr = GetSubmOpenTimestamp()) != 0)
            {
                subm_open_temp = *subm_open_ptr;
                //watr_time_delay = subm_open_temp.start;
                dosg_state = A2O_PUMP_ST_PRIMARY_A;
            }
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - subm_open_temp.start) > ((a2o_technology_argv.member.pump_dosg_delay_time * 60) + subm_open_temp.time))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 0);
                dosg_state = A2O_PUMP_ST_INIT;
            }
            else if ((time_now - subm_open_temp.start) > (a2o_technology_argv.member.pump_dosg_delay_time * 60))
            {
                if ((GetLiquidLevel(A2O_LLP_DOSING1_HIGH)) && (GetLiquidLevel(A2O_LLP_DOSING2_HIGH)))
                {
                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 1);
                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 1);
                    pump_st = 1;
                }  
                else
                {
                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 0);
                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 0);
                    pump_st = 2;
                }    
            }
            break;
//        case A2O_PUMP_ST_PRIMARY_B:
//            if ((time_now - watr_time_delay) > (a2o_technology_argv.member.pump_dosg_delay_time * 60))
//            {  
//                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 0);
//                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 0);
//                pump_st = 2;
//                dosg_state = A2O_PUMP_ST_INIT;
//            }
//            else
//            {
//                if ((GetLiquidLevel(A2O_LLP_DOSING1_HIGH)) && (GetLiquidLevel(A2O_LLP_DOSING2_HIGH)))
//                {
//                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 1);
//                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 1);
//                    pump_st = 1;
//                }  
//                else
//                {
//                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 0);
//                    SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 0);
//                    pump_st = 2;
//                }  
//            }
//            break;
        default:
            
            break;
    }
    if (IsDosingPumpOpen())
    {
        pump_state.pump_st.pump_dosg = 1;
    }
    else
    {
        pump_state.pump_st.pump_dosg = 0;
    }
}

/**  @breif  设备6、提升泵
 **  @notice 2台（端口1-2），一用一备，液位L1（提升机液位）高液位且L3（调节池液位）低液位启动主泵，液位L1低液位，主泵停止，
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **          液位L2高液位且L3低液位同时启动主泵和备用泵。        
 **/

#define IsLiftingPumpOpen()   \
            (GetPumpState(a2o_technology_argv.member.pump_lift1_port) || \
             GetPumpState(a2o_technology_argv.member.pump_lift2_port))

void A2OLiftingPumpCtrl(void)
{
    uint8_t pump_st = 0;    
    static uint8_t lift_state = A2O_PUMP_ST_INIT;
    static uint32_t lift_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (lift_state)
    {
        case A2O_PUMP_ST_INIT:
            lift_time_cycle = time_now;
            lift_state = A2O_PUMP_ST_PRIMARY_A;
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - lift_time_cycle) > a2o_technology_argv.member.pump_lift_cycle_time)  //  周期切换时间超时
            {
                lift_state = A2O_PUMP_ST_MINOR_A;
                lift_time_cycle = time_now;
            }
            if ((GetLiquidLevel(A2O_LLP_SUBM_HIGH)) || (GetLiquidLevel(A2O_LLP_LIFTING_HIGH) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);                
            }
            else if (GetLiquidLevel(A2O_LLP_LIFTING_ULTRAHIGH))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 1);
            }
            else 
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);
            }
            break;
            
        case A2O_PUMP_ST_MINOR_A:
            if ((time_now - lift_time_cycle) > a2o_technology_argv.member.pump_lift_cycle_time)  //  周期切换时间超时
            {
                lift_state = A2O_PUMP_ST_PRIMARY_A;
                lift_time_cycle = time_now;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_lift2_port - 1) == PUMP_ST_ERROR)
            {
            }
            if ((GetLiquidLevel(A2O_LLP_SUBM_HIGH)) || (GetLiquidLevel(A2O_LLP_LIFTING_HIGH) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);                
            }
            else if (GetLiquidLevel(A2O_LLP_LIFTING_ULTRAHIGH))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 1);
            }
            else 
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 1);
            }
            break;
        default:
            lift_state = A2O_PUMP_ST_INIT;
            break;
    }
    if (IsLiftingPumpOpen())
    {
        pump_state.pump_st.pump_lift = 1;
    }
    else
    {
        pump_state.pump_st.pump_lift = 0;
    }
}


void A2OStandbyPumpCtrl(void)
{
    if (GetLiquidLevel(A2O_LLP_STANDBY))
    {
        SinglePumpCtrl(a2o_technology_argv.member.pump_standby_port, 1);
    }
    else
    {
        SinglePumpCtrl(a2o_technology_argv.member.pump_standby_port, 0);
    }
}

void FlowA2O(void)
{
    A2OAerationPumpCtrl();
    A2OSubmersibleSewagePumpCtrl();
    A2OReflexPumpCtrl();
    A2OWaterPumpCtrl();
    A2ODosingPumpCtrl();
    A2OLiftingPumpCtrl();
    A2OStandbyPumpCtrl();
}


