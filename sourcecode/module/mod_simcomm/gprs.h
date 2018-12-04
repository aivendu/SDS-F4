#ifndef _GPRS_H
#define _GPRS_H
#include "stdint.h"


#define SIM_TIMER_TICK      5  //  定时器的tick时间, 单位ms

//GPRS状态
#define GPRS_OK					 1
#define GPRS_CONNECT_FAILED		-1 	//-1：连接失败
#define GPRS_MODULE_ERROR		-2	//-2: 模块故障
#define GPRS_SYSTEM_BUSY		-3	//-3: 系统忙

typedef enum s_at_state
{
	SIM_EXCEPTION    ,  //  异常状态, 需要重新初始化IO口
//	SIM_POWEROFF     ,  //  电源关闭状态, 需要重新上电
	SIM_ATCOMMAND    ,  //  AT命令模式空闲状态
	SIM_WAIT_RESPONSE,  //  等待AT命令回复
	SIM_CONNECT      ,  //
} s_at_state_t;
//extern s_at_state_t  sim_state;



typedef enum e_sim_c_type
{
	SIM_C_T_EXE  ,  //  执行命令
	SIM_C_T_READ ,  //  查询命令
	SIM_C_T_SET  ,  //  设置命令
	SIM_C_T_TEST ,  //  测试命令
	SIM_C_T_WAIT ,  //  等待下一条命令, 用于做等待超时
} e_sim_c_type_t;


typedef enum s_sim_ret
{
    SIMRetIdle           ,
    SIMRetPowerOff       ,
    SIMRetTIMEOUT        ,
    SIMRetUNKNOW         ,
    SIMRetNULLLine       ,
    SIMRetCommand        ,
    SIMRetOK             ,
    SIMRetError          ,
    SIMRetCME            ,
    SIMRetCMS            ,
    SIMRetConnect        ,
    SIMRetConnectOK      ,
    SIMRetRDY            ,
    SIMRetCallReady      ,
    SIMRetSMSReady       ,
    SIMRetALREADYCONNECT ,
    SIMRetConnectFail    ,
    SIMRetShutOk         ,
    SIMRetCloseOk        ,
	SIMRetState          ,
} s_sim_ret_t;

/****  sim模块处理任务  ***********************************
** 函数名称:   TaskSim
** 功能描述:   sim模块处理任务, 定时调用
** 输入:	   无
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/

extern void TaskSim(void *pdata);


/****  读取socket的透传数据  ***********************************
** 函数名称:   ReadTcpRecBuffer
** 功能描述:   读取socket的透传数据
** 输入:	   无
** 输出:	   -1 -- 无数据, >=0 有数据
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/

extern int32_t ReadTcpRecBuffer(void);


/****  发送socket的透传数据  ***********************************
** 函数名称:   WriteTcpSendBuffer
** 功能描述:   发送socket的透传数据
** 输入:	   无
** 输出:	   -1 -- 无数据, >=0 有数据
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/

extern int32_t WriteTcpSendBuffer(uint8_t *data, uint32_t len);

extern void ResetGPRSRunState(void);

typedef struct s_socket_cmd
{
    uint8_t  state;          //  命令执行状态  0--空闲状态, 1--正在连接, 2--连接成功, 3--正在关闭, 4--连接失败
    uint8_t  channel;        //  socket通道
    uint16_t socket_port;    //  socket的端口号
    char     socket_ip[32];  //  连接socket的
} s_socket_cmd_t;

extern s_socket_cmd_t socket;  //  socket操作接口变量


typedef struct s_gprs_state
{
    uint32_t sim_mode         : 1; 
    uint32_t sim_card         : 1;  
    uint32_t sim_power        : 1;  //  模块是否上电, 0--未上电, 1--已上电
    uint32_t sim_online       : 1;  //  模块是否在线, 是否可以和模块通讯, 0--不可以, 1--可以
    uint32_t sim_register     : 4;  //  模块是否在运营商网络上注册  0--否, 1--是
    uint32_t sim_attach       : 1;  //  GPRS是否附着在网络上  0--否, 1--是
    uint32_t sim_socket       : 2;  //  tcp的socket是否连上,   0--否, 1--是
    s_at_state_t sim_state    : 8;  //  sim模块工作模式
    uint32_t sim_debug        : 1;  //  通过串口调试sim模块
    uint8_t  sim_gprs_run_state;
    uint8_t  sim_attach_state;
    uint8_t  sim_pdp_state;
    uint8_t  sim_tcp_state;
} s_gprs_state_t;
extern s_gprs_state_t gprs_state;

#define sim_state gprs_state.sim_state

/*  读gprs状态 */
#define GetSIMPowerGPRSState()       (gprs_state.sim_power)
#define GetSIMOnlineGPRSState()      (gprs_state.sim_online)
#define GetSIMRegisterGPRSState()    (gprs_state.sim_register)
#define GetSIMAttatchGPRSState()     (gprs_state.sim_attach)
#define GetSIMSocketGPRSState()      (gprs_state.sim_socket)
#define GetSIMStateGPRSState()       (gprs_state.sim_state)
#define GetSIMDebugGPRSState()       (gprs_state.sim_debug)
#define GetSIMModeGPRSState()        (gprs_state.sim_mode)
#define GetSIMCardGPRSState()        (gprs_state.sim_card)

/*  写gprs状态 */
#define SetSIMPowerGPRSState(a)       (gprs_state.sim_power    = (a))
#define SetSIMOnlineGPRSState(a)      (gprs_state.sim_online   = (a))
#define SetSIMRegisterGPRSState(a)    (gprs_state.sim_register = (a))
#define SetSIMAttatchGPRSState(a)     (gprs_state.sim_attach   = (a))
#define SetSIMSocketGPRSState(a)      (gprs_state.sim_socket   = (a))
#define SetSIMStateGPRSState(a)       (gprs_state.sim_state    = (a))
#define SetSIMDebugGPRSState(a)       (gprs_state.sim_debug    = (a))
#define SetSIMModeGPRSState(a)        (gprs_state.sim_mode     = (a))
#define SetSIMCardGPRSState(a)        (gprs_state.sim_card     = (a))

#define IsSIMNetReady()          ((GetSIMOnlineGPRSState() == 1) && (GetSIMPowerGPRSState() == 1) &&\
                                  (GetSIMRegisterGPRSState() == 1) && (GetSIMAttatchGPRSState() == 1))
#define IsSIMSocketConnect()     ((GetSIMSocketGPRSState() == 1) && IsSIMNetReady())

#endif
