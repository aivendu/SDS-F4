#include "stdint.h"
#include "string.h"
#include "gprs.h"
#include "gprs_socket.h"
#include "queue.h"


/****  连接socket  ***********************************
** 函数名称:   GprsSocketConnect
** 功能描述:   连接socket, 在连之前需要确认socket是否被关闭。
** 输入:	   char *ip       --  服务器IP地址,格式x.x.x.x
**             uint16_t port  --  服务器端口号
** 输出:	  1: 连接成功
**            0: 正在执行命令
**           -1：连接失败
**           -2: 模块故障
**           -3: 系统忙
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/
int8_t GprsSocketConnect(char *ip, uint16_t port)
{
    if ((GetSIMOnlineGPRSState() == 0) || (GetSIMPowerGPRSState() == 0) ||
        (GetSIMRegisterGPRSState() == 0) || (GetSIMAttatchGPRSState() == 0))
    {
        return -2;
    }
    switch (socket.state)
    {
        case 1:  //  正在连接, 需要连接socket
            if (GetSIMSocketGPRSState() == 1)
            {
                socket.state = 2;
                return 1;
            }
            else
            {
                return 0;
            }
        case 2:  //  连接成功
            if (GetSIMSocketGPRSState() == 0)
            {
                strcpy(socket.socket_ip, ip);
                socket.socket_port = port;
                socket.state = 1;
                return 0;
            }
            else
            {
                return -3;
            }
        case 4:  //  连接失败
            socket.state = 0;
            return -1;
        default :
            strcpy(socket.socket_ip, ip);
            socket.socket_port = port;
            socket.state = 1;
            return 0;
    }
}

/****  关闭socket  ***********************************
** 函数名称:   GprsSocketClose
** 功能描述:   关闭socket
** 输入:	   无
** 输出:	  1：关闭成功
**            0: 正在关闭
**           -1：模块故障
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/
int8_t GprsSocketClose(void)
{
    if ((GetSIMOnlineGPRSState() == 0) || (GetSIMPowerGPRSState() == 0) ||
        (GetSIMRegisterGPRSState() == 0) || (GetSIMAttatchGPRSState() == 0))
    {
        return -1;
    }
    else if (GetSIMSocketGPRSState() == 1)
    {
        socket.state = 3;  //  断开socket
        return 0;
    }
    return 1;
}

/****  向socket发送数据  ***********************************
** 函数名称:   Gprssocket_send
** 功能描述:   向socket发送数据
** 输入:	   uint8_t *data  --  发送的数据
**             uint32_t len   --  发送的数据长度
** 输出:	 >0：发送成功的数据个数
**            0: 系统忙, 数据没有发送成功
**           -1: 发送失败, socket没有连上
**           -2：模块故障
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/
int32_t GprsSocketSend(uint8_t *data, uint32_t len)
{
    if ((GetSIMOnlineGPRSState() == 0) || (GetSIMPowerGPRSState() == 0) ||
        (GetSIMRegisterGPRSState() == 0) || (GetSIMAttatchGPRSState() == 0))
    {
        return -2;
    }
    else if (GetSIMSocketGPRSState() == 1)
    {
        return WriteTcpSendBuffer(data, len);
    }
    else
    {
        return -1;
    }
}


/****  读socket的数据  ***********************************
** 函数名称:   socket_read
** 功能描述:   读socket的数据
** 输入:	   uint8_t *data  --  接收数据时的缓存
**             uint32_t len   --  接收的数据的最大个数
** 输出:	>=0：接收成功的数据个数
**           -1: 接收失败, socket没有连上
**           -2：模块故障
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   杜其俊
** 日期:	   20160815
******************************************************************/
int32_t GprsSocketRead(uint8_t *data, uint32_t len_max)
{
    int32_t rec_len = 0;
    int32_t temp;
    if ((GetSIMOnlineGPRSState() == 0) || (GetSIMPowerGPRSState() == 0) ||
        (GetSIMRegisterGPRSState() == 0) || (GetSIMAttatchGPRSState() == 0))
    {
        return -2;
    }
    else if (GetSIMSocketGPRSState() == 1)
    {
        while ((temp = ReadTcpRecBuffer()) >= 0)
        {
            *data++ = (uint8_t)temp;
            if (++rec_len >= len_max)
            {
                break;
            }
        }
        return rec_len;
    }
    return -1;
}

int8_t GprsReturnSocketTCPState(void)
{
	return gprs_state.sim_gprs_run_state;
}


