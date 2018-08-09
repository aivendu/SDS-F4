/***********************************************************
** Copyright (c) 2014, 江苏慧乔信息科技有限公司
** All rights reserved.
** 文件名称： system_error.c
** 摘要：   该文件实现错误管理功能，提供错误激活、错误清除、错误发送标志置一、
**			获取对应错误状态、获取错误标志、清除错误标志。
**			在激活某一错误时，会把对应的错误标志置一，以提示应用层有错误置一
**          对各种错误状态进行定义
** 版本信息：
**   20140423, 杜其俊，实现错误管理功能
**-----------------------------------------------------------------------
**  版本    日期        修改人      描述
**-----------------------------------------------------------------------
**  v1.0  20171218  杜其俊      与上一版本不兼容,支持一个模块同时发生多个错误
**                           整理获取错误信息的接口
**
***********************************************************/
#ifndef _SYSTEM_ERROR_H
#define _SYSTEM_ERROR_H

#include "stdint.h"

//  目前只支持三种错误等级, 如果要增加错误等级，注意s_system_error_t中的severity的位数要改变
typedef enum
{
    SEVERITY_NO_ERR  = 0,		//	没有错误
    SEVERITY_WARNING = 1,		//	警告
    SEVERITY_MAJOR   = 2,			//	主要的错误
    SEVERITY_CRITICAL= 4,		//	紧急错误
    SEVERITY_MASK    = 0xFF,          //  用于搜索时，不匹配错误等级用
} e_sys_err_severity_t;

/*
System   Id Severity Comment
Controller  1  Warning Busy, please wait
Controller  2  Critical   Alarm configuration error
Controller  3  Critical   Stats configuration error
Controller  4  Critical   AES not supported
Controller  5  Critical   SSL not supported
Controller  6  Critical   3DES not supported
Controller  7  Major  Rate group configuration error
Controller  8  Warning Rate group not used
*/
typedef enum
{
    CONTROLLER_BUSY = 1,
    CONTROLLER_ALARM_CONFIGURATION_ERROR,
    CONTROLLER_STATS_CONFIGURATION_ERROR,
    CONTROLLER_AES_NOT_SUPPORTED,
    CONTROLLER_SSL_NOT_SUPPORTED,
    CONTROLLER_DES3_NOT_SUPPORTED,
    BDCOM_NOT_RESPONDING,
    CONTROLLER_RATE_GROUP_CONFIGURATION_ERROR,
    CONTROLLER_RATE_GROUP_NOT_USED,
    CONTROLLER_ERR_NUM,
} e_controller_err_t;

/*
CreditCardReader 1  Critical   Reader not responding
CreditCardReader 2  Major   Unit error
*/
typedef enum
{
    CREDITCARD_NORMAL,
    CREDITCARD_NOT_RESPONDING,
    CREDITCARD_NETWORK_ERROR,
    CREDITCARD_PINPAD_ERROR,
    CREDITCARD_READER_ERROR,
    CREDITCARD_OTHER_ERROR,
    CREDITCARD_ERR_NUM
} e_creditcard_err_t;

/*
Printer   1  Critical   Printer not responding
Printer   2  Major   Unit error
Printer   3  Major   Out of paper
Printer   4  Warning Paper low
*/
typedef enum
{
    PRINTER_NOT_RESPONDING = 1,
    PRINTER_UNIT_ERROR,
    PRINTER_OUT_OF_PAPER,
    PRINTER_PAPER_JAM,
    PRINTER_PAPER_LOW,
    PRINTER_OTHER_ERROR,
    PRINTER_ERR_NUM,
} e_printer_err_t;

/*
BillCollector   1  Critical   Bill collector not responding
BillCollector   2  Major   Unit error
*/
typedef enum
{
    BILL_NORMAL,
    BILL_NOT_RESPONDING,
    BILL_RECEIVE_FULL,
    BILL_DISPENSER_EMPTY,
    BILL_JAM,
    BILL_RECEIVE_WARNING,
    BILL_DISPENSER_LOW,
    BILL_SENSER_ERROR,
    BILL_OTHER_ERROR,
    BILL_ERR_NUM
} e_bill_err_t;

/*
Coin unit   1  Critical   Coin unit not responding
Coin unit   2  Major   Unit error
Coin unit   3  Major   Velcro error
Coin unit   4  Major   Coin collector full
*/
typedef enum
{
    COIN_NORMAL,
    COIN_NOT_RESPONDING,
    COIN_RECEIVE_FULL,
    COIN_DISPENSER_EMPTY,
    COIN_JAM,
    COIN_RECEIVE_WARNING,
    COIN_DISPENSER_LOW,
    COIN_SENSER_ERROR,
    COIN_OTHER_ERROR,
    COIN_ERR_NUM
} e_coin_err_t;


/*
Display  1  Critical   Display not responding
Display  2  Major   Unit error
Display  3  Warning Illumination error  */
typedef enum
{
    LCD_SCREEN_NOT_RESPONDING=1,
    LCD_SCREEN_ERROR,
    LCD_ILLUMINATION_ERROR,
    LCD_SCREEN_OTHER_ERROR,
    LCDSCREEN_ERR_NUM,
} e_lcdscreen_err_t;


/*
Door   1  Critical   Upper door safety alarm
Door   2  Critical   Lower door safety alarm
Door   3  Critical   Battery door safety alarm
*/
typedef enum
{
    UPPER_DOOR_SAFETY_ALARM = 1,
    LOWER_DOOR_SAFETY_ALARM,
    BATTERY_DOOR_SAFETY_ALARM,
    UPPER_DOOR_OPEN,
    LOWER_DOOR_OPEN,
    BATTERY_DOOR_OPEN,
    UPPER_DOOR_CLOSE,
    LOWER_DOOR_CLOSE,
    BATTERY_DOOR_CLOSE,
    DOOR_ERR_NUM,
} e_door_err_t;

/*
SolarPanel   1  Critical   Solar Panel not responding
SolarPanel   2  Major   Unit error
*/
typedef enum
{
    SOLARPANEL_NOT_RESPONDING = 1,
    SOLARPANEL_UNIT_ERROR,
    SOLARPANEL_ERR_NUM,
} e_solarpanel_err_t;
/*
IOCTRL  1  Critical   IO unit not responding
*/
typedef enum
{
    IO_UNIT_NOT_RESPONDING = 1,
    IOCTRL_ERR_NUM,
} e_ioctrl_err_t;
/*
LED  1  Critical   LED unit not responding
*/
typedef enum
{
    LED_SCREEN_NORMAL,
    LED_SCREEN_NOT_RESPONDING,
    LED_SCREEN_OTHER_ERROR,
    LED_SCREEN_ERR_NUM
} e_led_err_t;

/*
USB1   1  Major   Error
USB2   2  Major   Error
USB3   3  Major   Error
*/
typedef enum
{
    USB1_ERROR = 1,
    USB2_ERROR,
    USB3_ERROR,
    USB_ERR_NUM
} e_usb_err_t;


typedef enum alarm_code_system
{
    SYSTEM_ALL,
    SYSTEM_CONTROLLER,          //  Controller
    SYSTEM_CREDITCARD,          //  CreditCardReader
    SYSTEM_PRINTER,             //  Printer
    SYSTEM_BILLCOLLECTOR,       //  BillCollector
    SYSTEM_COIN_UNIT,           //  Coin unit
    SYSTEM_DISPLAY,             //  Display
    SYSTEM_DOOR,                //  Door
    SYSTEM_SOLARPANEL,          //  SolarPanel
    SYSTEM_IOCTRL,              //  IOCTRL
    SYSTEM_USB,                 //  Ethernet
    SYSTEM_LEDSCREEN,           //  led screens
    SYSTEM_COIN_DISPENSER,       //  coin dispenser
    SYSTEM_BILL_DISPENSER,      //  bill dispenser
    SYSTEM_COIN_DISPENSER2,      //  coin dispenser2
    SYSTEM_NUMBER
} e_alarm_code_t;


//  设置错误
//  module -- 需要操作的模块
//  err_id  -- 需要操作的错误号
extern int8_t ActivateSystemError(e_alarm_code_t module, uint8_t err_id);

//重新激活还存在的错误，使其再向server发送一次
extern int8_t ReActivateSystemError(void);

//  清除错误错误
//  module -- 需要操作的模块
//  err_id  -- 需要操作的错误号
extern int8_t ClearSystemError(e_alarm_code_t module, uint8_t err_id);
extern int8_t ClearModuleAllError(e_alarm_code_t module);

//  置起错误已发送标志
//  module -- 需要操作的模块
//  err_id  -- 需要操作的错误号
extern int8_t ClearErrorSentFlag(e_alarm_code_t module, uint8_t err_id);


/**
 * @brief  获取对应模块的错误id
 * @param     e_alarm_code_t module -- 错误模块，表示某一错误种类
 * @param     state_mask: 是否比较状态,  当为 ERROR_GETID_MASK 时表示不比较该条件
 * @param     sendflag_mask: 是否比较sendflag
 * @param     level: 是否比较severity
 * @retval   >0 模块的错误ID
 *            0 没有错误
 *           <0 参数错误
 */
#define ERROR_GETID_MASK          0xFF    //  
extern int8_t GetErrorID(e_alarm_code_t module, uint8_t state_mask, uint8_t sendflag_mask, uint8_t level);
extern int8_t GetErrorSeverity(e_alarm_code_t module, uint8_t err_id);
extern int8_t GetErrorSendFlag(e_alarm_code_t module, uint8_t err_id);
extern int8_t GetErrorState(e_alarm_code_t module, uint8_t err_id);
extern int16_t GetErrorCount(e_alarm_code_t module, uint8_t err_id);


//  获取错误标志，该标志表示有错误需要处理
//  返回错误标志，0 --没 有需要处理的错误，1-- 有需要处理的错误
extern uint8_t GetErrorSeverityFlag(e_sys_err_severity_t flag);

//  清除错误标志
extern uint8_t ClearErrorSeverityFlag(e_sys_err_severity_t flag);


#endif

