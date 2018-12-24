/*  MBR.c
 *  实现MBR(商达)工艺的处理
 *   设备1、曝气泵
        2台，两台泵分别单独时间控制，默认0：00~23:00开启，23:01~23:59停止，时间可调。
     设备2、潜污泵
         2台，一用一备，液位L3高液位且L5是低液位，启动1台主泵或备用泵1，液位L3低液位或L5为高液位停止。
         同时可时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
         主泵和备用泵1每２４ｈ（可调）自动切换运行，如主泵故障，则用自动启备用泵1，
         同理，备用泵故障，自动切换到主泵运行。L4高液位且L5低液位时，主泵和备用泵同时启动。
     设备3、吸膜泵
         2台（端口9-10），一用一备，液位L5和时间控制（开启A分钟关闭B分钟）。
         曝气泵开启状态下，L5高液位且洗膜泵停止时，开启A分钟关闭B分钟，循环执行。
         任一条件不满足，则停止；主泵和备用泵每24小时切换运行，如主泵故障，则启用备用泵，
         同理，备用泵故障，自动切换到主泵运行。
     设备4、加药泵（L6和L7为高液位时，加药泵动作是吸膜泵动作的延迟执行）
         2台（端口11-12），分别加PAC及次氯酸钠。和吸膜泵联动并受液位L6和L7（加药桶液位）控制，
         潜污泵开启后A（可调，A默认值为10）分钟且L6和L7为高液位，两台加药泵同时启动，
         启动时长和关闭时长与潜污泵一致。若L6和L7中有一个为低液位，则加药泵停止。
         若L6和L7为高液位，潜污泵停止后，加药泵对应延迟A分钟停止。（L6,L7检测药量的）
     设备5、洗膜泵
         2台（端口13-14），一用一备，L8高液位时，时间控制，开启A分钟，关闭B分钟，循环执行，
         在启动期间洗膜泵停止，洗膜泵的控制优先级高于吸膜泵。（A，B参数可调，分别默认15和480）
     设备6、提升泵
         2台（端口1-2），一用一备，液位L1（提升机液位）高液位且L3（调节池液位）低液位启动主泵，
         液位L1低液位或L3高液位时，主泵停止，２台水泵２４ｈ自动切换运行。如主泵故障，则启用备用泵，
         同理，备用泵故障，自动切换到主泵运行。液位L2高液位且L3低液位同时启动主泵和备用泵。
     设备7、回流泵
         2台（端口7-8），一用一备，时间控制，开启A分钟，关闭B分钟，循环执行。
         2台水泵24h（可调）自动切换运行。如主泵故障，则用启用备用泵，
         同理，备用泵故障，自动切换到主泵运行。
 
 */
#include "a2o.h"
#include "mbr.h"
#include "pump.h"
#include "my_time.h"
#include "sys_config.h"

/*
液位说明：
1、L1、L2分别为提升井中的高液位和超高液位
2、L3、L4为调节池高液位、超高液位
3、L5为MBR池液位
4、L6和L7为加药桶液位
5、L8清水池液位
 */
#define MBR_LLP_LIFTING_HIGH           1
#define MBR_LLP_LIFTING_ULTRAHIGH      2
#define MBR_LLP_SUBM_HIGH              3
#define MBR_LLP_SUBM_ULTRAHIGH         4
#define MBR_LLP_MBR_POOL_HIGH          5
#define MBR_LLP_DOSING1_HIGH           6
#define MBR_LLP_DOSING2_HIGH           7
#define MBR_LLP_CLEAN_WATER_HIGH       8


enum e_mbr_pump_state
{
    MBR_PUMP_ST_INIT,
    MBR_PUMP_ST_PRIMARY_A,
    MBR_PUMP_ST_PRIMARY_B,
    MBR_PUMP_ST_MINOR_A,
    MBR_PUMP_ST_MINOR_B
} ;


#define IsMBRAerationPumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, aera1))==PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, aera2))==PUMP_ST_ON)

//  曝气泵控制
//设备1、曝气泵
//3台（端口3-5），三台泵分别单独时间控制，默认0：00~23:00开启，23:01~23:59停止，时间可调。
void MBRAerationPumpCtrl(void)
 {
    uint32_t now_ts = time(0);
    struct tm now = *localtime(&now_ts);
    if (GetPumpPort(mbr, aera1))  //  泵是否使能, 配置了端口表示使能
    {
        //  判断对应时间是否需要开启
        if ((1<< now.tm_hour) & GetPumpArgv(mbr, pump_aera1_time))
        {
            //  当前小时需要开启
            SinglePumpCtrl(GetPumpPort(mbr, aera1), 1);
        }
        else
        {
            SinglePumpCtrl(GetPumpPort(mbr, aera1), 0);
        }
    }
    
    if (GetPumpPort(mbr, aera2))  //  泵是否使能, 配置了端口表示使能
    {
        //  判断对应时间是否需要开启
        if ((1<< now.tm_hour) & GetPumpArgv(mbr, pump_aera2_time))
        {
            //  当前小时需要开启
            SinglePumpCtrl(GetPumpPort(mbr, aera2), 1);
        }
        else
        {
            SinglePumpCtrl(GetPumpPort(mbr, aera2), 0);
        }
    }
    if (IsMBRAerationPumpOpen())
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

int8_t MBRSubmCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(mbr, subm1))
    {
        standby = GetPumpPort(mbr, subm2);
    }
    else if (channel == GetPumpPort(mbr, subm2))
    {
        standby = GetPumpPort(mbr, subm1);
    }
    else  //  参数错误
    {
        return -1;
    }
    if (GetLiquidLevel(MBR_LLP_MBR_POOL_HIGH))
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
    else if (GetLiquidLevel(MBR_LLP_SUBM_ULTRAHIGH) && value)
    {
        return PairOfPumpCtrl(channel, standby, 2);
    }
    else if (GetLiquidLevel(MBR_LLP_SUBM_HIGH) && value)
    {
        return PairOfPumpCtrl(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
}

#define IsMBRSubmersibleSewagePumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, subm1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, subm2)) == PUMP_ST_ON\
            )

void MBRSubmersibleSewagePumpCtrl(void)
{
    static uint8_t state = MBR_PUMP_ST_INIT;
    static uint32_t time_change = 0;
    static uint32_t time_cycle = 0;
    static uint8_t open_state;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (state)
    {
        case MBR_PUMP_ST_INIT:
            time_change = time_now;
            time_cycle = time_now;
            state = MBR_PUMP_ST_PRIMARY_A;
            MBRSubmCtrl(GetPumpPort(mbr, subm1), 1);
            //open_time = 0;
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - time_change) > GetPumpArgv(mbr, pump_subm_on_time))  //  开启时间超时
            {
                SinglePumpCtrl(GetPumpPort(mbr, subm1), 0);  //  关闭泵
                time_change = time_now;
                state = MBR_PUMP_ST_PRIMARY_B;
            }
            else if (MBRSubmCtrl(GetPumpPort(mbr, subm1), 1) ==  GetPumpPort(mbr, subm2))  //  检测泵是否坏
            {
                state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        case MBR_PUMP_ST_PRIMARY_B:
            if ((time_now - time_change) > GetPumpArgv(mbr, pump_subm_off_time))  //  关闭时间超时
            {
                //open_time = 0;
                time_change = time_now;
                if ((time_now - time_cycle) > GetPumpArgv(mbr, pump_subm_cycle_time))  //  周期切换时间超时
                {
                    state = MBR_PUMP_ST_MINOR_A;
                    time_cycle = time_now;
                }
                else
                {
                    state = MBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case MBR_PUMP_ST_MINOR_A:
            if ((time_now - time_change) > GetPumpArgv(mbr, pump_subm_on_time))  //  开启时间超时
            {
                SinglePumpCtrl(GetPumpPort(mbr, subm2), 0);  //  关闭泵
                time_change = time_now;
                state = MBR_PUMP_ST_MINOR_B;
            }
            else if (MBRSubmCtrl(GetPumpPort(mbr, subm2), 1) ==  GetPumpPort(mbr, subm1))  //  检测泵是否坏
            {
                state = MBR_PUMP_ST_PRIMARY_A;
            }
            break;
        case MBR_PUMP_ST_MINOR_B:
            if ((time_now - time_change) > GetPumpArgv(mbr, pump_subm_off_time))  //  关闭时间超时
            {
                //open_time = 0;
                if ((time_now - time_cycle) > GetPumpArgv(mbr, pump_subm_cycle_time))  //  周期切换时间超时
                {
                    state = MBR_PUMP_ST_PRIMARY_A;
                    time_cycle = time_now;
                }
                else
                {
                    state = MBR_PUMP_ST_MINOR_A;
                }
                time_change = time_now;
            }
            break;
        default:
            state = MBR_PUMP_ST_INIT;
            break;
    }
    if (open_state != IsMBRSubmersibleSewagePumpOpen())
    {
        s_subm_open_t subm_open_current;
        open_state = IsMBRSubmersibleSewagePumpOpen();
        if (IsMBRSubmersibleSewagePumpOpen())
        {
            pump_state.pump_st.pump_subm = 1;
            subm_open_current.type = 1;
        }
        else
        {
            pump_state.pump_st.pump_subm = 0;
            subm_open_current.type = 0;
        }
        subm_open_current.start = time(0);
        AddSubmOpenTimestamp(&subm_open_current);
    }
}

/**  @breif  设备3、回流泵
 **  @notice 2台（端口9-10），一用一备，时间控制，开启A分钟，关闭B分钟，循环执行（A，B参数可调）。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则用启用备用泵，同理，备用泵故障，自动切换到主泵运行。
 **/
int8_t MBRReflexCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(mbr, rflx1))
    {
        standby = GetPumpPort(mbr, rflx2);
    }
    else if (channel == GetPumpPort(mbr, rflx2))
    {
        standby = GetPumpPort(mbr, rflx1);
    }
    else  //  参数错误
    {
        return 0;
    }
    if (value)
    {
        return PairOfPumpCtrl(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
}

#define IsMBRReflexPumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, rflx1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, rflx2)) == PUMP_ST_ON)

void MBRReflexPumpCtrl(void)
{   
    static uint8_t refx_state = MBR_PUMP_ST_INIT;
    static uint32_t refx_time_change = 0;
    static uint32_t refx_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (refx_state)
    {
        case MBR_PUMP_ST_INIT:
            refx_time_change = time_now;
            refx_time_cycle = time_now;
            refx_state = MBR_PUMP_ST_PRIMARY_A;
            MBRReflexCtrl(GetPumpPort(mbr, rflx1), 1);
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - refx_time_change) > GetPumpArgv(mbr, pump_rflx_on_time))  //  开启时间超时
            {
                MBRReflexCtrl(GetPumpPort(mbr, rflx1), 0);  //  关闭主泵
                refx_time_change = time_now;
                refx_state = MBR_PUMP_ST_PRIMARY_B;
            }
            else if (MBRReflexCtrl(GetPumpPort(mbr, rflx1), 1) == GetPumpPort(mbr, rflx2))  //  检测泵是否坏
            {
                refx_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        case MBR_PUMP_ST_PRIMARY_B:
            if ((time_now - refx_time_change) > GetPumpArgv(mbr, pump_rflx_off_time))  //  关闭时间超时
            {
                refx_time_change = time_now;
                if ((time_now - refx_time_cycle) > GetPumpArgv(mbr, pump_rflx_cycle_time))  //  周期切换时间超时
                {
                    refx_state = MBR_PUMP_ST_MINOR_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    refx_state = MBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case MBR_PUMP_ST_MINOR_A:
            if ((time_now - refx_time_change) > GetPumpArgv(mbr, pump_rflx_on_time))  //  开启时间超时
            {
                MBRReflexCtrl(GetPumpPort(mbr, rflx2), 0);  //  关闭备用泵
                refx_time_change = time_now;
                refx_state = MBR_PUMP_ST_MINOR_B;
            }
            else if (MBRReflexCtrl(GetPumpPort(mbr, rflx2), 1) == GetPumpPort(mbr, rflx1))  //  检测泵是否坏
            {
                refx_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        case MBR_PUMP_ST_MINOR_B:
            if ((time_now - refx_time_change) > GetPumpArgv(mbr, pump_rflx_off_time))  //  关闭时间超时
            {
                if ((time_now - refx_time_cycle) > GetPumpArgv(mbr, pump_rflx_cycle_time))  //  周期切换时间超时
                {
                    refx_state = MBR_PUMP_ST_PRIMARY_A;
                    refx_time_cycle = time_now;
                }
                else
                {
                    refx_state = MBR_PUMP_ST_MINOR_A;
                }
                refx_time_change = time_now;
            }
            break;
        default:
            refx_state = MBR_PUMP_ST_INIT;
            break;
    }
    if (IsMBRReflexPumpOpen())
    {
        pump_state.pump_st.pump_rflx = 1;
    }
    else
    {
        pump_state.pump_st.pump_rflx = 0;
    }
}

/**  @breif  设备4、洗膜泵
 **  @notice 2台（端口11-12），一用一备，液位L6（沉淀池液位）高液位启动主泵，液位L6低液位，主泵停止。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用用备用泵，同理，备用泵故障，自动切换到主泵运行。

 **/

int8_t MBRWaterMembraneCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(mbr, wmbr1))
    {
        standby = GetPumpPort(mbr, wmbr2);
    }
    else if (channel == GetPumpPort(mbr, wmbr2))
    {
        standby = GetPumpPort(mbr, wmbr1);
    }
    else  //  参数错误
    {
        return 0;
    }
    if ((GetLiquidLevel(MBR_LLP_CLEAN_WATER_HIGH)) && value)
    {
        return PairOfPumpCtrl(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
}

#define IsMBRWaterMembranePumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, wmbr1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, wmbr2)) == PUMP_ST_ON)
void MBRWaterMembranePumpCtrl(void)
{
//    uint8_t pump_st = 0;
    static uint8_t wmbr_state = MBR_PUMP_ST_INIT;
    static uint32_t wmbr_time_change = 0;
    static uint32_t wmbr_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    wmbr_time_cycle = wmbr_time_cycle;  //  去除警告
    switch (wmbr_state)
    {
        case MBR_PUMP_ST_INIT:
//            wmbr_time_cycle = time_now;
            wmbr_state = MBR_PUMP_ST_PRIMARY_A;
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - wmbr_time_change) > GetPumpArgv(mbr, pump_wmbr_on_time))
            {
                wmbr_time_change = time_now;
                SinglePumpCtrl(GetPumpPort(mbr, wmbr1), 0);
                wmbr_state = MBR_PUMP_ST_PRIMARY_B;
                break;
            }
            else if (MBRWaterMembraneCtrl(GetPumpPort(mbr, wmbr1), 1) == GetPumpPort(mbr, wmbr2))
            {
                //wmbr_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        
        case MBR_PUMP_ST_PRIMARY_B:
            if ((time_now - wmbr_time_change) > GetPumpArgv(mbr, pump_wmbr_off_time))
            {
                wmbr_time_change = time_now;
                //  不要24小时更换
                //if ((time_now - wmbr_time_cycle) > GetPumpArgv(mbr, pump_wmbr_cycle_time))  //  周期切换时间超时
                if (0)
                {
                    wmbr_state = MBR_PUMP_ST_MINOR_A;
                    wmbr_time_cycle = time_now;
                }
                else
                {
                    wmbr_state = MBR_PUMP_ST_PRIMARY_A;
                }
                break;
            }
            break;
            
        case MBR_PUMP_ST_MINOR_A:
            if ((time_now - wmbr_time_change) > GetPumpArgv(mbr, pump_wmbr_on_time))
            {
                wmbr_time_change = time_now;
                SinglePumpCtrl(GetPumpPort(mbr, wmbr2), 0);
                wmbr_state = MBR_PUMP_ST_MINOR_B;
                break;
            }
            else if (MBRWaterMembraneCtrl(GetPumpPort(mbr, wmbr2), 1) == GetPumpPort(mbr, wmbr1))
            {
                wmbr_state = MBR_PUMP_ST_PRIMARY_A;
            }
            break;
            
        case MBR_PUMP_ST_MINOR_B:
            if ((time_now - wmbr_time_change) > GetPumpArgv(mbr, pump_wmbr_off_time))
            {
                wmbr_time_change = time_now;
                //  不要24小时更换
                //if ((time_now - wmbr_time_cycle) > GetPumpArgv(mbr, pump_wmbr_cycle_time))  //  周期切换时间超时
                if (1)
                {
                    wmbr_state = MBR_PUMP_ST_PRIMARY_A;
                    wmbr_time_cycle = time_now;
                }
                else
                {
                    wmbr_state = MBR_PUMP_ST_MINOR_A;
                }
                break;
            }
            break;
        default:
            wmbr_state = MBR_PUMP_ST_INIT;
            break;
    }
    if (IsMBRWaterMembranePumpOpen())
    {
        pump_state.pump_st.pump_wash = 1;
    }
    else
    {
        pump_state.pump_st.pump_wash = 0;
    }
}


/**  @breif  设备4、吸膜泵
 **  @notice 2台（端口11-12），一用一备，液位L6（沉淀池液位）高液位启动主泵，液位L6低液位，主泵停止。
 **          2台水泵２４ｈ自动切换运行。如主泵故障，则启用用备用泵，同理，备用泵故障，自动切换到主泵运行。

 **/

int8_t MBRSuckMembraneCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(mbr, smbr1))
    {
        standby = GetPumpPort(mbr, smbr2);
    }
    else if (channel == GetPumpPort(mbr, smbr2))
    {
        standby = GetPumpPort(mbr, smbr1);
    }
    else  //  参数错误
    {
        return 0;
    }
    if ((GetLiquidLevel(MBR_LLP_MBR_POOL_HIGH)) && (IsMBRAerationPumpOpen()) && 
        (IsMBRWaterMembranePumpOpen() == 0) && value)
    {
        return PairOfPumpCtrl(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
}

#define IsMBRSuckMembranePumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, smbr1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, smbr2)) == PUMP_ST_ON)
void MBRSuckMembranePumpCtrl(void)
{
//    uint8_t pump_st = 0;
    static uint8_t smbr_state = MBR_PUMP_ST_INIT;
    static uint32_t smbr_time_change = 0;
    static uint32_t smbr_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (smbr_state)
    {
        case MBR_PUMP_ST_INIT:
            smbr_time_cycle = time_now;
            smbr_state = MBR_PUMP_ST_PRIMARY_A;
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - smbr_time_change) > GetPumpArgv(mbr, pump_smbr_on_time))
            {
                smbr_time_change = time(0)/60;
                SinglePumpCtrl(GetPumpPort(mbr, smbr1), 0);
                smbr_state = MBR_PUMP_ST_PRIMARY_B;
                break;
            }
            else if (MBRWaterMembraneCtrl(GetPumpPort(mbr, smbr1), 1) == GetPumpPort(mbr, smbr2))
            {
                smbr_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        
        case MBR_PUMP_ST_PRIMARY_B:
            if ((time_now - smbr_time_change) > GetPumpArgv(mbr, pump_smbr_off_time))
            {
                smbr_time_change = time(0)/60;
                if ((time_now - smbr_time_cycle) > GetPumpArgv(mbr, pump_smbr_cycle_time))  //  周期切换时间超时
                {
                    smbr_time_cycle = time(0)/60;
                    smbr_state = MBR_PUMP_ST_MINOR_A;
                    smbr_time_cycle = time_now;
                }
                else
                {
                    smbr_state = MBR_PUMP_ST_PRIMARY_A;
                }
                break;
            }
            break;
            
        case MBR_PUMP_ST_MINOR_A:
            if ((time_now - smbr_time_change) > GetPumpArgv(mbr, pump_smbr_on_time))
            {
                smbr_time_change = time(0)/60;
                SinglePumpCtrl(GetPumpPort(mbr, smbr2), 0);
                smbr_state = MBR_PUMP_ST_MINOR_B;
                break;
            }
            else if (MBRWaterMembraneCtrl(GetPumpPort(mbr, smbr2), 1) == GetPumpPort(mbr, smbr1))
            {
                smbr_state = MBR_PUMP_ST_PRIMARY_A;
            }
            break;
            
        case MBR_PUMP_ST_MINOR_B:
            if ((time_now - smbr_time_change) > GetPumpArgv(mbr, pump_smbr_off_time))
            {
                smbr_time_change = time(0)/60;
                if ((time_now - smbr_time_cycle) > GetPumpArgv(mbr, pump_smbr_cycle_time))  //  周期切换时间超时
                {
                    smbr_time_cycle = time(0)/60;
                    smbr_state = MBR_PUMP_ST_PRIMARY_A;
                    smbr_time_cycle = time_now;
                }
                else
                {
                    smbr_state = MBR_PUMP_ST_MINOR_A;
                }
                break;
            }
            break;
        default:
            smbr_state = MBR_PUMP_ST_INIT;
            break;
    }
    if (IsMBRSuckMembranePumpOpen())
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

#define IsMBRDosingPumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, dosg1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, dosg2)) == PUMP_ST_ON)

void MBRDosingPumpCtrl(void)
{
    static uint8_t dosg_state = MBR_PUMP_ST_INIT;
    //static uint32_t watr_time_delay = 0;
    static s_subm_open_t subm_open_temp;
    s_subm_open_t *subm_open_ptr;
    uint32_t time_now = time(0);   //  以分钟为单位
    switch (dosg_state)
    {
        case MBR_PUMP_ST_INIT:
            if ((subm_open_ptr = GetSubmOpenTimestamp()) != 0)
            {
                subm_open_temp = *subm_open_ptr;
                if (subm_open_temp.type == 0)
                {
                    dosg_state = MBR_PUMP_ST_PRIMARY_B;
                }
                else
                {
                    dosg_state = MBR_PUMP_ST_PRIMARY_A;
                }
            }
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - subm_open_temp.start) > (GetPumpArgv(mbr, pump_dosg_delay_time) * 60))
            {
                if ((GetLiquidLevel(MBR_LLP_DOSING1_HIGH)) && (GetLiquidLevel(MBR_LLP_DOSING2_HIGH)))
                {
                    SinglePumpCtrl(GetPumpPort(mbr, dosg1), 1);
                    SinglePumpCtrl(GetPumpPort(mbr, dosg2), 1);
                }  
                else
                {
                    SinglePumpCtrl(GetPumpPort(mbr, dosg1), 0);
                    SinglePumpCtrl(GetPumpPort(mbr, dosg2), 0);
                }   
                if ((subm_open_ptr = GetSubmOpenTimestamp()) != 0)
                {
                    if (subm_open_ptr->type == 0)
                    {
                        subm_open_temp = *subm_open_ptr;
                        dosg_state = MBR_PUMP_ST_PRIMARY_B;
                    }
                }
            }
            break;
        case MBR_PUMP_ST_PRIMARY_B:
            if ((time_now - subm_open_temp.start) > (GetPumpArgv(mbr, pump_dosg_delay_time) * 60))
            {
                SinglePumpCtrl(GetPumpPort(mbr, dosg1), 0);
                SinglePumpCtrl(GetPumpPort(mbr, dosg2), 0);
                dosg_state = MBR_PUMP_ST_INIT;
            }
            break;
        default:
            
            break;
    }
    if (IsMBRDosingPumpOpen())
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

int8_t MBRLiftingCtrl(uint32_t channel, uint8_t value)
{
    uint32_t standby;
    if (channel == GetPumpPort(mbr, lift1))
    {
        standby = GetPumpPort(mbr, lift2);
    }
    else if (channel == GetPumpPort(mbr, lift2))
    {
        standby = GetPumpPort(mbr, lift1);
    }
    else  //  参数错误
    {
        return 0;
    }
    
    if ((GetLiquidLevel(MBR_LLP_SUBM_HIGH)) || (GetLiquidLevel(MBR_LLP_SUBM_ULTRAHIGH)))
    {
        return PairOfPumpCtrl(channel, standby, 0);                
    }
    else if (GetLiquidLevel(MBR_LLP_LIFTING_ULTRAHIGH))
    {
        return PairOfPumpCtrl(channel, standby, 2);
    }
    else if (GetLiquidLevel(MBR_LLP_LIFTING_HIGH))
    {
        return PairOfPumpCtrl(channel, standby, 1);
    }
    else
    {
        return PairOfPumpCtrl(channel, standby, 0);
    }
}


#define IsMBRLiftingPumpOpen()   \
            (GetPumpState(GetPumpPort(mbr, lift1)) == PUMP_ST_ON || \
             GetPumpState(GetPumpPort(mbr, lift2)) == PUMP_ST_ON)

void MBRLiftingPumpCtrl(void)
{
    static uint8_t lift_state = MBR_PUMP_ST_INIT;
    static uint32_t lift_time_cycle = 0;
    uint32_t time_now = time(0)/60;   //  以分钟为单位
    switch (lift_state)
    {
        case MBR_PUMP_ST_INIT:
            lift_time_cycle = time_now;
            lift_state = MBR_PUMP_ST_PRIMARY_A;
            break;
        case MBR_PUMP_ST_PRIMARY_A:
            if ((time_now - lift_time_cycle) > GetPumpArgv(mbr, pump_lift_cycle_time))  //  周期切换时间超时
            {
                lift_state = MBR_PUMP_ST_MINOR_A;
                lift_time_cycle = time_now;
            }
            else if (MBRLiftingCtrl(GetPumpPort(mbr, lift1), 1) == GetPumpPort(mbr, lift2))
            {
                lift_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
            
        case MBR_PUMP_ST_MINOR_A:
            if ((time_now - lift_time_cycle) > GetPumpArgv(mbr, pump_lift_cycle_time))  //  周期切换时间超时
            {
                lift_state = MBR_PUMP_ST_PRIMARY_A;
                lift_time_cycle = time_now;
            }
            else if (MBRLiftingCtrl(GetPumpPort(mbr, lift2), 1) == GetPumpPort(mbr, lift1))
            {
                lift_state = MBR_PUMP_ST_MINOR_A;
            }
            break;
        default:
            lift_state = MBR_PUMP_ST_INIT;
            break;
    }
    if (IsMBRLiftingPumpOpen())
    {
        pump_state.pump_st.pump_lift = 1;
    }
    else
    {
        pump_state.pump_st.pump_lift = 0;
    }
}


void FlowMBR(void)
{
    MBRAerationPumpCtrl();
    MBRSubmersibleSewagePumpCtrl();
    MBRReflexPumpCtrl();
    MBRWaterMembranePumpCtrl();
    MBRSuckMembranePumpCtrl();
    MBRDosingPumpCtrl();
    MBRLiftingPumpCtrl();
}



