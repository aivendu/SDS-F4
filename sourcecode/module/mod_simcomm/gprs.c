#include "string.h"
#include "stdint.h"
#include "stdlib.h"
//#include "sim_at.h"
#include "gprs.h"
#include "queue.h"
//#include "my_debug.h"
#include "stdarg.h"
#include "sys_config.h"
#include "cj01_io_api.h"
#include "bsp_includes.h"

#define gprsDebug(format, ...)    {}
#define SIMIOSend(data, len)   IoWrite(MINI_PCIE, data, len)
int16_t  sim_buffer_index;    //  正在接收数据时接收索引,-1表示接收完成, -2表示接收超时，-3表示buffer接收满
char     sim_buffer[560];     //  接收的数据缓存器, 长度根据模块文档设置, 每一个命令行buffer最大556个字符
    
static uint32_t sim_delay_time;      //  接收定时器，单位1ms
static uint16_t sim_cmd_time;
static int32_t  sim_data_empty_time; //  当有数据正在发送时为负数, 大于0表示串口空闲时间
    

/****  接收处理AT命令  ***********************************
** 函数名称:   ReceiveCommand
** 功能描述:   处理AT命令的收.
** 输入:	   无
** 输出:	    =0 -- 正在接收数据
**              -1 -- 没有收到数据
**               2 -- 收到断开连接数据
** 全局变量:
** 调用模块:
** 备注:	   需要定时被调用
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
int32_t ReceiveCommand(void)
{
    uint8_t temp = 0;
	if (Ioctl(MINI_PCIE, MINIPCIE_WWAN_STATE) == 0)
	{
//        Printf_D("gprs", "Poweroff exp done!\r\n");
        return  -1;
	}
	if (sim_buffer_index >= 0)
	{
		while (IoRead(MINI_PCIE, &temp, 1) > 0)
		{
            //SetSimDataEmptyTime(1);
			sim_buffer[sim_buffer_index++] = temp;
            sim_cmd_time = 1;
			if (sim_buffer_index >= (sizeof(sim_buffer)-1))
			{
                sim_delay_time = 0;
				sim_buffer[sim_buffer_index] = 0;
				sim_buffer_index = -3; //  buffer存满了
				sim_cmd_time = 0;
				return 0;
			}
			if (temp == '\n')
			{
			    sim_buffer[sim_buffer_index] = 0;
				sim_buffer_index = -1;      //  一行数据接收完成
				sim_cmd_time = 0;
				return 0;
			}
//			IWDG_Feed();
		}
//		if(strstr(sim_buffer,"CLOSED") != NULL)
//		{   //在传输的过程中 出现断开连接
//			return 2;
//		}
        if(sim_cmd_time==0)
        {
            if (sim_delay_time == 0)
            {
                sim_buffer[sim_buffer_index] = 0;
                sim_buffer_index = -2;  //  接收超时
                sim_cmd_time = 0;
            }
        }
        else if (sim_cmd_time > 20)
        {
			sim_buffer[sim_buffer_index] = 0;
			sim_buffer_index = -1;  //  一行数据接收完成
			sim_cmd_time = 0;
        }
//        if (sim_delay_time == 0)
//        {
//            sim_buffer[sim_buffer_index] = 0;
//            sim_buffer_index = -2;  //  接收超时
//            sim_cmd_time = 0;
//        }
//        else if (sim_cmd_time > 20)
//        {
//			sim_buffer[sim_buffer_index] = 0;
//			sim_buffer_index = -1;  //  一行数据接收完成
//			sim_cmd_time = 0;
//        }
	}
    //Printf_D("rec", "%s - %d\r\n", sim_buffer, sim_buffer_index);
    return 0;
}

    
void SIMDelayTime(void)
{
    if (sim_data_empty_time < 0x7FFFFFF0)
    {
        sim_data_empty_time+=SIM_TIMER_TICK;
    }
    if ((sim_cmd_time) && (sim_cmd_time < 0xFFF0))
    {
        sim_cmd_time += SIM_TIMER_TICK;
    }
    if (sim_delay_time > SIM_TIMER_TICK)    sim_delay_time -= SIM_TIMER_TICK;
	else if (sim_delay_time)   sim_delay_time = 0;
}
    

void SetSimDataEmptyTime(uint32_t t)
{
    if (sim_data_empty_time > 0)
    {
        sim_data_empty_time = -t;
    }
    else
    {
        sim_data_empty_time-=t;
    }
}

/****  发送退出透传模式的命令  ***********************************
** 函数名称:   SendExitPassthroughCommand
** 功能描述:   发送退出透传模式的命令
** 输入:	   无
** 输出:	   0 -- 还未发送成功, 1--发送成功
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/

int8_t SendExitPassthroughCommand(void)
{
    if (sim_data_empty_time > 1100)  //  发送+++之前需要等待1S
    {
        SetSimDataEmptyTime(100);
        Ioctl(MINI_PCIE, MINIPCIE_SENDDATA_REALTIME, "+++", 3);
        //SIMIOSend("+++",3);
        gprsDebug("+++\r\n");
        //sim_data_empty_time = -1000;
        sim_delay_time = 6000;
        return 1;
    }
    return 0;
}
    
  
/****  向SOCKET发送透传数据  ***********************************
** 函数名称:   SendTcpData
** 功能描述:   向SOCKET发送透传数据
** 输入:	   uint8_t *data -- 发送的数据
**             uint32_t len  -- 数据的长度
** 输出:	   uint32_t -- 发送成功的数据长度
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
int32_t SendTcpData(uint8_t *data, uint32_t len)
{
    SetSimDataEmptyTime(len);
	SIMIOSend(data, len);
	return len;
}
  
    
/****  发送AT命令, 不等待回复  ***********************************
** 函数名称:   SendATCommand
** 功能描述:   处理AT命令的发和收.
** 输入:	   uint8_t type -- 命令类型,  SIM_C_T_EXE: 执行命令, SIM_C_T_QUERY:查询命令,
**                                        SIM_C_T_SET:设置命令,  SIM_C_T_TEST:测试命令
**             char * command --  命令
**			   char * arg  --  参数
**             uint32_t time -- 命令超时时间
** 输出:	 >=0：当前状态
**            -1：因为忙发送失败
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
int8_t SendATCommand(e_sim_c_type_t type, uint32_t time, char * command, char * arg)
{
    uint32_t command_len = 0, arg_len = 0;
    if (command)    command_len = strlen(command);
    if (arg)        arg_len     = strlen(arg);

    if (type == SIM_C_T_WAIT)
    {
        sim_delay_time = time;
        return 0;
    }
	if (sim_data_empty_time < 21)  //  AT 命令必须要在空闲10ms后才能发送
	{
		return -1;
	}
	SIMIOSend("AT",2);
	SIMIOSend(command, command_len);
    sim_delay_time = time;
	switch (type)
	{
		case SIM_C_T_EXE   :
			if (arg)  SIMIOSend(arg, arg_len);
	        SIMIOSend("\r\n", 2);
            gprsDebug("%u,AT%s%s\r\n", sim_delay_time, command, (arg==0)?"":arg);
			break;
		case SIM_C_T_READ :
			SIMIOSend("?\r\n",3);
            gprsDebug("%u,AT%s?\r\n", sim_delay_time, command);
			break;
		case SIM_C_T_SET   :
			SIMIOSend("=",1);
			if (arg) SIMIOSend(arg, strlen(arg));
	        SIMIOSend("\r\n", 2);
            gprsDebug("%u,AT%s=%s\r\n", sim_delay_time, command, (arg==0)?"":arg);
			break;
		case SIM_C_T_TEST  :
			SIMIOSend("=?\r\n",4);
            gprsDebug("%u,AT%s=?\r\n", sim_delay_time, command);
			break;
		default: break;
	}
    SetSimDataEmptyTime(5 + arg_len + command_len);
	return 0;
} 
    
    
    
    
#define GPRS_INIT        0    //  模块初始化
#define GPRS_POWERON     1    //  模块上电
#define GPRS_ATTACH      2    //  模块附着操作
#define GPRS_CONNECT     3    //  模块GPRS连接
#define GPRS_TCP         4    //  模块TCP连接
#define GPRS_POWEROFF    5    //  模块下电
#define GPRS_DEBUG       6    //  模块下电
#define GPRS_WAIT_RDY    7    //  等待模块启动

//static   uint8_t  gprs_run_state;       //  GPRS模块执行状态
#define  gprs_run_state gprs_state.sim_gprs_run_state

#define GPRSATTACH_RDY                  0    //  等待模块启动
#define GPRSATTACH_E0                   4    //  ATE0关闭回显
#define GPRSATTACH_CGREG                1    //  CGREG查询注册状态
#define GPRSATTACH_CGATT                2    //  查询附着状态
#define GPRSATTACH_CIPMUX               3    //  设置为单连接
#define GPRSAT_CSQ               		5    //  查询信号强度
#define GPRSATTACH_ENTER_AT             6    //  进入AT命令模式
#define GPRSATTACH_CGDCONT              7    //  设置PDP上下文
#define GPRSAT_CREG                     8    //  CREG
#define GPRSAT_CSMINS                   9


//static   uint8_t attach_state = 0;  //  attach执行状态
#define  attach_state gprs_state.sim_attach_state

#define  GPRSPDP_SHUT                   0  //  SHUT命令关闭移动场景
#define  GPRSPDP_CIPMODE                1  //  CIPMODE命令设置为数据透传模式
#define  GPRSPDP_CSTT                   2  //  CSTT启动任务
#define  GPRSPDP_CIICR                  3  //  CIICR激活移动场景
#define  GPRSPDP_CIFSR                  4  //  CIFSR获取本地IP地址
#define  GPRSPDP_CIPMUX                 5  //  CIPMUX设置IP单链接
#define  GPRSPDP_CDNSCFG                6  //  CDNSCFG配置DNS服务器地址
#define  GPRSPDP_CSOCKSETPN             7
#define  GPRSPDP_CIPMODE_TEST           8

//static   uint8_t gprs_state.pdp_state      //  激活pdp执行状态
#define  pdp_state gprs_state.sim_pdp_state


#define  GPRSTCP_IDLE                   0  //  TCP空闲状态
#define  GPRSTCP_DATA                   1  //  数据透传状态
#define  GPRSTCP_CLOSE                  2  //  关闭socket
#define  GPRSTCP_OPEN                   3  //  打开socket
#define  GPRSTCP_SHUT                   4  //  关闭移动场景
#define  GPRSTCP_STATUS                 5  //  查询TCP/IP连接状态
#define  GPRSTCP_EXIT_PASSTHROUGH       6  //  退出透传
#define  GPRSTCP_WAIT_CONNECT           7  //  等待连接socket
#define  GPRSTCP_GETIP_FROM_DNS         8  //  通过DNS获取IP
#define GPRSTCP_NETOPEN                9
#define GPRSTCP_GETIP                  10

#include  "sys_config.h"
#define  GprsTcpDns1()  sys_config_ram.local_dns1
#define  GprsTcpDns2()  sys_config_ram.local_dns2


// static uint8_t tcp_state;
#define  tcp_state gprs_state.sim_tcp_state//  tcp执行状态

static uint32_t tcp_buffer[150];    //  tcp透传接收数据buffer, 使用QUEUE操作, 容量要大于gprs模块的单个命令的大小参考

s_socket_cmd_t socket;      //  socket连接处理接口变量
s_gprs_state_t gprs_state;  //  gprs模块状态

extern uint8_t reconnect_flag;//socket 重连标志

static uint16_t sim_step_handle_time;

//s_at_state_t  sim_state = SIM_EXCEPTION;


/****  进入AT命令模式  ***********************************
** 函数名称:   EnterATMode
** 功能描述:   通过发送"+++"进入命令模式
** 输入:	   int8_t mode --  模式  0:非阻塞模式  1:阻塞模式, "发送+++"  2:查询是否进入阻塞模式
**
** 输出:	  >0：当前状态
**            -1：超时
**            -2: 进入失败
**            -3: 参数错误
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
static int8_t EnterATMode(int8_t mode)
{
	if (mode > 2)
	{
		return -3;
	}
	do {
        //Printf_D("gprs", "EnterAT: %u, buf:%s\r\n", sim_state, sim_buffer);
		switch (sim_state)
		{
            case SIM_CONNECT    :
                if (SendExitPassthroughCommand() == 1)
                {
                    sim_buffer_index = 0;
                    sim_state = SIM_WAIT_RESPONSE;
                }
				break;
			case SIM_WAIT_RESPONSE:
			    if ((sim_buffer_index < 0) && (strcmp(sim_buffer, "OK\r\n") == 0))    //  等待命令处理完
				{
                    //memset(sim_buffer, 0, sizeof(sim_buffer));
					sim_state = SIM_ATCOMMAND;  //  无论如何都进入AT命令模式
					mode = 0;
                    return 1;
				}
                else if (sim_buffer_index == -2)
                {
                    //SendExitPassthroughCommand();
                    return -1;
                }
				break;
			default :
				sim_state = SIM_CONNECT;
				mode = 0;  //  设置为非阻塞模式，以便退出函数
				break;
		}
	} while (mode); //  当模式为非阻塞时退出
    //Printf_D("gprs", "EnterAT: %d, buf:%s\r\n", sim_state, sim_buffer);
	return 0;
}

#if 0
/****  退出AT命令模式  ***********************************
** 函数名称:   ExitATMode
** 功能描述:   通过发送 EXIT_MODE_COMMAND 定义的命令 退出命令模式
** 输入:	   int8_t mode --  模式  0:非阻塞模式  1:阻塞模式  2:查询是否进入阻塞模式
**
** 输出:	 >=0：当前状态
**            -1：超时
**            -2: 退出失败
**            -3: 参数错误
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
static int8_t ExitATMode(int8_t mode)
{
	if (mode > 2)
	{
		return -3;
	}
	do {
        Printf_D("gprs", "EnterAT: %u, buf:%s\r\n", state, sim_buffer);
		switch (sim_state)
		{
			case SIM_CONNECT    :
                mode = 0;  //  设置为非阻塞模式，以便退出函数
				break;
			case SIM_ATCOMMAND  :
			    if (SendATCommand(SIM_C_T_EXE, "O", 0, 0) == 0)
			    {
    				sim_state = SIM_CONNECT;
                    sim_buffer_index = 0;
                    memset(sim_buffer, 0, sizeof(sim_buffer));
			    }
				break;

			case SIM_WAIT_RESPONSE :
				break;
			default :
				sim_state = SIM_EXCEPTION;
				mode = 1;  //  设置为非阻塞模式，一边退出函数
				break;
		}
	} while (mode <= 0);
    Printf_D("gprs", "EnterAT: %d, buf:%s\r\n", state, sim_buffer);
	return state;
}
#endif

static uint32_t ATResponseProccess(char *command)
{
    char *rec_temp = strtok(sim_buffer, "\r\n");
    if (rec_temp)
    {
        gprsDebug("%s\r\n", rec_temp);
        if (strcmp(rec_temp, "OK") == 0)
        {
            return SIMRetOK;
        }
        else if (strcmp(rec_temp, "ERROR") == 0)
        {
            return SIMRetError;
        }
        else if ((command) && (strncmp(rec_temp, command, strlen(command)) == 0))
        {
            return SIMRetCommand;
        }
        else if (strncmp(rec_temp, "+CME ERROR", 10) == 0)
        {
            return SIMRetCME;
        }
        else if (strncmp(rec_temp, "+CMS ERROR", 10) == 0)
        {
            return SIMRetCMS;
        }
        else if (strstr(rec_temp, "CONNECT") != 0)
        {
            return SIMRetConnect;
        }
        else if (strcmp(rec_temp, "ALREADY CONNECT") == 0)
        {
            return SIMRetALREADYCONNECT;
        }
        else if (strcmp(rec_temp, "CONNECT FAIL") == 0)
        {
            return SIMRetConnectFail;
        }
        else if (strcmp(rec_temp, "SHUT OK") == 0)
        {
            return SIMRetShutOk;
        }
        else if (strcmp(rec_temp, "CLOSE OK") == 0)
        {
            return SIMRetCloseOk;
        }
        else if (strcmp(rec_temp, "RDY") == 0)
        {
            return SIMRetRDY;
        }
        else if (strcmp(rec_temp, "Call Ready") == 0)
        {
            return SIMRetCallReady;
        }
		else if (strncmp(rec_temp, "STATE:", 6) == 0)
		{
			return SIMRetState;
		}		
        else
        {
            return SIMRetUNKNOW;
        }
    }
    return SIMRetNULLLine;
}

/****  处理AT命令的收发  ***********************************
** 函数名称:   ATCommandProccess
** 功能描述:   处理AT命令的发和收. 如果不处于AT命令模式将会进入AT命令模式
** 输入:	   e_sim_c_type_t type  --  命令类型
**             char * command --  命令
**			   char * arg  --  参数
**             char * ret  --  返回结果存储地址
**             uint32_t ret_len  --  返回的数据的结果
**             uint32_t timeout --  超时时间, 0表示非阻塞模式
** 输出:	  0：正在处理
**            1：超时
**            2: 非AT命令模式
**            3: 返回OK, 不带参数
**            4: 返回ERROR, 不带参数
**            5: 返回其他错误
**          >=256: 数据指针
** 全局变量:
** 调用模块:
** 备注:	   查询返回的信息存入ret数组中
** 作者:	   杜其俊
** 日期:	   20160403
******************************************************************/
static uint32_t ATCommandProccess(e_sim_c_type_t type, uint32_t timeout, char * command, char * arg, ...)
{
    uint32_t rec_temp;
    va_list  argv;
    char *   argv_buffer;
   // Printf_D("gprs", "EnterAT: %u, buf:%s\r\n", sim_state, sim_buffer);
	switch (sim_state)
	{
		case SIM_ATCOMMAND  :
            if (arg == 0)  //  如果没有参数
            {
                argv_buffer = 0;  //  传入下一层时也没有参数
            }
            else
            {
                if ((argv_buffer = malloc(1024)) == 0)  //  获取缓存的空间
                {
                    return 0;
                }
                //  生成参数
                va_start(argv, arg);
                vsprintf(argv_buffer, arg, argv);
                va_end(argv);
            }
            //  发送数据
            if (SendATCommand(type, timeout, command, argv_buffer) == 0)
            {
                sim_state = SIM_WAIT_RESPONSE;
    			if (sim_buffer_index < 0)       //  去掉没有处理的回复
    			{
                    sim_buffer_index = 0;
    			}
            }
            if (argv_buffer)  free(argv_buffer);  //  释放缓存空间
			break;

		case SIM_WAIT_RESPONSE       :
		    //  AT命令的回复处理
            if ((sim_buffer_index == -1) || (sim_buffer_index == -3)) //  接收到一行或者buffer已经被填满
            {
                if ((rec_temp = ATResponseProccess(command)) == SIMRetNULLLine)  //  空行不处理
                {
                    sim_buffer_index = 0;
                }
                else  //  有回复数据
                {
                    return rec_temp;
                }
            }
            else if (sim_buffer_index == -2)  //  接收超时
            {
                return SIMRetTIMEOUT;
            }
			break;
		default :
			sim_state = SIM_ATCOMMAND;
			break;
	}
	return 0;
}



/****  查询注册和附着  ***********************************
** 函数名称:   GPRSAttachProccess
** 功能描述:   查询注册和附着
** 输入:
** 输出:	 =0：正在处理
**            1: 可以进行下一步
**            2：返回上一步
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/
int8_t GPRSAttachProccess(void)
{
    uint32_t rec_buf;
    char *   proccess_p=0;
    switch (attach_state)
    {
	    case GPRSATTACH_RDY:
//		    if ((sim_buffer_index == -1) || (sim_buffer_index == -3)) //  接收到一行或者buffer已经被填满
//            {
//                if (ATResponseProccess(0) == SIMRetRDY)  //  空行不处理
//                {
//                    sim_state = SIM_ATCOMMAND;
//                    attach_state = GPRSATTACH_E0;
//                }
//            }
//            else if (sim_buffer_index == -2)  //  接收超时
//            {
//                SetSIMOnlineGPRSState(0);
//                sim_state = SIM_ATCOMMAND;  //  表示模块脱机
//			    return 2;
//            }
//		    break;
        case GPRSATTACH_E0:
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 5000, "E0", 0)) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    SetSIMOnlineGPRSState(1);
                    sim_state = SIM_ATCOMMAND;
                    attach_state = GPRSAT_CSQ;
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    SetSIMOnlineGPRSState(0);
                    sim_state = SIM_CONNECT;  //  表示模块脱机
                    attach_state = GPRSATTACH_ENTER_AT;
                    //return 2;
                }
                else  //  接收到其他命令不处理, 继续等待
                {
                   //sim_state = SIM_ATCOMMAND;
                }
            }
            break;
        case GPRSATTACH_ENTER_AT:
            if (EnterATMode(0) != 0)
            {
                attach_state = GPRSATTACH_E0;
                sim_state = SIM_ATCOMMAND;
            }
            break;
		case GPRSAT_CSQ:
			if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 2000, "+CSQ", 0)) != 0)//执行命令,查询信号强度
            {
                if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)//测试命令 回复 OK
                {
                    sim_state = SIM_ATCOMMAND;
                    attach_state = GPRSAT_CSMINS;
                }
            }
            break;
        case GPRSAT_CSMINS:
            if (sys_config_ram.com_mode == CM_4G)
            {
                attach_state = GPRSATTACH_CGDCONT;
                break;
//                rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CSIM", "1,242"); 
//                if (rec_buf != 0)
//                {
//                    if (rec_buf == SIMRetTIMEOUT)
//                    {
//                        sim_state = SIM_ATCOMMAND;
//                    }
//                    else if (rec_buf == SIMRetOK)//测试命令 回复 OK
//                    {
//                        sim_state = SIM_ATCOMMAND;
//                        //if (GetSIMCardGPRSState())   
//                        attach_state = GPRSATTACH_CGDCONT;
//                    }
//                }
//                break;
            }
            if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+CSMINS", 0)) != 0)//执行命令,查询信号强度
            {
                if (rec_buf == SIMRetCommand)
                {
                    strtok((char *)sim_buffer, ",");
                    SetSIMCardGPRSState(atoi(strtok(0, ",")));
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)//测试命令 回复 OK
                {
                    sim_state = SIM_ATCOMMAND;
                    if (GetSIMCardGPRSState())   attach_state = GPRSATTACH_CGDCONT;
                }
            }
            break;
        case GPRSATTACH_CGDCONT:
            if (sys_config_ram.com_mode == CM_4G)
            {
                rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CGSOCKCONT", 
                                 "1,\"IP\",\"%s\"",sys_config_ram.gprs_apn);
            }
            else 
            {
                rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CGDCONT", 
                                 "1,\"IP\",\"%s\"",sys_config_ram.gprs_apn);
            }
			if (rec_buf != 0)//执行命令,查询信号强度
                
            {
                if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)//测试命令 回复 OK
                {
                    sim_state = SIM_ATCOMMAND;
                    if (sys_config_ram.com_mode == CM_4G)
                    {
                        attach_state = GPRSAT_CREG;
                    }
                    else
                    {
                        attach_state = GPRSATTACH_CGREG;
                    }
                }
            }
            break;
		case GPRSAT_CREG:
            if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+CREG", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
                    strtok((char *)sim_buffer, ",");
                    SetSIMRegisterGPRSState(atoi(strtok(0, ",")));
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
					attach_state = GPRSATTACH_E0;
                }
                else if (rec_buf == SIMRetOK)
                {
                    if ((GetSIMRegisterGPRSState() == 1) || (GetSIMRegisterGPRSState() == 5))
                    {
                        attach_state = GPRSATTACH_CGREG;
                    }
                    sim_state = SIM_ATCOMMAND;
                }
            }
            break;
        case GPRSATTACH_CGREG:
            if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+CGREG", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
                    strtok((char *)sim_buffer, ",");
                    SetSIMRegisterGPRSState(atoi(strtok(0, ",")));
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)
                {
                    if ((GetSIMRegisterGPRSState() == 1) || (GetSIMRegisterGPRSState() == 5))
                    {
                        if (sys_config_ram.com_mode == CM_4G)
                        {
                            attach_state = GPRSATTACH_E0;
                            return 1;
                        }
                        else
                        {
                            attach_state = GPRSATTACH_CGATT;
                        }
						//attach_state = GPRSATTACH_CGATT;
                        sim_step_handle_time=0;  //  只要注册命令一直回复OK， 就不要重启模块
                        sim_state = SIM_ATCOMMAND;
                    }
                    else
                    {
                        sim_step_handle_time=0;  //  只要注册命令一直回复OK， 就不要重启模块
                        sim_state = SIM_ATCOMMAND;
                        //return 2;
                    }
                }
            }
            break;
        case GPRSATTACH_CGATT:
            if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+CGATT", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
                    proccess_p = sim_buffer;
					strtok(proccess_p, ":");
					proccess_p = strtok(0, ",");
					if (proccess_p)   SetSIMAttatchGPRSState(atoi(proccess_p));
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)
                {
                    sim_state = SIM_ATCOMMAND;
                    if (GetSIMAttatchGPRSState()  == 1)
                    {
                        attach_state = GPRSATTACH_E0;
                        return 1;
                    }
                }
                else if (rec_buf == SIMRetError)
                {
                    sim_state = SIM_ATCOMMAND;
                    attach_state = GPRSATTACH_CGREG;
                }
            }
            break;
        default :
            attach_state = GPRSATTACH_E0;
            sim_state = SIM_ATCOMMAND;
            break;
    }
    return 0;
}


/****  激活移动场景  ***********************************
** 函数名称:   GPRSActivePDPContext
** 功能描述:   激活移动场景
** 输入:
** 输出:	 =0：正在处理
**            1: 可以进行下一步
**            2：返回上一步
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/

int8_t GPRSActivePDPContext(void)
{
    uint32_t rec_buf;
    switch (pdp_state)
    {
        case GPRSPDP_SHUT:
            if (sys_config_ram.com_mode == CM_4G)
            {
                pdp_state = GPRSPDP_CSOCKSETPN;
                sim_state = SIM_ATCOMMAND;
                break;
            }
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 35000, "+CIPSHUT", 0)) != 0)
            {
                if (rec_buf == SIMRetShutOk)
                {
                    pdp_state = GPRSPDP_CIPMUX;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSPDP_CSOCKSETPN:
			if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CSOCKSETPN", "1")) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    pdp_state = GPRSPDP_CIPMODE_TEST;
                    sim_state = SIM_ATCOMMAND;
                }
            }
			break;
        case GPRSPDP_CIPMUX:
            if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CIPMUX", "0")) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    pdp_state = GPRSPDP_CIPMODE;
                }
                else if (rec_buf == SIMRetError)
                {
                    pdp_state = GPRSPDP_SHUT;
                }
                sim_state = SIM_ATCOMMAND;
            }

            break;
        case GPRSPDP_CIPMODE_TEST:
            if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+CIPMODE", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
                    SetSIMAttatchGPRSState(1);
                    if(strchr((char *)sim_buffer,'1'))  SetSIMModeGPRSState(1);					 
					else SetSIMModeGPRSState(0); 					  					
                }
                else if (rec_buf == SIMRetOK)
                {
                    sim_state = SIM_ATCOMMAND;
					if(GetSIMModeGPRSState() == 0)  pdp_state = GPRSPDP_CIPMODE;
					else {pdp_state = GPRSPDP_SHUT; return 1;}
                }
            }
            break;
        case GPRSPDP_CIPMODE:
            if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, "+CIPMODE", "1")) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    if (sys_config_ram.com_mode == CM_4G)
                    {
                        pdp_state = GPRSPDP_SHUT;
                        return 1;
                    }
                    else
                    {
                        pdp_state = GPRSPDP_CSTT;
                    }
                }
                else
                {
                    pdp_state = GPRSPDP_SHUT;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSPDP_CDNSCFG:
            if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 2000, 
                               "+CDNSCFG", "\"%s\",\"%s\"", GprsTcpDns1(), GprsTcpDns2())) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    pdp_state = GPRSPDP_CSTT;
                }
                else
                {
                    pdp_state = GPRSPDP_SHUT;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSPDP_CSTT:
            if ((strlen(sys_config_ram.gprs_apn) < 2) //  未配置APN, 则不设置
                // || (strlen(sys_config_ram.gprs_apn_username) < 2)//  未配置APN 用户名, 则不设置
                )
            {
                if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 5000, "+CSTT", 0)) != 0)
                {
                    if (rec_buf == SIMRetOK)
                    {
                        pdp_state = GPRSPDP_CIICR;
                    }
                    else
                    {
                        pdp_state = GPRSPDP_SHUT;
                    }
                    sim_state = SIM_ATCOMMAND;
                }
                break;
            }
            else
            {
                if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 5000, "+CSTT", "\"%s\",\"%s\",\"%s\"",
                                     sys_config_ram.gprs_apn, 
                                     sys_config_ram.gprs_apn_username,
                                     sys_config_ram.gprs_apn_passwd)) != 0)
                {
                    if (rec_buf == SIMRetOK)
                    {
                        pdp_state = GPRSPDP_CIICR;
                    }
                    else
                    {
                        pdp_state = GPRSPDP_SHUT;
                    }
                    sim_state = SIM_ATCOMMAND;
                }
            }
            break;
        case GPRSPDP_CIICR:
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 45000, "+CIICR", 0)) != 0)
            {
                if (rec_buf == SIMRetOK)
                {
                    pdp_state = GPRSPDP_CIFSR;
                }
                else
                {
                    pdp_state = GPRSPDP_SHUT;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSPDP_CIFSR:
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 2000, "+CIFSR", 0)) != 0)
            {
                if (rec_buf == SIMRetUNKNOW)
                {
                    pdp_state = GPRSPDP_SHUT;
                    return 1;
                }
                else 
                {
                    pdp_state = GPRSPDP_SHUT;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        default:
            pdp_state = GPRSPDP_SHUT;
            sim_state = SIM_ATCOMMAND;
            break;
    }
    return 0;
}


/****  tcp连接与断开逻辑处理  ***********************************
** 函数名称:   GPRSTcpProccess
** 功能描述:   TCP连接与断开逻辑处理
** 输入:
** 输出:	 =0：正在处理
**            1: 需要重新激活移动场景
**            2：需要重新查询注册状态
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/

int8_t GPRSTcpProccess(void)
{
    uint16_t i;
    uint32_t rec_buf = 0;
    switch (tcp_state)
    {
        case GPRSTCP_IDLE:  //  等待连接SOCKET请求
            if (socket.state == 1)
            {
                sim_state = SIM_ATCOMMAND;
                if (sys_config_ram.com_mode == CM_4G)
                {
                    tcp_state = GPRSTCP_NETOPEN;
                    break;
                }
                //  关闭socket
                if ((socket.socket_ip[0] >= '0') && (socket.socket_ip[0] <= '9'))
                {
                    tcp_state = GPRSTCP_OPEN;   //  已经是IP地址
                }
                else
                {
                    tcp_state = GPRSTCP_GETIP_FROM_DNS;
                }
            }
            break;
        case GPRSTCP_DATA:  //  数据模式
            if (socket.state == 3)
            {
                //  关闭socket
                SetSIMSocketGPRSState(0);
                tcp_state = GPRSTCP_EXIT_PASSTHROUGH;
            }
            else if (sim_buffer_index == -1)  //  透传命令处理
            {
                if (strncmp((char *)sim_buffer, "CLOSED", 6) == 0)
                {
                    gprsDebug("CLOSED\r\n");
                    if (socket.state == 1)
                    {
                        socket.state = 4;
                    }
                    SetSIMSocketGPRSState(0);
                    tcp_state = GPRSTCP_IDLE;
                    sim_state = SIM_ATCOMMAND;
                }
				else if(strncmp((char *)sim_buffer,"+PDP: DEACT",11) == 0)
				{
                    gprsDebug("+PDP: DEACT,disconnected by network\r\n");
//				    if (socket.state == 1) 
//                        socket.state = 4;
//                    }
                    
                    SetSIMAttatchGPRSState(0);//附着失败
//                    tcp_state = GPRSTCP_IDLE;
//                    sim_state = SIM_ATCOMMAND;
                    return 2;
				}
                else
                {
                    //  转发数据
                    for (i=0; i<strlen(sim_buffer); i++)
                        QueueWrite(tcp_buffer, sim_buffer[i]);
                }
            }
            else if ((sim_buffer_index == -2) || (sim_buffer_index == -3))  //  透传命令处理
            {
                //  转发数据
                for (i=0; i<strlen(sim_buffer); i++)
                    QueueWrite((uint8_t *)tcp_buffer, sim_buffer[i]);
            }
            //sim_state = SIM_CONNECT;
            break;
        case GPRSTCP_EXIT_PASSTHROUGH:
            if (EnterATMode(0) != 0)
            {
                tcp_state = GPRSTCP_CLOSE;
                sim_state = SIM_ATCOMMAND;
            }
            break;
		 case GPRSTCP_NETOPEN://启动Net网络
			 if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 2000, "+NETOPEN", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
//                    strtok((char *)sim_buffer, ":");
//                    SetSIMAttatchGPRSState(atoi(strtok(0, ":")));
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetError)
                {
					tcp_state = GPRSTCP_CLOSE;
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)
                {
                    SetSIMAttatchGPRSState(1);
					tcp_state = GPRSTCP_GETIP;
                    sim_state = SIM_ATCOMMAND;
                }
            }
            break;
             
		case GPRSTCP_GETIP://获取IP地址
			 if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 2000, "+IPADDR", 0)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {

                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetOK)
                {
					tcp_state = GPRSTCP_OPEN;
                    sim_state = SIM_ATCOMMAND;
                }
            }
            break;
		
        case GPRSTCP_CLOSE:  //  关闭socket
            if (sys_config_ram.com_mode == CM_4G)
            {
                rec_buf = ATCommandProccess(SIM_C_T_SET, 10000, "+CIPCLOSE", "0");
            }
            else
            {
                rec_buf = ATCommandProccess(SIM_C_T_EXE, 10000, "+CIPCLOSE", 0);
            }
            if (rec_buf != 0)
            {
                if ((rec_buf == SIMRetCloseOk) || (rec_buf == SIMRetError))
                {
                    SetSIMSocketGPRSState(0);
                    tcp_state = GPRSTCP_OPEN;
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                    return 2;
                }
            }
            break;
        case GPRSTCP_GETIP_FROM_DNS:
            if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 10000, "+CDNSGIP", socket.socket_ip)) != 0)
            {
                if (rec_buf == SIMRetCommand)
                {
                    strtok(sim_buffer, ":");
                    if (atoi(strtok(0, ",")) == 1)
                    {
                        strtok(0, ",");
                        strncpy(socket.socket_ip, strtok(0, ","), sizeof(socket.socket_ip));
                        i = 0;
                        while (((socket.socket_ip[i] < '0') || (socket.socket_ip[i] > '9')) && 
                               (sizeof(socket.socket_ip) > i)) 
                            i++;
                        memcpy(socket.socket_ip, &socket.socket_ip[i], sizeof(socket.socket_ip) - i);
                        i = 0;
                        while ((sizeof(socket.socket_ip) > i) &&  (socket.socket_ip[i] != '\"')) i++;
                        socket.socket_ip[i] = 0;
                    }
                    tcp_state = GPRSTCP_OPEN;
                }
                else if (rec_buf == SIMRetOK)
                {
                    break;
                }
                else if (rec_buf == SIMRetError)
                {
                    tcp_state = GPRSTCP_IDLE;
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    tcp_state = GPRSTCP_STATUS;
                    //return 2;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSTCP_OPEN:  //  打开socket
        {
            char arg[32];
            if (sys_config_ram.com_mode == CM_4G)
            {
                sprintf(arg, "0,\"TCP\",\"%s\",%u", socket.socket_ip, socket.socket_port);
                if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 10000, "+CIPOPEN", arg)) != 0)
                {
                    if ((rec_buf == SIMRetALREADYCONNECT) || (rec_buf == SIMRetOK))
                    {
                        SendATCommand(SIM_C_T_WAIT, 120000, 0, 0);
                        tcp_state = GPRSTCP_WAIT_CONNECT;
                    }
                    else if (rec_buf == SIMRetError)
                    {
    					tcp_state = GPRSTCP_STATUS;
                    }
                    else if (rec_buf == SIMRetConnectFail)
                    {
    					tcp_state = GPRSTCP_STATUS;
                    }
                    else if (rec_buf == SIMRetTIMEOUT)
                    {
    					tcp_state = GPRSTCP_STATUS;
                    }
                    else if (rec_buf == SIMRetConnect)
                    {
                        SetSIMSocketGPRSState(1);
                        sim_state = SIM_ATCOMMAND;
                        tcp_state = GPRSTCP_DATA;
                    }
                }
                break;
            }
            
            sprintf(arg, "\"TCP\",\"%s\",\"%u\"", socket.socket_ip, socket.socket_port);
            if ((rec_buf = ATCommandProccess(SIM_C_T_SET, 10000, "+CIPSTART", arg)) != 0)
            {
                if ((rec_buf == SIMRetALREADYCONNECT) || (rec_buf == SIMRetOK))
                {
                    SendATCommand(SIM_C_T_WAIT, 120000, 0, 0);
                    tcp_state = GPRSTCP_WAIT_CONNECT;
                }
                else if (rec_buf == SIMRetError)
                {
                    socket.state = 4;
                    tcp_state = GPRSTCP_SHUT;
                }
                else if (rec_buf == SIMRetConnectFail)
                {
                    socket.state = 4;
                    tcp_state = GPRSTCP_IDLE;
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetConnect)
                {
                    SetSIMSocketGPRSState(1);
                    sim_state = SIM_CONNECT;
                    tcp_state = GPRSTCP_DATA;
                    SendATCommand(SIM_C_T_WAIT, 1, 0, 0);
                }
            }
            break;
        }
        case GPRSTCP_WAIT_CONNECT:
            if (socket.state == 3)
            {
                //  关闭socket
                tcp_state = GPRSTCP_EXIT_PASSTHROUGH;
            }
            else if ((sim_buffer_index == -1) || (sim_buffer_index == -3)) //  接收到一行或者buffer已经被填满
            {
                if ((rec_buf = ATResponseProccess(0)) == SIMRetNULLLine)  //  空行不处理
                {
                }
                else if (rec_buf == SIMRetConnect)
                {
                    SetSIMSocketGPRSState(1);
                    sim_state = SIM_CONNECT;
                    tcp_state = GPRSTCP_DATA;
                    SendATCommand(SIM_C_T_WAIT, 1, 0, 0);
                }
                else if (rec_buf == SIMRetConnectFail)
                {
                    socket.state = 4;
                    tcp_state = GPRSTCP_IDLE;
                }
            }
            else if (sim_buffer_index == -2)
            {
                socket.state = 4;
                tcp_state = GPRSTCP_EXIT_PASSTHROUGH;
            }
            break;
        case GPRSTCP_SHUT:
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 5000, "+CIPSHUT", 0)) != 0)
            {
                if ((uint32_t)rec_buf == SIMRetShutOk)
                {
                    tcp_state = GPRSTCP_STATUS;
                }
                sim_state = SIM_ATCOMMAND;
            }
            break;
        case GPRSTCP_STATUS:
            if (sys_config_ram.com_mode == CM_4G)
            {
                if ((rec_buf = ATCommandProccess(SIM_C_T_READ, 2000, "+NETOPEN", 0)) != 0)
                {
                    if (rec_buf == SIMRetCommand)
                    {
                        strtok((char *)sim_buffer, ":");
                        SetSIMAttatchGPRSState(atoi(strtok(0, ":")));
                    }
                    else if (rec_buf == SIMRetTIMEOUT)
                    {
                        SetSIMSocketGPRSState(0);
                        tcp_state = GPRSTCP_EXIT_PASSTHROUGH;
                        sim_state = SIM_ATCOMMAND;
                    }
                    else if (rec_buf == SIMRetOK)
                    {
                        if (GetSIMAttatchGPRSState() == 1)
                        {
                            tcp_state = GPRSTCP_CLOSE;
                        }
    					else
    					{
    						 tcp_state = GPRSTCP_NETOPEN;
    					}
                        sim_state = SIM_ATCOMMAND;
                    }
                }
                break;
            }
            if ((rec_buf = ATCommandProccess(SIM_C_T_EXE, 2000, "+CIPSTATUS", 0)) != 0)
            {
                if (rec_buf == SIMRetState)
                {
                    if (strncmp((char *)&sim_buffer[7], "PDP DEACT", 9) == 0)//场景被释放
                    {
                        return 2;
                    }
                    else if ((strncmp((char *)&sim_buffer[7], "IP START", 8) == 0) ||//获得本地IP地址
                             (strncmp((char *)&sim_buffer[7], "IP CONFIG", 9) == 0) ||//配置场景
                             (strncmp((char *)&sim_buffer[7], "IP GPRSACT", 10) == 0))//接受场景
                    {
                        return 1;
                    }
                    else if ((strncmp((char *)&sim_buffer[7], "IP INITIAL", 10) == 0) ||//初始化
                             (strncmp((char *)&sim_buffer[7], "IP STATUS", 9) == 0))//启动任务
                    {
                        if (socket.state == 1)
                        {
                            if ((socket.socket_ip[0] >= '0') && (socket.socket_ip[0] <= '9'))
                            {
                                tcp_state = GPRSTCP_OPEN;   //  已经是IP地址
                            }
                            else
                            {
                                tcp_state = GPRSTCP_GETIP_FROM_DNS;
                            }
                        }
                        else
                        {
                            tcp_state = GPRSTCP_IDLE;
                        }
                    }
                    else
                    {
                        tcp_state = GPRSTCP_SHUT;
                    }
                    sim_state = SIM_ATCOMMAND;
                }
                else if (rec_buf == SIMRetError)
                {
                    return 2;
                }
                else if (rec_buf == SIMRetTIMEOUT)
                {
                    SetSIMSocketGPRSState(0);
                    tcp_state = GPRSTCP_EXIT_PASSTHROUGH;
                    sim_state = SIM_ATCOMMAND;
                }
            }
            break;
        default: tcp_state = GPRSTCP_STATUS; return 1;
    }
    return 0;
}


void ResetGPRS(void)
{
    s_gprs_state_t kk = gprs_state;
    attach_state = GPRSATTACH_E0;
    pdp_state = GPRSPDP_SHUT;
    tcp_state = GPRSTCP_IDLE;
    sim_state = SIM_ATCOMMAND;
    memset(&gprs_state, 0, sizeof(gprs_state));
    gprs_state.sim_debug = kk.sim_debug;
    gprs_run_state = kk.sim_gprs_run_state;
}


//  为debug做的发送数据的函数
void SendDebugData(void)
{
    uint32_t i;
    for (i=0; i<strlen(sim_buffer); i++)
        QueueWrite(tcp_buffer, sim_buffer[i]);
}

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

void SimHandler(void)
{
    int8_t state = 0;
    
    if (GetSIMDebugGPRSState())
    {
        gprs_run_state = GPRS_DEBUG;
    }
    SIMDelayTime();  //  定时器处理
    if (ReceiveCommand() == -1)
    {
        if ((gprs_run_state != GPRS_POWERON) && (gprs_run_state != GPRS_DEBUG))
        {
            ResetGPRS();
        }
    }
//	else if(ReceiveCommand() == 2)
//	{//断开自动重连
//		gprs_run_state = GPRS_ATTACH;
//		attach_state = 0;
//		reconnect_flag = 1;
//	}
    switch (gprs_run_state)
    {
        case GPRS_DEBUG:
            if (GetSIMDebugGPRSState() == 0)
            {
                ResetGPRS();
            }
            else if (sim_buffer_index < 0)
            {
                SendDebugData();
            }
            break;
        case GPRS_INIT:  //
            //GPRS_IOInit();
            QueueCreate(tcp_buffer, sizeof(tcp_buffer), 0, 0);
            gprs_run_state = GPRS_POWERON;
        case GPRS_POWERON:
            state = Ioctl(MINI_PCIE, MINIPCIE_POWRE_ON);
            if (state == 1)
            {
                SetSIMPowerGPRSState(1);
                sim_state = SIM_ATCOMMAND;
                gprs_run_state = GPRS_ATTACH;
                sim_step_handle_time=0;
            }
            break;
        // case GPRS_WAIT_RDY:
            // if (sim_buffer_index == -2)
            // {
                // attach_state = GPRSATTACH_E0;
                // sim_state = SIM_ATCOMMAND;
                // gprs_run_state = GPRS_ATTACH;
            // }
            // else if ((sim_buffer_index < 0) && (ATResponseProccess(0) == SIMRetRDY))
            // {
                // SetSIMPowerGPRSState(1);
                // attach_state = GPRSATTACH_E0;
                // sim_state = SIM_ATCOMMAND;
                // gprs_run_state = GPRS_ATTACH;
            // }
            // break;
        case GPRS_POWEROFF:
            if (Ioctl(MINI_PCIE, MINIPCIE_POWRE_OFF) == 1)
            {
                ResetGPRS();
                sim_state = SIM_ATCOMMAND;
                gprs_run_state = GPRS_POWERON;
            }
            break;
        case GPRS_ATTACH:   //  模块附着操作
            if(sim_step_handle_time>(200*30))
            {
                sim_state = SIM_ATCOMMAND;  //  表示模块脱机
                sim_step_handle_time=0;
                gprs_run_state = GPRS_POWEROFF;
            }
            else
            {
                sim_step_handle_time++;
            }
            state = GPRSAttachProccess();
            if (state == 1)
            {
                sim_step_handle_time=0;
                pdp_state = GPRSPDP_SHUT;
                gprs_run_state = GPRS_CONNECT;
            }
            else if (state == 2)
            {
                gprs_run_state = GPRS_POWEROFF;
            }
            else if (GetSIMPowerGPRSState() == 0)
            {
                gprs_run_state = GPRS_POWERON;
            }

            break;
        case GPRS_CONNECT:  //  模块GPRS连接
            if(sim_step_handle_time>(200*20))
            {
                sim_state = SIM_ATCOMMAND;  //  表示模块脱机
                sim_step_handle_time=0;
                gprs_run_state = GPRS_POWEROFF;
            }
            else
            {
                sim_step_handle_time++;
            }
            state = GPRSActivePDPContext();
            if (state == 1)
            {
                sim_step_handle_time=0;
                tcp_state = GPRSTCP_IDLE;
                gprs_run_state = GPRS_TCP;
            }
            else if (GetSIMPowerGPRSState() == 0)
            {
                gprs_run_state = GPRS_POWERON;
            }
            break;
        case GPRS_TCP:  //  模块TCP连接
            state = GPRSTcpProccess();
            if (state == 1)
            {
                sim_step_handle_time=0;
                gprs_run_state = GPRS_CONNECT;
            }
            else if (state == 2)
            {
                sim_step_handle_time=0;
                gprs_run_state = GPRS_ATTACH;
            }
            else if (GetSIMPowerGPRSState() == 0)
            {
                SetSIMSocketGPRSState(0);
                tcp_state = GPRSTCP_STATUS;
                gprs_run_state = GPRS_POWERON;
            }
            break;
        default :
            gprs_run_state = GPRS_ATTACH;
            break;
    }
    if (sim_buffer_index < 0)
    {
        sim_buffer_index = 0;  //  处理完成后，处理下一行
    }
}


const s_UartStr_t sim_uart = {115200, 8,0,1};
void TaskSim(void *pdata)
{
    pdata = pdata;
    IoOpen(MINI_PCIE, &sim_uart, sizeof(s_UartStr_t));
    while (1)
    {
        OSTimeDly(OS_TICKS_PER_SEC/200);
        SimHandler();
    }
}
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

int32_t ReadTcpRecBuffer(void)
{
    uint8_t temp;
    if (QueueRead(&temp, (uint8_t *)tcp_buffer) == QUEUE_OK)
    {
        return temp;
    }
    return -1;
}

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

int32_t WriteTcpSendBuffer(uint8_t *data, uint32_t len)
{
    return SendTcpData(data, len);
}
/*GPRS 运行状态重置*/
void ResetGPRSRunState()
{
	gprs_run_state = GPRS_INIT;
}


