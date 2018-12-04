/***********************************************************
** Copyright (c) 2015, 	 江苏慧乔信息科技有限公司
** All rights reserved.
** 文件名称： modbus_master.h
** 摘要：     modbus协议，服务器实现，基于<Modbus_Application_Protocol_V1_1b3.pdf>
** 当前版本： 1.0, aiven，20150528，创建
***********************************************************/

#include "modbus_master.h"
#include "spi_uart.h"
#include "modbus_crc.h"
#include "string.h"
#include "stdarg.h"
#include "debug.h"
#include "modbus_core.h"


uint32_t modbus_master_timer;
#define DebugPrintf(a,...)   
#define DebugPrintf_1(a,...)
//  输入接口
#define ModbusReadBus(a)   SpiUart2Read(0,a,1)
#define ModbusWriteBus(dat,len)  SpiUart2Write(0,dat,len)


uint32_t modbus_idle_timer;  //  总线空闲时间,  单位ms

static s_modbus_master_rec_t modbus_rec;  //  modbus 命令接收状态
//static s_modbus_master_send_t modbus_send; //  modbus 命令发送状态


//  主机发送命令格式数据
const s_support_code_t master_command_code[SUPPORT_CODE_NUMBER] =
{
    {0x01, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Coils
    {0x02, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Discrete Inputs
    {0x03, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Holding Registers
    {0x04, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Input Register
    {0x05, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Write Single Coil
    {0x06, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Write Single Register
    {0x07, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Exception status
    {0x08, 0x02, 0x00, 0x00, 0x00,  1,  NULL,  NULL},  //  Diagnostic
    {0x0B, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Get Com event counter
    {0x0C, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Get Com Event Log
    {0x0F, 0x05, 0x04, 0x01, 0x01,  0,  NULL,  NULL},  //  Write Multiple Coils
    {0x10, 0x05, 0x04, 0x01, 0x01,  0,  NULL,  NULL},  //  Write Multiple Registers
    {0x11, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Report Server ID
    {0x14, 0x01, 0x00, 0x01, 0x01,  0,  NULL,  NULL},  //  Read File record
    {0x15, 0x01, 0x00, 0x01, 0x01,  0,  NULL,  NULL},  //  Write File record
    {0x16, 0x06, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Mask Write Register
    {0x17, 0x09, 0x08, 0x01, 0x01,  0,  NULL,  NULL},  //  Read/Write Multiple Registers
    {0x18, 0x02, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read FIFO queue
    {0x2B, 0x03, 0x00, 0x00, 0x00,  1,  NULL,  NULL}   //  Read device Identification
};

//  回复命令格式数据
const s_support_code_t slave_command_code[SUPPORT_CODE_NUMBER] =
{
    {0x01, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 0  Read Coils
    {0x02, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 1 Read Discrete Inputs
    {0x03, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 2 Read Holding Registers
    {0x04, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 3 Read Input Register
    {0x05, 0x04, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 4 Write Single Coil
    {0x06, 0x04, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 5 Write Single Register
    {0x07, 0x01, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 6 Read Exception status
    {0x08, 0x02, 0x00, 0x00,  0x00, 1,  NULL,  NULL},  // 7 Diagnostic
    {0x0B, 0x04, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 8 Get Com event counter
    {0x0C, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 9 Get Com Event Log
    {0x0F, 0x04, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 10 Write Multiple Coils
    {0x10, 0x04, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 11 Write Multiple Registers
    {0x11, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 12 Report Server ID
    {0x14, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 13 Read File record
    {0x15, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 14 Write File record
    {0x16, 0x06, 0x00, 0x00,  0x00, 0,  NULL,  NULL},  // 15 Mask Write Register
    {0x17, 0x01, 0x00, 0x01,  0x01, 0,  NULL,  NULL},  // 16 Read/Write Multiple Registers
    {0x18, 0x04, 0x02, 0x02,  0x02, 0,  NULL,  NULL},  // 17 Read FIFO queue
    {0x2B, 0x02, 0x00, 0x00,  0x00, 1,  NULL,  NULL}   // 18 Read device Identification
};

/*  modbus 命令在s_support_code_t 表中的索引*/
const uint8_t modbus_code_index[]=
{
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7, 0xFF, 0xFF,    8,    9, 0xFF, 0xFF,   10,
      11,   12, 0xFF, 0xFF,   13,   14,   15,   16,   17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   18, 0xFF, 0xFF, 0xFF, 0xFF,
};


/******************************************************************
** 函数名称:   ModbusMasterRec
** 功能描述:   接收modbus 数据，按帧处理，
** 输入:
**
** 输出:	   0 -- 读成功； 其他返回MODBUS错误码
** 全局变量:   modbus_rec  --  当前总线数据接收状态
**                           modbus_code_index,  slave_command_code -- modbus 数据格式
** 调用模块:   ModbusReadBus  --  读取总线上的数据, 返回0表示没有接收到数据,  返回1表示接收到数据
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150604
******************************************************************/
void * ModbusMasterSend(uint8_t dev, uint8_t code, va_list argv)
{
	uint8_t *send_buffer = 0;
	uint8_t *data;
	uint16_t len;
	uint16_t addr;
	uint16_t crc = 0xFFFF;

	switch (code)
	{
	case MODBUS_CODE_READ_DISCRETE_INPUTS			:
		break;
	case MODBUS_CODE_READ_COILS						:
		break;
	case MODBUS_CODE_WRITE_SINGLE_COIL				:
		break;
	case MODBUS_CODE_WRITE_MULTIPLE_COILS			:
		break;
	case MODBUS_CODE_READ_INPUT_REGISTER 			:
		break;
	case MODBUS_CODE_READ_HOLDING_REGISTERS			:
		addr = va_arg(argv, uint32_t);
        data = va_arg(argv, uint8_t *);
		len  = va_arg(argv, uint32_t);
		send_buffer = malloc(master_command_code[modbus_code_index[code]].length + len + 2);
		if (send_buffer)
		{
			DebugPrintf(__FILE__, __LINE__, "malloc", "(%u):0x%X - ModbusMasterSend-1\r\n",
				master_command_code[modbus_code_index[code]].length + len + 2,
				(uint32_t)send_buffer);
			send_buffer+=2;
		    send_buffer[0] = dev;
		    send_buffer[1] = code;
		    send_buffer[2] = addr >> 8;
			send_buffer[3] = addr & 0xFF;
			send_buffer[4] = 0;
	        send_buffer[5] = len & 0xFF;
			len =  6;
			crc = CRC16(crc, send_buffer, len);
			send_buffer[len++] = (crc) & 0xFF;
			send_buffer[len++] = (crc >> 8) & 0xFF;
			send_buffer-=2;
			send_buffer[0] = len;
			send_buffer[1] = len >> 8;
		}
		else
		{
			DebugPrintf(__FILE__, __LINE__, "sys_busy", "\r\n");
		}
		break;
	case MODBUS_CODE_WRITE_SINGLE_REGISTER			:
		//dev  = va_arg(argv, uint32_t);
		addr = va_arg(argv, uint32_t);
        data = va_arg(argv, uint8_t *);
		len  = va_arg(argv, uint32_t);
		send_buffer = malloc(master_command_code[modbus_code_index[code]].length + len + 2);
		if (send_buffer)
		{
			DebugPrintf(__FILE__, __LINE__, "malloc", "(%u):0x%X - ModbusMasterSend-2\r\n",
				master_command_code[modbus_code_index[code]].length + len + 2,
				(uint32_t)send_buffer);
			send_buffer+=2;
		    send_buffer[0] = dev;
		    send_buffer[1] = code;
		    send_buffer[2] = addr >> 8;
			send_buffer[3] = addr & 0xFF;
			send_buffer[4] = data[1];
	        send_buffer[5] = data[0];
			len =  6;
			crc = CRC16(crc, send_buffer, len);
			send_buffer[len++] = (crc) & 0xFF;
			send_buffer[len++] = (crc >> 8) & 0xFF;
			send_buffer-=2;
			send_buffer[0] = len;
			send_buffer[1] = len >> 8;
		}
		else
		{
			DebugPrintf(__FILE__, __LINE__, "sys_busy", "\r\n");
		}
		break;
	case MODBUS_CODE_WRITE_MULTIPLE_REGISTERS		:
		//dev  = va_arg(argv, uint32_t);
		addr = va_arg(argv, uint32_t);
        data = va_arg(argv, uint8_t *);
		len  = va_arg(argv, uint32_t);
		send_buffer = malloc(master_command_code[modbus_code_index[code]].length + len + 2);
		if (send_buffer)
		{
			DebugPrintf(__FILE__, __LINE__, "malloc", "(%u):0x%X - ModbusMasterSend-3\r\n",
				master_command_code[modbus_code_index[code]].length + len + 2,
				(uint32_t)send_buffer);
			send_buffer+=2;
		    send_buffer[0] = dev;
		    send_buffer[1] = code;
		    send_buffer[2] = addr >> 8;
			send_buffer[3] = addr & 0xFF;
			send_buffer[4] = 0;
	        send_buffer[5] = len/ 2;
			send_buffer[6] = len;
			//memcpy(send_buffer, data, len);
            HalfWordBigEndianCopy(&send_buffer[7], data, len);
			len +=  7;
			crc = CRC16(crc, send_buffer, len);
			send_buffer[len++] = (crc) & 0xFF;
			send_buffer[len++] = (crc >> 8) & 0xFF;
			send_buffer-=2;
			send_buffer[0] = len;
			send_buffer[1] = len >> 8;
		}
		else
		{
			DebugPrintf(__FILE__, __LINE__, "sys_busy", "\r\n");
		}
		break;
	case MODBUS_CODE_READ_WRITE_MULTIPLE_REGISTERS	:
		break;
	case MODBUS_CODE_MASK_WRITE_REGISTER 			:
		break;
	case MODBUS_CODE_READ_FIFO_QUEUE 				:
		break;
	case MODBUS_CODE_READ_FILE_RECORD				:
		break;
	case MODBUS_CODE_WRITE_FILE_RECORD				:
		break;
	case MODBUS_CODE_READ_EXCEPTION_STATUS			:
		break;
	case MODBUS_CODE_DIAGNOSTIC						:
		break;
	case MODBUS_CODE_GET_COM_EVENT_COUNTER			:
		break;
	case MODBUS_CODE_GET_COM_EVENT_LOG				:
		break;
	case MODBUS_CODE_REPORT_SERVER_ID				:
		break;
	case MODBUS_CODE_READ_DEVICE_IDENTIFICATION		:
		break;
	default:
		len = 0;
		break;
	}
    return send_buffer;
}

/******************************************************************
** 函数名称:   ModbusMasterRec
** 功能描述:   接收modbus 数据，按帧处理，
** 输入:
**
** 输出:	   0 -- 读成功； 其他返回MODBUS错误码
** 全局变量:   modbus_rec  --  当前总线数据接收状态
**                           modbus_code_index,  slave_command_code -- modbus 数据格式
** 调用模块:   ModbusReadBus  --  读取总线上的数据, 返回0表示没有接收到数据,  返回1表示接收到数据
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150604
******************************************************************/
int8_t ModbusMasterRec(uint8_t dev, uint8_t code)
{
    uint8_t  temp;
	uint8_t  index = 0;
	uint16_t crc = 0;
	uint32_t len   = 0;
    //  接收数据
    while (ModbusReadBus(&temp))  //  处理所有数据
    {
	    //  处理数据
        if (modbus_rec.rec_len == 0)
        {
			if (temp != dev)
			{
				return 0;
			}

			modbus_rec.buffer[modbus_rec.rec_len] = temp;
        }
		else if (modbus_rec.rec_len == 1)  //  开始两个字节是地址和命令码
	    {
			if (temp != code)
			{
                modbus_rec.rec_len = 0;
				return 0;
			}
	        //  数据存入buffer 中
	        modbus_rec.buffer[modbus_rec.rec_len] = temp;
		    modbus_rec.dat = 0;
		    modbus_rec.dat_len = 0;
	    }
		else if (modbus_rec.buffer[1] & 0x80)
		{
			modbus_rec.buffer[modbus_rec.rec_len] = temp;
			if (modbus_rec.rec_len >= 4)
			{
				crc = 0xFFFF;
				crc = CRC16(crc, modbus_rec.buffer, 3);
				modbus_rec.crc = (modbus_rec.buffer[4] << 8) + modbus_rec.buffer[3];

		        if (crc == modbus_rec.crc)
		        {
	                modbus_rec.errcode = 0;
		        }
		        else
		        {
		            modbus_rec.errcode = MODBUS_ERRORCODE_CRC;
		        }
				return 1;
			}
		}
	    else
	    {
	        //   已经接收了命令码, 取命令的索引
	        index = modbus_code_index[modbus_rec.buffer[1]];
		    if (index >= SUPPORT_CODE_NUMBER)
		    {
		        modbus_rec.errcode = MODBUS_ERRORCODE_ILLEGAL_FUNCTION;
				return -1;
		    }
		    if (modbus_rec.rec_len < (slave_command_code[index].length + 2))  //  实际数据之前的字节
		    {
		        //  数据存入buffer 中
		        modbus_rec.buffer[modbus_rec.rec_len] = temp;
		    }
	        else if ((slave_command_code[index].length_type)  //  是否有数据需要接收
	            && (modbus_rec.dat == 0))    //  检测是否有数据, 如有是否有数据存储空间
	        {
		        //  计算数据长度
		        if (slave_command_code[index].length_type == 1)
		        {
		            len = modbus_rec.buffer[slave_command_code[index].length_index + 2];
		        }
		        else if (slave_command_code[index].length_type == 2)
		        {
		            len  = modbus_rec.buffer[slave_command_code[index].length_index + 2] << 8;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 3];
		        }
		        else if (slave_command_code[index].length_type == 3)
		        {
		            len  = modbus_rec.buffer[slave_command_code[index].length_index + 2] << 16;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 3] << 8;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 4];
		        }
		        else if (slave_command_code[index].length_type == 4)
		        {
		            len  = modbus_rec.buffer[slave_command_code[index].length_index + 2] << 24;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 3] << 16;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 4] << 8;
		            len += modbus_rec.buffer[slave_command_code[index].length_index + 5];
		        }
		        modbus_rec.dat_len = len * slave_command_code[index].data_align;
		        //  申请内存空间
		        modbus_rec.dat = malloc(modbus_rec.dat_len + 4);
				if (modbus_rec.dat)
				{
					DebugPrintf(__FILE__, __LINE__, "malloc", "(%u):0x%X - ModbusMasterRec\r\n",
						modbus_rec.dat_len + 4,
						(uint32_t)modbus_rec.dat);
		        	modbus_rec.dat[modbus_rec.rec_len - slave_command_code[index].length - 2] = temp;
				}
				else
				{
					DebugPrintf(__FILE__, __LINE__, "sys_busy", "\r\n");
				}
	        }
	        else if ((slave_command_code[index].length_type)  //  是否有数据需要接收
	            && ((modbus_rec.rec_len - slave_command_code[index].length - 2) < modbus_rec.dat_len))  // 数据是否接收完成
	        {
	            modbus_rec.dat[modbus_rec.rec_len - slave_command_code[index].length - 2] = temp;
	        }
	        else if (modbus_rec.rec_len == (slave_command_code[index].length + modbus_rec.dat_len + 2))  //  判断是否接收CRC
	        {
	            modbus_rec.crc = (temp) & 0xFF00;
	        }
	        else  //  数据接收完成
	        {
	            modbus_rec.crc += (temp << 8);
				crc = 0xFFFF;
		        crc = CRC16(crc, modbus_rec.buffer, slave_command_code[modbus_code_index[modbus_rec.buffer[1]]].length + 2);

	            if (modbus_rec.dat_len && modbus_rec.dat)
		        {
		            crc = CRC16(crc, modbus_rec.dat, modbus_rec.dat_len);
		        }
		        if (crc == modbus_rec.crc)
		        {
	                modbus_rec.errcode = 0;
		        }
		        else
		        {
		            modbus_rec.errcode = MODBUS_ERRORCODE_CRC;
		        }
                modbus_rec.rec_len--;  //  长度包含一个CRC字节，返回时减掉
				return 1;
	        }
	    }
		modbus_rec.rec_len++;
    }
	return 0;
}


/******************************************************************
** 函数名称:   ModbusCommHandle
** 功能描述:   读队列数据
** 输入:
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
#define MODBUS_ST_IDLE            0  //  空闲状态，可以准备数据
#define MODBUS_ST_SEND            1  //  正在发送
#define MODBUS_ST_REC             2  //  等待接收
#define MODBUS_ST_TIMEOUT         3  //  命令超时
#define MODBUS_ST_WAIT            4  //  命令间隔等待
static uint8_t ModbusCommHandle(uint8_t code, uint8_t dev, ...)
{
	//static s_timer_argv_t * modbus_timer = 0;
    static uint8_t *send_buffer = 0;
	static uint8_t modbus_state = 0;
	static uint16_t  len = 0;
    uint16_t  send_len;
	va_list argp;

    //  超时则为下一帧数据

	switch (modbus_state)
	{
	case MODBUS_ST_IDLE:  //  modbus 空闲状态，可以发起命令
        va_start(argp, dev);
        send_buffer = ModbusMasterSend(dev, code, argp);  //  生成modbus发送数据
		va_end(argp);
		if (send_buffer == 0)  //  生成成功
		{
			return MODBUS_ST_IDLE;
		}
		len = 0;
		modbus_state = MODBUS_ST_SEND;
		break;
	case MODBUS_ST_SEND    :
		send_len = send_buffer[0] + (send_buffer[1] << 8);  //  读需要发送到数据
		if (len >= send_len)  //  数据是否发送完成
		{
			//modbus_timer = CreateTimer(0,0,0,100,0);  //  发送完成，创建超时时钟
			//if (modbus_timer)  //  时钟创建成功
            modbus_master_timer = 500;  //500ms
			{
				modbus_state = MODBUS_ST_REC;  //  准备接收数据
				memset(&modbus_rec, 0, sizeof(modbus_rec));
			}
		}
		else
		{
			len += ModbusWriteBus(&send_buffer[2+len], send_len-len);  //  发送数据
			if (len >= send_len)   //  数据是否发送完成
			{
                DebugPrintf(__FILE__, __LINE__, "modbus", "Send: %02X ", send_buffer[2]);
				for (len=1; len<send_len; len++)
				{
					DebugPrintf_1("modbus", "%02X ", send_buffer[2+len]);
				}
				DebugPrintf_1("modbus", "\r\n");
				DebugPrintf(__FILE__, __LINE__, "free", "0x%X - ModbusCommHandle\r\n",(uint32_t)send_buffer);
				free(send_buffer);
				//modbus_timer = CreateTimer(0,0,0,100,0);  //  发送完成，创建超时时钟
				//if (modbus_timer)  //  时钟创建成功
                modbus_master_timer = 500;
				{
					modbus_state = MODBUS_ST_REC;  //  准备接收数据
					memset(&modbus_rec, 0, sizeof(modbus_rec));
				}
			}

		}
		break;
	case MODBUS_ST_REC:   //  modbus 接收状态
		if (ModbusMasterRec(dev, code) == 1)
		{
			//DelTimer(modbus_timer);

            DebugPrintf(__FILE__, __LINE__, "modbus", "Rec:  %u\r\n", modbus_rec.rec_len);
            modbus_state = MODBUS_ST_WAIT;
		}
		//else if (CheckTimer(modbus_timer, 1) == 0)
        else if (modbus_master_timer == 0)
		{
			if (modbus_rec.dat)
			{
				DebugPrintf(__FILE__, __LINE__, "free", "0x%X - ModbusCommHandle\r\n",(uint32_t)modbus_rec.dat);
				free(modbus_rec.dat);
				modbus_rec.dat = 0;
			}
			memset(&modbus_rec, 0, sizeof(modbus_rec));
			modbus_state = MODBUS_ST_TIMEOUT;
		}
	    break;
	case MODBUS_ST_WAIT:  //  延时等待总线空闲
	    modbus_state = MODBUS_ST_IDLE;
        break;
	case MODBUS_ST_TIMEOUT:
	    modbus_state = MODBUS_ST_IDLE;
		break;
	default:
		modbus_state = MODBUS_ST_WAIT;
		break;
	}
    return modbus_state;
}


/******************************************************************
** 函数名称:   ModbusReadC
** 功能描述:   读取线圈
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 线圈地址
**             uint16_t len   -- 线圈个数
**             uint16_t *buf  -- 读取的数据存储空间
**
** 输出:	   0 -- 读取成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusReadC(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}


/******************************************************************
** 函数名称:   ModbusWriteC
** 功能描述:   写线圈
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 线圈地址
**             uint16_t len   -- 线圈个数
**             uint16_t *buf  -- 写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusWriteC(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}

/******************************************************************
** 函数名称:   ModbusReadR
** 功能描述:   读取寄存器
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 寄存器地址
**             uint16_t len   -- 数据长度
**             uint16_t *buf  -- 读取的数据存储空间
**
** 输出:	   0 -- 读取成功； 其他返回MODBUS错误码
**             -1 -- 正在处理
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
int8_t ModbusReadR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf)
{
	uint8_t err;
	uint16_t rec_len;
	err = ModbusCommHandle(MODBUS_CODE_READ_HOLDING_REGISTERS, dev, addr, buf, len);
	if (err == MODBUS_ST_WAIT)
	{
		if (modbus_rec.errcode == 0)
		{
			if (modbus_rec.buffer[1] & 0x80)
			{
				return modbus_rec.buffer[2];
			}
			else
			{
				rec_len = modbus_rec.rec_len -
					slave_command_code[modbus_code_index[MODBUS_CODE_READ_HOLDING_REGISTERS]].length - 2;
				if (modbus_rec.dat)
				{
                	//memcpy(buf, modbus_rec.dat, rec_len);
                    HalfWordBigEndianCopy(buf, modbus_rec.dat, rec_len);
					DebugPrintf(__FILE__, __LINE__, "free", "0x%X - ModbusReadR\r\n",(uint32_t)modbus_rec.dat);
					free(modbus_rec.dat);
				}
				return 0;
			}
		}
		else
		{
			if (modbus_rec.dat)
			{
				DebugPrintf(__FILE__, __LINE__, "free", "0x%X - ModbusReadR\r\n",(uint32_t)modbus_rec.dat);
				free(modbus_rec.dat);
				modbus_rec.dat = 0;
			}
			return MODBUS_ERRORCODE_CRC;
		}
	}
	else if (err == MODBUS_ST_TIMEOUT)
	{
		return MODBUS_ERRORCODE_GATEWAY_FAILED_TO_RESPOND;
	}
    return -1;
}


/******************************************************************
** 函数名称:   ModbusWriteR
** 功能描述:   写寄存器
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 寄存器地址
**             uint16_t len   -- 数据长度
**             uint16_t *buf  -- 写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
**             -1 -- 正在处理
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
int8_t ModbusWriteR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf)
{
    uint8_t err;
	if (len == 1)
	{
        err = ModbusCommHandle(MODBUS_CODE_WRITE_SINGLE_REGISTER, dev, addr, buf, len*2);
	}
	else if (len)
	{
		err = ModbusCommHandle(MODBUS_CODE_WRITE_MULTIPLE_REGISTERS, dev, addr, buf, len*2);
	}
	if (err == MODBUS_ST_WAIT)
	{
		if (modbus_rec.errcode == 0)
		{
			if (modbus_rec.buffer[1] & 0x80)
			{
				return modbus_rec.buffer[2];
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return MODBUS_ERRORCODE_CRC;
		}
	}
	else if (err == MODBUS_ST_TIMEOUT)
	{
		return MODBUS_ERRORCODE_GATEWAY_FAILED_TO_RESPOND;
	}
    return -1;
}
/******************************************************************
** 函数名称:   ModbusRWR
** 功能描述:   读和写寄存器
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 寄存器地址
**             uint16_t len   -- 数据长度
**             uint16_t *buf  -- 读和写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusRWR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}

/******************************************************************
** 函数名称:   ModbusReadF
** 功能描述:   读取文件记录
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t file_no   -- 文件好
**             uint16_t record_no -- 记录号
**             uint8_t record_len -- 记录长度
**             uint16_t *buf  -- 读取的数据存储空间
**
** 输出:	   0 -- 读取成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusReadF(uint8_t dev, uint16_t file_no, uint16_t record_no, uint8_t record_len, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}


/******************************************************************
** 函数名称:   ModbusWriteF
** 功能描述:   写文件记录
** 输入:	   uint8_t  dev       -- 设备地址
**             uint16_t file_no   -- 文件好
**             uint16_t record_no -- 记录号
**             uint8_t record_len -- 记录长度
**             uint16_t *buf      -- 写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusWriteF(uint8_t dev, uint16_t file_no, uint16_t record_no, uint8_t record_len, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}


/******************************************************************
** 函数名称:   ModbusReadQ
** 功能描述:   读队列数据
** 输入:	   uint8_t  dev       -- 设备地址
**             uint16_t addr   -- 队列地址
**             uint16_t *buf      -- 写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:	   buf 的长度大于等于64byte
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
uint8_t ModbusReadQ(uint8_t dev, uint16_t addr, uint16_t *buf)
{
    return MODBUS_ERRORCODE_DISABLE_OPRATION;
}



