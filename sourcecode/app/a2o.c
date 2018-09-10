/*  A2O.c
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
    return 0;
}

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
    if (state == 1)
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
    if ((yw_input & (1<<3)) && value)
    {
        SinglePumpCtrl(channel, 1);
        pump_state.pump_st.pump_subm = 1;
    }
    else
    {
        SinglePumpCtrl(channel, 0);
    }
}
void A2OSubmersibleSewagePumpCtrl(void)
{
    uint8_t pump_st = 0;
    static uint8_t state = A2O_PUMP_ST_INIT;
    static uint32_t time_change = 0;
    static uint32_t time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (state)
    {
        case A2O_PUMP_ST_INIT:
            time_change = time_now;
            time_cycle = time_now;
            state = A2O_PUMP_ST_PRIMARY_A;
            A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);
            pump_st = 1;
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_on_time)  //  开启时间超时
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 0);  //  关闭主泵
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 0);  //  关闭备用泵
                time_change = time_now;
                state = A2O_PUMP_ST_PRIMARY_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_subm1_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 0);
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);
                state = A2O_PUMP_ST_MINOR_A;
                pump_st = 1;
            }
            break;
        case A2O_PUMP_ST_PRIMARY_B:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_off_time)  //  关闭时间超时
            {
                time_change = time_now;
                if ((time_now - time_cycle) > a2o_technology_argv.member.pump_subm_cycle_time)  //  周期切换时间超时
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 1);  //  开启备用泵
                    state = A2O_PUMP_ST_MINOR_A;
                    time_cycle = time_now;
                }
                else
                {
                    A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);  //  开启主泵
                    state = A2O_PUMP_ST_PRIMARY_A;
                }
                pump_st = 1;
            }
            break;
        case A2O_PUMP_ST_MINOR_A:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_on_time)  //  开启时间超时
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 0);  //  关闭主泵
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 0);  //  关闭备用泵
                time_change = time_now;
                state = A2O_PUMP_ST_MINOR_B;
                pump_st = 2;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_subm2_port - 1) == PUMP_ST_ERROR)  //  检测泵是否坏
            {
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm2_port, 0);
                A2OSubmCtrl(a2o_technology_argv.member.pump_subm1_port, 1);
                state = A2O_PUMP_ST_MINOR_A;
                pump_st = 1;
            }
            break;
        case A2O_PUMP_ST_MINOR_B:
            if ((time_now - time_change) > a2o_technology_argv.member.pump_subm_off_time)  //  关闭时间超时
            {
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
    if (yw_input & (1<<4))
    {
        SinglePumpCtrl(a2o_technology_argv.member.pump_subm3_port, 1);
    }
    else
    {
        SinglePumpCtrl(a2o_technology_argv.member.pump_subm3_port, 0);
        if (pump_st == 2)
        {
            pump_state.pump_st.pump_subm = 0;
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
    if (pump_st == 2)
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
    if ((yw_input & (1<<5)) && value)
    {
        SinglePumpCtrl(channel, 1);
        pump_state.pump_st.pump_watr = 1;
    }
    else
    {
        SinglePumpCtrl(channel, 0);
    }
    if ((yw_input & (1<<5)) == 0)
    {
        pump_state.pump_st.pump_watr = 0;
    }
}
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
}
/**  @breif  设备5、加药泵
 **  @notice 2台（端口13-14），分别加PAC及次氯酸钠。
 **          和潜污泵联动并受液位L7和L8（加药桶液位）控制，潜污泵开启后A（可调，A默认值为10）分钟且L7和L8为高液位，
 **          两台加药泵同时启动，若L7和L8中有一个为低液位，则加药泵停止。若L7和L8为高液位，潜污泵停止，则加药泵延迟A分钟停止。
 **/

void A2ODosingPumpCtrl(void)
{
    uint8_t pump_st = 0;
    static uint8_t dosg_state = A2O_PUMP_ST_INIT;
    static uint32_t watr_time_delay = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (dosg_state)
    {
        case A2O_PUMP_ST_INIT:
            if (pump_state.pump_st.pump_subm == PUMP_ST_OPEN)
            {
                watr_time_delay = time_now;
                dosg_state = A2O_PUMP_ST_PRIMARY_A;
            }
            break;
        case A2O_PUMP_ST_PRIMARY_A:
            if ((time_now - watr_time_delay) > a2o_technology_argv.member.pump_dosg_delay_time)
            {
                if ((yw_input & (1<<6)) && (yw_input & (1<<7)))
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
            if (pump_state.pump_st.pump_subm != PUMP_ST_OPEN)
            {
                watr_time_delay = time_now;
                dosg_state = A2O_PUMP_ST_PRIMARY_B;
            }
            break;
        case A2O_PUMP_ST_PRIMARY_B:
            if ((time_now - watr_time_delay) > a2o_technology_argv.member.pump_dosg_delay_time)
            {  
                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_dosg2_port, 0);
                pump_st = 2;
                dosg_state = A2O_PUMP_ST_INIT;
            }
            else
            {
                if ((yw_input & (1<<6)) && (yw_input & (1<<7)))
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
        default:
            
            break;
    }
    if (pump_st == 1)
    {
        pump_state.pump_st.pump_dosg = 1;
    }
    else if (pump_st == 2)
    {
        pump_state.pump_st.pump_dosg = 0;
    }
}

/**  @breif  设备6、提升泵
 **  @notice 2台（端口1-2），一用一备，液位L1（提升机液位）高液位且L3（调节池液位）低液位启动主泵，液位L1低液位，主泵停止，
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **          液位L2高液位且L3低液位同时启动主泵和备用泵。        
 **/
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
            if ((time_now - lift_time_cycle) > a2o_technology_argv.member.pump_rflx_cycle_time)  //  周期切换时间超时
            {
                lift_state = A2O_PUMP_ST_MINOR_A;
                lift_time_cycle = time_now;
            }
            if ((yw_input & (1<<2)) && ((yw_input & (1<<3)) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 1);
                pump_st = 1;
            }
            else if ((yw_input & (1<<1)) && ((yw_input & (1<<3)) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);
                pump_st = 1;
            }
            else if ((yw_input & (1<<1)) == 0)
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);
                pump_st = 2;
            }
            break;
            
        case A2O_PUMP_ST_MINOR_A:
            if ((time_now - lift_time_cycle) > a2o_technology_argv.member.pump_rflx_cycle_time)  //  周期切换时间超时
            {
                lift_state = A2O_PUMP_ST_PRIMARY_A;
                lift_time_cycle = time_now;
            }
            else if (GetPumpState(a2o_technology_argv.member.pump_lift2_port - 1) == PUMP_ST_ERROR)
            {
            }
            if ((yw_input & (1<<1)) && ((yw_input & (1<<2)) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 1);
                pump_st = 1;
            }
            else if ((yw_input & (1<<0)) && ((yw_input & (1<<2)) == 0))
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 1);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);
                pump_st = 1;
            }
            else if ((yw_input & (1<<0)) == 0)
            {
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift1_port, 0);
                SinglePumpCtrl(a2o_technology_argv.member.pump_lift2_port, 0);
                pump_st = 2;
            }
            break;
        default:
            lift_state = A2O_PUMP_ST_INIT;
            break;
    }
    if (pump_st == 1)
    {
        pump_state.pump_st.pump_lift = 1;
    }
    else if (pump_st == 2)
    {
        pump_state.pump_st.pump_lift = 0;
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
}



