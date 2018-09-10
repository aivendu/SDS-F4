/***********************************************************
** Copyright (c) 2015, 	 江苏慧乔信息科技有限公司
** All rights reserved.
** 文件名称： modbus_master.h
** 摘要：     modbus协议，服务器实现，基于<Modbus_Application_Protocol_V1_1b3.pdf>
** 当前版本： 1.0, aiven，20150528，创建
***********************************************************/

#ifndef _MODBUS_MASTER_H
#define _MODBUS_MASTER_H

#include "stdint.h"

/*   MODBUS Exception Codes
-----------------------------------------------------------------------------------------
Code|  Name                 |   Meaning
----+-----------------------+--------------------------------------------------------------
01  | ILLEGAL FUNCTION      |  The  function  code  received  in  the  query  is  not  an
    |                       |  allowable  action  for  the server.  This  may  be
    |                       |  because  the  function  code  is  only  applicable  to
    |                       |  newer  devices,  and  was  not  implemented  in  the
    |                       |  unit selected. It could also indicate  that the server
    |                       |  is  in  the  wrong  state  to  process  a  request  of  this
    |                       |  type,  for  example  because  it  is  unconfigured  and
    |                       |  is being asked to return register values.
----|-----------------------|--------------------------------------------------------------
02  | ILLEGAL DATA ADDRESS  |  The data address received in the query is not an
    |                       |  allowable address for the server.  More
    |                       |  specifically, the combination of reference number
    |                       |  and transfer length is invalid. For a controller with
    |                       |  100 registers, the PDU addresses the first register
    |                       |  as 0,  and the last one as 99.  If a request is
    |                       |  submitted with a starting register address of 96
    |                       |  and a quantity of registers of 4, then this request
    |                       |  will successfully operate (address-wise at least)
    |                       |  on registers 96, 97, 98, 99.  If a request is
    |                       |  submitted with a starting register address of 96
    |                       |  and  a  quantity  of  registers  of  5,  then  this  request
    |                       |  will  fail  with  Exception  Code  0x02  “Illegal  Data
    |                       |  Address” since it attempts to operate on registers
    |                       |  96,  97, 98,  99  and  100,  and  there  is  no  register
    |                       |  with address 100.
----|-----------------------|--------------------------------------------------------------
03  | ILLEGAL DATA VALUE    |  A  value contained  in the  query  data field  is  not  an
    |                       |  allowable value for server. This indicates a fault in
    |                       |  the  structure  of  the  remainder  of  a  complex
    |                       |  request,  such  as  that  the  implied  length  is
    |                       |  incorrect.  It  specifically  does  NOT  mean  that  a
    |                       |  data item submitted for storage in a register has a
    |                       |  value  outside  the  expectation  of  the  application
    |                       |  program,  since  the  MODBUS  protocol  is  unaware
    |                       |  of  the  significance  of  any  particular  value  of  any
    |                       |  particular register.
----|-----------------------|--------------------------------------------------------------
04  | SERVER DEVICE FAILURE |  An  unrecoverable  error  occurred  while  the  server
    |                       |  was attempting to perform the requested action.
----|-----------------------|--------------------------------------------------------------
05  | ACKNOWLEDGE           |  Specialized  use  in  conjunction  with  programming
    |                       |  commands.
    |                       |  The  server  has  accepted  the  request  and  is
    |                       |  processing  it,  but  a  long  duration  of  time  will  be
    |                       |  required  to  do  so.  This  response  is  returned  to
    |                       |  prevent  a  timeout  error  from  occurring  in  the
    |                       |  client.  The  client  can  next  issue  a  Poll  Program
    |                       |  Complete  message  to  determine  if  processing  is
    |                       |  completed.
----|-----------------------|--------------------------------------------------------------
06  | SERVER DEVICE BUSY    |  Specialized  use  in  conjunction  with  programming
    |                       |  commands.
    |                       |  The  server  is  engaged  in  processing  a  long–
    |                       |  duration  program  command.  The  client  should
    |                       |  retransmit  the  message  later  when  the  server  is
    |                       |  free.
----|-----------------------|--------------------------------------------------------------
08  | MEMORY PARITY ERROR   |  Specialized use in conjunction with function codes
    |                       |  20  and  21  and  reference  type  6,  to  indicate  that
    |                       |  the extended file area failed to pass a consistency
    |                       |  check.
    |                       |  The  server  attempted  to  read  record  file,  but
    |                       |  detected  a  parity  error  in  the  memory.  The  client
    |                       |  can retry the request, but service may be required
    |                       |  on the server device.
----|-----------------------|--------------------------------------------------------------
0A  | GATEWAY PATH          |  Specialized  use  in  conjunction  with  gateways,
    | UNAVAILABLE           |  indicates  that  the  gateway  was  unable  to  allocate
    |                       |  an internal communication path from the input port
    |                       |  to  the  output  port  for  processing  the  request.
    |                       |  Usually  means  that  the  gateway  is  misconfigured
    |                       |  or overloaded.
----|-----------------------|--------------------------------------------------------------
0B  | GATEWAY TARGET DEVICE |  Specialized  use  in  conjunction  with  gateways,
    | FAILED TO RESPOND     |  indicates  that  no  response  was  obtained  from  the
    |                       |  target device. Usually means that the device is not
    |                       |  present on the network.
-------------------------------------------------------------------------------------------
*/
#define MODBUS_ERRORCODE_SUCCESSED                  0x00  //  操作成功
#define MODBUS_ERRORCODE_ILLEGAL_FUNCTION           0x01  //  非法功能码
#define MODBUS_ERRORCODE_ILLEGAL_DATA_ADDRESS       0x02  //  非法数据地址
#define MODBUS_ERRORCODE_ILLEGAL_DATA_VALUE         0x03  //  非法数据值
#define MODBUS_ERRORCODE_SERVER_DEVICE_FAILURE      0x04  //  从站设备故障
#define MODBUS_ERRORCODE_ACKNOWLEDGE                0x05  //  请求已被确认，但需要较长时间来处理请求
#define MODBUS_ERRORCODE_SERVER_DEVICE_BUSY         0x06  //  从设备忙
#define MODBUS_ERRORCODE_MEMORY_PARITY_ERROR        0x08  //  存储奇偶性差错
#define MODBUS_ERRORCODE_GATEWAY_PATH_UNAVAILABLE   0x0A  //  不可用的网关
#define MODBUS_ERRORCODE_GATEWAY_FAILED_TO_RESPOND  0x0B  //  网关目标设备响应失败
#define MODBUS_ERRORCODE_DISABLE_OPRATION           0x40  //  禁止操作
#define MODBUS_ERRORCODE_CRC                        0x41  //  CRC 校验错误
#define MODBUS_ERRORCODE_UNDEFINE                   0xFF  //  未定义错误

//  定义操作码
#define MODBUS_CODE_READ_DISCRETE_INPUTS			 0x02  //  读分立元件输入
#define MODBUS_CODE_READ_COILS						 0x01  //  读线圈
#define MODBUS_CODE_WRITE_SINGLE_COIL				 0x05  //  读单个线圈
#define MODBUS_CODE_WRITE_MULTIPLE_COILS			 0x0F  //  写多个线圈
#define MODBUS_CODE_READ_INPUT_REGISTER 			 0x04  //  读输入寄存器
#define MODBUS_CODE_READ_HOLDING_REGISTERS			 0x03  //  读保持寄存器
#define MODBUS_CODE_WRITE_SINGLE_REGISTER			 0x06  //  写单个寄存器
#define MODBUS_CODE_WRITE_MULTIPLE_REGISTERS		 0x10  //  写多个寄存器
#define MODBUS_CODE_READ_WRITE_MULTIPLE_REGISTERS	 0x17  //  读写多个寄存器
#define MODBUS_CODE_MASK_WRITE_REGISTER 			 0x16  //  写保持寄存器的某一位
#define MODBUS_CODE_READ_FIFO_QUEUE 				 0x18  //  读队列里的数据
#define MODBUS_CODE_READ_FILE_RECORD				 0x14  //  读文件记录
#define MODBUS_CODE_WRITE_FILE_RECORD				 0x15  //  写文件记录
#define MODBUS_CODE_READ_EXCEPTION_STATUS			 0x07  //  读异常状态
#define MODBUS_CODE_DIAGNOSTIC						 0x08  //  诊断
#define MODBUS_CODE_GET_COM_EVENT_COUNTER			 0x0B  //
#define MODBUS_CODE_GET_COM_EVENT_LOG				 0x0C  //
#define MODBUS_CODE_REPORT_SERVER_ID				 0x11  //
#define MODBUS_CODE_READ_DEVICE_IDENTIFICATION		 0x2B  //  读设备信息
//#define MODBUS_CODE_ENCAPSULATED_INTERFACE_TRANSPORT 0x2B  //
//#define MODBUS_CODE_CANOPEN_GENERAL_REFERENCE		 0x2B  //



/*  定义  */
typedef struct
{
    uint8_t  state;     //  当前命令的处理状态
    #define MODBUS_ST_IDLE            0  //  没有命令处理
    #define MODBUS_ST_SENT            1  //  有命令已经被发送
    #define MODBUS_ST_RECEIVED        2  //  命令接收完成
    uint8_t  errcode;   //  错误码
    uint16_t rec_len;   //  数据当前接收的个数，包扩帧内所有数据
    uint16_t dat_len;   //  数据个数, 命令实际操作的数据的总个数
    uint16_t crc;       //  校验
    uint8_t  buffer[16];
    uint8_t  *dat;
} s_modbus_master_rec_t;

/* 定义主机发送命令参数 */
typedef struct
{
	uint8_t  state;
    uint8_t  dev;
    uint8_t  code;
    uint8_t  len;
    uint16_t addr;
    uint8_t  *dat;
} s_modbus_master_send_t;




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
extern uint8_t ModbusReadC(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf);


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
extern uint8_t ModbusWriteC(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf);

/******************************************************************
** 函数名称:   ModbusReadR
** 功能描述:   读取寄存器
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 寄存器地址
**             uint16_t len   -- 数据长度
**             uint16_t *buf  -- 读取的数据存储空间
**
** 输出:	   0 -- 读取成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
extern int8_t ModbusReadR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf);


/******************************************************************
** 函数名称:   ModbusWriteR
** 功能描述:   写寄存器
** 输入:	   uint8_t  dev   -- 设备地址
**             uint16_t addr  -- 寄存器地址
**             uint16_t len   -- 数据长度
**             uint16_t *buf  -- 写的数据存储空间
**
** 输出:	   0 -- 写成功； 其他返回MODBUS错误码
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   Aiven / 杜其俊
** 日期:	   20150528
******************************************************************/
extern int8_t ModbusWriteR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf);


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
extern uint8_t ModbusRWR(uint8_t dev, uint16_t addr, uint16_t len, uint16_t *buf);



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
extern uint8_t ModbusReadF(uint8_t dev, uint16_t file_no, uint16_t record_no, uint8_t record_len, uint16_t *buf);


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
extern uint8_t ModbusWriteF(uint8_t dev, uint16_t file_no, uint16_t record_no, uint8_t record_len, uint16_t *buf);


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
extern uint8_t ModbusReadQ(uint8_t dev, uint16_t addr, uint16_t *buf);


#endif
