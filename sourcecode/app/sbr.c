#include "sbr.h"
#include "a2o.h"
#include "pump.h"
#include "my_time.h"
#include "sys_config.h"


/*
液位说明：
1、L1、L2分别为提升井中的高液位和超高液位
2、L3、L4为调节池高液位、超高液位
3、L5为SBR池液位
4、L6和L7为加药桶液位
5、L8清水池液位
 */
#define SBR_LLP_LIFTING_HIGH           1
#define SBR_LLP_LIFTING_ULTRAHIGH      2
#define SBR_LLP_SUBM_HIGH              3
#define SBR_LLP_SUBM_ULTRAHIGH         4
#define SBR_LLP_SBR_POOL_HIGH          5
#define SBR_LLP_DOSING1_HIGH           6
#define SBR_LLP_DOSING2_HIGH           7
#define SBR_LLP_CLEAN_WATER_HIGH       8

enum e_sbr_pump_state
{
    SBR_PUMP_ST_INIT,
    SBR_PUMP_ST_PRIMARY_A,
    SBR_PUMP_ST_PRIMARY_B,
    SBR_PUMP_ST_MINOR_A,
    SBR_PUMP_ST_MINOR_B
} ;



/**  @breif  设备6、提升泵
 **  @notice 2台（端口1-2），一用一备，液位L1（提升机液位）高液位且L3（调节池液位）低液位启动主泵，液位L1低液位，主泵停止，
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **          液位L2高液位且L3低液位同时启动主泵和备用泵。        
 **/

int8_t SBRLiftingCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(sbr, lift1))
    {
        standby = GetPumpPort(sbr, lift2);
    }
    else if (channel == GetPumpPort(sbr, lift2))
    {
        standby = GetPumpPort(sbr, lift1);
    }
    else  //  参数错误
    {
        return 0;
    }
    
    if ((GetLiquidLevel(SBR_LLP_SUBM_HIGH)) || (GetLiquidLevel(SBR_LLP_SUBM_ULTRAHIGH)))
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);                
    }
    else if (GetLiquidLevel(SBR_LLP_LIFTING_ULTRAHIGH))
    {
        return PairOfPumpCtrlByTech(channel, standby, 2);
    }
    else if (GetLiquidLevel(SBR_LLP_LIFTING_HIGH))
    {
        return PairOfPumpCtrlByTech(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);
    }
}

#define IsSBRLiftingPumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, lift1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, lift2)) == PUMP_ST_ON)

void SBRLiftingPumpCtrl(void)
{
    static uint8_t lift_state = SBR_PUMP_ST_INIT;
    static uint32_t lift_time_cycle = 0;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (lift_state)
    {
        case SBR_PUMP_ST_INIT:
            lift_time_cycle = time_now;
            lift_state = SBR_PUMP_ST_PRIMARY_A;
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            if ((time_now - lift_time_cycle) > GetTechArgv(sbr, pump_lift_cycle_time) * 60)  //  周期切换时间超时
            {
                lift_state = SBR_PUMP_ST_MINOR_A;
                lift_time_cycle = time_now;
            }
            else if (SBRLiftingCtrl(GetPumpPort(sbr, lift1), 1) == GetPumpPort(sbr, lift2))
            {
                lift_state = SBR_PUMP_ST_MINOR_A;
            }
            break;
            
        case SBR_PUMP_ST_MINOR_A:
            if ((time_now - lift_time_cycle) > GetTechArgv(sbr, pump_lift_cycle_time) * 60)  //  周期切换时间超时
            {
                lift_state = SBR_PUMP_ST_PRIMARY_A;
                lift_time_cycle = time_now;
            }
            else if (SBRLiftingCtrl(GetPumpPort(sbr, lift2), 1) == GetPumpPort(sbr, lift1))
            {
                lift_state = SBR_PUMP_ST_MINOR_A;
            }
            break;
        default:
            lift_state = SBR_PUMP_ST_INIT;
            break;
    }
    if (IsSBRLiftingPumpOpen())
    {
        pump_state.pump_st.pump_lift = 1;
    }
    else
    {
        pump_state.pump_st.pump_lift = 0;
    }
}



/**  @breif  设备3、回流泵
 **  @notice 2台（端口9-10），一用一备，时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
 **          L8高液位时，启动回流泵，L8低液位停止，主泵和备用泵切换运行。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则用启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **/
int8_t SBRReflexCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(sbr, rflx1))
    {
        standby = GetPumpPort(sbr, rflx2);
    }
    else if (channel == GetPumpPort(sbr, rflx2))
    {
        standby = GetPumpPort(sbr, rflx1);
    }
    else  //  参数错误
    {
        return 0;
    }
    if (GetLiquidLevel(SBR_LLP_CLEAN_WATER_HIGH) && value)
    {
        return PairOfPumpCtrlByTech(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);
    }
}

#define IsSBRReflexPumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, rflx1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, rflx2)) == PUMP_ST_ON)

void SBRReflexPumpCtrl(void)
{   
    static uint8_t refx_state = SBR_PUMP_ST_INIT;
    static uint32_t refx_time_change = 0;
    static uint32_t refx_time_cycle = 0;
    uint32_t time_now = time(0);   //  以分钟为单位
    refx_time_change = refx_time_change;
    switch (refx_state)
    {
        case SBR_PUMP_ST_INIT:
            refx_time_change = time_now;
            refx_time_cycle = time_now;
            refx_state = SBR_PUMP_ST_PRIMARY_A;
            SBRReflexCtrl(GetPumpPort(sbr, rflx1), 1);
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            //if ((time_now - refx_time_change) > GetTechArgv(sbr, pump_rflx_on_time) * 60)  //  开启时间超时
            if (0)
            {
                SBRReflexCtrl(GetPumpPort(sbr, rflx1), 0);  //  关闭主泵
                refx_time_change = time_now;
                refx_state = SBR_PUMP_ST_PRIMARY_B;
            }
            else if (SBRReflexCtrl(GetPumpPort(sbr, rflx1), 1) == GetPumpPort(sbr, rflx2))  //  检测泵是否坏
            {
                refx_state = SBR_PUMP_ST_MINOR_A;
            }
            //break;
        case SBR_PUMP_ST_PRIMARY_B:
            //if ((time_now - refx_time_change) > GetTechArgv(sbr, pump_rflx_off_time) * 60)  //  关闭时间超时
            if (1)
            {
                refx_time_change = time_now;
                if ((time_now - refx_time_cycle) > GetTechArgv(sbr, pump_rflx_cycle_time) * 60)  //  周期切换时间超时
                {
                    refx_state = SBR_PUMP_ST_MINOR_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    refx_state = SBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case SBR_PUMP_ST_MINOR_A:
            //if ((time_now - refx_time_change) > GetTechArgv(sbr, pump_rflx_on_time) * 60)  //  开启时间超时
            if (0)
            {
                SBRReflexCtrl(GetPumpPort(sbr, rflx2), 0);  //  关闭备用泵
                refx_time_change = time_now;
                refx_state = SBR_PUMP_ST_MINOR_B;
            }
            else if (SBRReflexCtrl(GetPumpPort(sbr, rflx2), 1) == GetPumpPort(sbr, rflx1))  //  检测泵是否坏
            {
                refx_state = SBR_PUMP_ST_MINOR_A;
            }
            //break;
        case SBR_PUMP_ST_MINOR_B:
            //if ((time_now - refx_time_change) > GetTechArgv(sbr, pump_rflx_off_time) * 60)  //  关闭时间超时
            if (1)
            {
                if ((time_now - refx_time_cycle) > GetTechArgv(sbr, pump_rflx_cycle_time) * 60)  //  周期切换时间超时
                {
                    refx_state = SBR_PUMP_ST_PRIMARY_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    refx_state = SBR_PUMP_ST_MINOR_A;
                }
                refx_time_change = time_now;
            }
            break;
        default:
            refx_state = SBR_PUMP_ST_INIT;
            break;
    }
    if (IsSBRReflexPumpOpen())
    {
        pump_state.pump_st.pump_rflx = 1;
    }
    else
    {
        pump_state.pump_st.pump_rflx = 0;
    }
}


/**  @breif  设备5、加药泵
 **  @notice 2台（端口13-14），分别加PAC及次氯酸钠。
 **          和潜污泵联动并受液位L7和L8（加药桶液位）控制，潜污泵开启后A（可调，A默认值为10）分钟且L7和L8为高液位，
 **          两台加药泵同时启动，若L7和L8中有一个为低液位，则加药泵停止。若L7和L8为高液位，潜污泵停止，则加药泵延迟A分钟停止。
 **/

#define IsSBRDosingPumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, dosg1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, dosg2)) == PUMP_ST_ON)

void SBRDosingPumpCtrl(void)
{
    static uint8_t dosg_state = SBR_PUMP_ST_INIT;
    //static uint32_t watr_time_delay = 0;
    static s_subm_open_t subm_open_temp;
    s_subm_open_t *subm_open_ptr;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (dosg_state)
    {
        case SBR_PUMP_ST_INIT:
            if ((subm_open_ptr = GetSubmOpenTimestamp()) != 0)
            {
                subm_open_temp = *subm_open_ptr;
                if (subm_open_temp.type == 0)
                {
                    dosg_state = SBR_PUMP_ST_PRIMARY_B;
                }
                else
                {
                    dosg_state = SBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            if ((time_now - subm_open_temp.start) > (GetTechArgv(sbr, pump_dosg_delay_time) * 60))
            {
                if ((GetLiquidLevel(SBR_LLP_DOSING1_HIGH)) && (GetLiquidLevel(SBR_LLP_DOSING2_HIGH)))
                {
                    PumpCtrlByTech(GetPumpPort(sbr, dosg1), 1);
                    PumpCtrlByTech(GetPumpPort(sbr, dosg2), 1);
                }  
                else
                {
                    PumpCtrlByTech(GetPumpPort(sbr, dosg1), 0);
                    PumpCtrlByTech(GetPumpPort(sbr, dosg2), 0);
                }   
                if ((subm_open_ptr = GetSubmOpenTimestamp()) != 0)
                {
                    if (subm_open_ptr->type == 0)
                    {
                        subm_open_temp = *subm_open_ptr;
                        dosg_state = SBR_PUMP_ST_PRIMARY_B;
                    }
                }
            }
            break;
        case SBR_PUMP_ST_PRIMARY_B:
            if ((time_now - subm_open_temp.start) > (GetTechArgv(sbr, pump_dosg_delay_time) * 60))
            {
                PumpCtrlByTech(GetPumpPort(sbr, dosg1), 0);
                PumpCtrlByTech(GetPumpPort(sbr, dosg2), 0);
                dosg_state = SBR_PUMP_ST_INIT;
            }
            break;
        default:
            
            break;
    }
    if (IsSBRDosingPumpOpen())
    {
        pump_state.pump_st.pump_dosg = 1;
    }
    else
    {
        pump_state.pump_st.pump_dosg = 0;
    }
}



/**  @breif  设备2、潜污泵
 **  @notice 3台（端口6-8），一用二备，液位L4高液位（调节池液位）启动1台主泵或备用泵1，液位L4低液位停止。
 **          同时可时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
 **          主泵和备用泵1每２４ｈ自动切换运行，如主泵故障，则用自动启备用泵1，
 **          同理，备用泵故障，自动切换到主泵运行。
 **          液位L5高液位（调节池超高液位），启动备用泵2，L5低液位，备用泵2停止。
 **/

int8_t SBRSubmCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(sbr, subm1))
    {
        standby = GetPumpPort(sbr, subm2);
    }
    else if (channel == GetPumpPort(sbr, subm2))
    {
        standby = GetPumpPort(sbr, subm1);
    }
    else  //  参数错误
    {
        return -1;
    }
    if (GetLiquidLevel(SBR_LLP_SBR_POOL_HIGH))
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);
    }
    else if (GetLiquidLevel(SBR_LLP_SUBM_ULTRAHIGH) && value)
    {
        return PairOfPumpCtrlByTech(channel, standby, 2);
    }
    else if (GetLiquidLevel(SBR_LLP_SUBM_HIGH) && value)
    {
        return PairOfPumpCtrlByTech(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);
    }
}

#define IsSBRSubmersibleSewagePumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, subm1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, subm2)) == PUMP_ST_ON\
            )

int8_t SBRSubmersibleSewagePumpCtrl(void)
{
    static uint8_t state = SBR_PUMP_ST_INIT;
    static uint32_t time_change = 0;
    static uint32_t time_cycle = 0;
    uint32_t time_now = time(0);   //  以分钟为单位
    time_change = time_change;
    switch (state)
    {
        case SBR_PUMP_ST_INIT:
            time_change = time_now;
            time_cycle = time_now;
            state = SBR_PUMP_ST_PRIMARY_A;
            SBRSubmCtrl(GetPumpPort(sbr, subm1), 1);
            //open_time = 0;
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            //if ((time_now - time_change) > GetTechArgv(sbr, pump_subm_on_time) * 60)  //  开启时间超时
            if (0)
            {
                PumpCtrlByTech(GetPumpPort(sbr, subm1), 0);  //  关闭泵
                time_change = time_now;
                state = SBR_PUMP_ST_PRIMARY_B;
            }
            else if (SBRSubmCtrl(GetPumpPort(sbr, subm1), 1) ==  GetPumpPort(sbr, subm2))  //  检测泵是否坏
            {
                state = SBR_PUMP_ST_MINOR_A;
            }
            //break;
        case SBR_PUMP_ST_PRIMARY_B:
            //if ((time_now - time_change) > GetTechArgv(sbr, pump_subm_off_time) * 60)  //  关闭时间超时
            {
                //open_time = 0;
                time_change = time_now;
                if ((time_now - time_cycle) > GetTechArgv(sbr, pump_subm_cycle_time) * 60)  //  周期切换时间超时
                {
                    state = SBR_PUMP_ST_MINOR_A;
                    time_cycle = time_now;
                }
                else
                {
                    state = SBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case SBR_PUMP_ST_MINOR_A:
            //if ((time_now - time_change) > GetTechArgv(sbr, pump_subm_on_time) * 60)  //  开启时间超时
            if (0)
            {
                PumpCtrlByTech(GetPumpPort(sbr, subm2), 0);  //  关闭泵
                time_change = time_now;
                state = SBR_PUMP_ST_MINOR_B;
            }
            else if (SBRSubmCtrl(GetPumpPort(sbr, subm2), 1) ==  GetPumpPort(sbr, subm1))  //  检测泵是否坏
            {
                state = SBR_PUMP_ST_PRIMARY_A;
            }
            //break;
        case SBR_PUMP_ST_MINOR_B:
            //if ((time_now - time_change) > GetTechArgv(sbr, pump_subm_off_time) * 60)  //  关闭时间超时
            {
                //open_time = 0;
                if ((time_now - time_cycle) > GetTechArgv(sbr, pump_subm_cycle_time) * 60)  //  周期切换时间超时
                {
                    state = SBR_PUMP_ST_PRIMARY_A;
                    time_cycle = time_now;
                }
                else
                {
                    state = SBR_PUMP_ST_MINOR_A;
                }
                time_change = time_now;
            }
            break;
        default:
            state = SBR_PUMP_ST_INIT;
            break;
    }
    if (IsSBRSubmersibleSewagePumpOpen())
    {
        pump_state.pump_st.pump_subm = 1;
    }
    else
    {
        pump_state.pump_st.pump_subm = 0;
    }
    if (GetLiquidLevel(SBR_LLP_SBR_POOL_HIGH) && (pump_state.pump_st.pump_subm == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


#define IsSBRAerationPumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, aera1))==PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, aera2))==PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, aera3))==PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, aera4))==PUMP_ST_ON)

//  曝气泵控制
//设备1、曝气泵
//3台（端口3-5），三台泵分别单独时间控制，默认0：00~23:00开启，23:01~23:59停止，时间可调。
int8_t SBRAerationPumpCtrl(void)
 {
    static uint8_t state = SBR_PUMP_ST_INIT;
    static uint32_t time_change = 0;
    static uint32_t time_cycle = 0;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (state)
    {
        case SBR_PUMP_ST_INIT:
            time_change = time_now;
            time_cycle = time_now;
            state = SBR_PUMP_ST_PRIMARY_A;
            //open_time = 0;
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            if ((time_now - time_change) > GetTechArgv(sbr, pump_aera_run_time) * 60)  //  开启时间超时
            {
                PairOfPumpCtrlByTech(GetPumpPort(sbr, aera1), GetPumpPort(sbr, aera2), 0);  //  关闭泵
                if (GetPumpState(GetPumpPort(sbr, aera1))!=PUMP_ST_ON &&  GetPumpState(GetPumpPort(sbr, aera2))!=PUMP_ST_ON)
                {
                    state = SBR_PUMP_ST_PRIMARY_B;
                }
            }
            else
            {
                PairOfPumpCtrlByTech(GetPumpPort(sbr, aera1), GetPumpPort(sbr, aera2), 2);
            }
            break;
        case SBR_PUMP_ST_PRIMARY_B:
            time_change = time_now;
            if ((time_now - time_cycle) > GetTechArgv(sbr, pump_aera_cycle_time) * 60)  //  开启时间超时
            {
                state = SBR_PUMP_ST_PRIMARY_A;
                time_cycle = time_now;
            }
            else
            {
                state = SBR_PUMP_ST_MINOR_A;
            }
            break;
        case SBR_PUMP_ST_MINOR_A:
            if ((time_now - time_change) > GetTechArgv(sbr, pump_aera_run_time) * 60)  //  开启时间超时
            {
                PairOfPumpCtrlByTech(GetPumpPort(sbr, aera3), GetPumpPort(sbr, aera4), 0);  //  关闭泵
                if (GetPumpState(GetPumpPort(sbr, aera3))!=PUMP_ST_ON &&  GetPumpState(GetPumpPort(sbr, aera4))!=PUMP_ST_ON)
                {
                    state = SBR_PUMP_ST_PRIMARY_B;
                }
            }
            else
            {
                PairOfPumpCtrlByTech(GetPumpPort(sbr, aera3), GetPumpPort(sbr, aera4), 2);
            }
            break;
        case SBR_PUMP_ST_MINOR_B:
            time_change = time_now;
            if ((time_now - time_cycle) > GetTechArgv(sbr, pump_aera_cycle_time) * 60)  //  开启时间超时
            {
                state = SBR_PUMP_ST_MINOR_A;
                time_cycle = time_now;
            }
            else
            {
                state = SBR_PUMP_ST_PRIMARY_A;
            }
            break;
        default:
            state = SBR_PUMP_ST_INIT;
            break;
    }
    if (IsSBRAerationPumpOpen())
    {
        pump_state.pump_st.pump_aera = 1;
    }
    else
    {
        pump_state.pump_st.pump_aera = 0;
    }
    if (((state == SBR_PUMP_ST_PRIMARY_B) || (state == SBR_PUMP_ST_MINOR_B))
        && (pump_state.pump_st.pump_aera == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**  @breif  设备4、出水泵
 **  @notice 2台（端口11-12），一用一备，液位L6（沉淀池液位）高液位启动主泵，液位L6低液位，主泵停止。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用用备用泵，同理，备用泵故障，自动切换到主泵运行。

 **/

int8_t SBRWaterCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(sbr, watr1))
    {
        standby = GetPumpPort(sbr, watr2);
    }
    else if (channel == GetPumpPort(sbr, watr2))
    {
        standby = GetPumpPort(sbr, watr1);
    }
    else  //  参数错误
    {
        return 0;
    }
    if ((GetLiquidLevel(SBR_LLP_SUBM_HIGH) || GetLiquidLevel(SBR_LLP_SUBM_ULTRAHIGH)) && 
        (GetLiquidLevel(SBR_LLP_SBR_POOL_HIGH)) && value)
    {
        return PairOfPumpCtrlByTech(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrlByTech(channel, standby, 0);
    }
}

#define IsSBRWaterPumpOpen()   \
            (GetPumpState(GetPumpPort(sbr, watr1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(sbr, watr2)) == PUMP_ST_ON)
int8_t SBRWaterPumpCtrl(void)
{
//    uint8_t pump_st = 0;
    static uint8_t watr_state = SBR_PUMP_ST_INIT;
    static uint8_t open_state = 0;
    static uint32_t watr_time_cycle = 0;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (watr_state)
    {
        case SBR_PUMP_ST_INIT:
            watr_time_cycle = time_now;
            watr_state = SBR_PUMP_ST_PRIMARY_A;
            break;
        case SBR_PUMP_ST_PRIMARY_A:
            if ((time_now - watr_time_cycle) > GetTechArgv(sbr, pump_watr_cycle_time) * 60)  //  周期切换时间超时
            {
                watr_state = SBR_PUMP_ST_MINOR_A;
                watr_time_cycle = time_now;
            }
            else if (SBRWaterCtrl(GetPumpPort(sbr, watr1), 1) == GetPumpPort(sbr, watr2))
            {
                watr_state = SBR_PUMP_ST_MINOR_A;
            }
            break;
        
        case SBR_PUMP_ST_MINOR_A:
            if ((time_now - watr_time_cycle) > GetTechArgv(sbr, pump_watr_cycle_time) * 60)  //  周期切换时间超时
            {
                watr_state = SBR_PUMP_ST_PRIMARY_A;
                watr_time_cycle = time_now;
            }
            else if (SBRWaterCtrl(GetPumpPort(sbr, watr2), 1) == GetPumpPort(sbr, watr1))
            {
                watr_state = SBR_PUMP_ST_PRIMARY_A;
            }
            break;
        default:
            watr_state = SBR_PUMP_ST_INIT;
            break;
    }
    
    if (open_state != IsSBRWaterPumpOpen())
    {
        s_subm_open_t subm_open_current;
        open_state = IsSBRWaterPumpOpen();
        if (IsSBRWaterPumpOpen())
        {
            pump_state.pump_st.pump_watr = 1;
            subm_open_current.type = 1;
        }
        else
        {
            pump_state.pump_st.pump_watr = 0;
            subm_open_current.type = 0;
        }
        subm_open_current.start = time(0);
        AddSubmOpenTimestamp(&subm_open_current);
    }
    if ((GetLiquidLevel(SBR_LLP_SUBM_HIGH) || GetLiquidLevel(SBR_LLP_SUBM_HIGH)) && 
        (GetLiquidLevel(SBR_LLP_SBR_POOL_HIGH) == 0) && (pump_state.pump_st.pump_watr == 0))
    {
        return 1;
    }
    return 0;
}


void FlowSBR(void)
{
    static uint8_t sbr_step = 0;
    static uint32_t sbr_step_4_time = 0;
    SBRLiftingPumpCtrl();
    switch (sbr_step)
    {
        case 0: 
            SBRReflexPumpCtrl();
            SBRDosingPumpCtrl();
            if (SBRSubmersibleSewagePumpCtrl())
            {
                sbr_step = 1;
            }
            break;
        case 1: 
            SBRReflexPumpCtrl();
            SBRDosingPumpCtrl();
            if (SBRAerationPumpCtrl())
            {
                sbr_step = 2;
            }
            break;
        case 2: 
            if ((pump_state.pump_st.pump_subm == 0) && (pump_state.pump_st.pump_aera == 0) &&
                (pump_state.pump_st.pump_watr == 0) && (pump_state.pump_st.pump_dosg == 0) &&
                (pump_state.pump_st.pump_rflx == 0))
            {
                sbr_step = 3;
                PairOfPumpCtrlByTech(GetPumpPort(sbr, dosg1), GetPumpPort(sbr, dosg2), 0);
                PairOfPumpCtrlByTech(GetPumpPort(sbr, rflx2), GetPumpPort(sbr, rflx2), 0);
                sbr_step_4_time = time(0);
            }
            break;
        case 3:
            if ((time(0) - sbr_step_4_time) > GetTechArgv(sbr, precipitate_time) * 60)
            {
                sbr_step = 4;
            }
            break;
        case 4: 
            SBRReflexPumpCtrl();
            SBRDosingPumpCtrl();
            if (SBRWaterPumpCtrl())
            {
                sbr_step = 0;
            }
            break;
        default: sbr_step = 0; break;
    }
    GetTechArgv(sbr, current_phase) = sbr_step;
}


