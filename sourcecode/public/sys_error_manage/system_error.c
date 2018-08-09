
/***********************************************************
** Copyright (c) 2014, 江苏慧乔信息科技有限公司
** All rights reserved.
** 文件名称： system_error.c
** 摘要：   该文件实现错误管理功能，提供错误激活、错误清除、错误发送标志置一、
**			获取对应错误状态、获取错误标志、清除错误标志。
**			在激活某一错误时，会把对应的错误标志置一，以提示应用层有错误置一
** 版本信息：
**   20140423, 杜其俊，实现错误管理功能
**
***********************************************************/
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "system_error.h"
#ifdef _SYSTEM_ERROR_H

#define  SYSTEM_ERROR_DEBUG_EN     1

#if  SYSTEM_ERROR_DEBUG_EN
#include "debug_log.h"
#define ErrorDebug       Printf_D
#else
#define ErrorDebug(type,format,...)
#endif

#define ALARM_CODE_SYSTEM_NUM		SYSTEM_NUMBER  //  提供报警的种类数量
#define MAX_ALARM_CODE_ID			12             //  在每个报警种类中，错误类型的最大值


//  错误的数据结构
typedef struct s_system_error
{
    const uint32_t id			: 8;		//	错误号
    const uint32_t severity	: 3;		//	错误优先级, 按位表示, 分别是0bit:warning, 1bit:major, 2bit:critical
    uint32_t sendflag	: 1;		//	错误发送标志,1--未发送，0--已发送
    uint32_t edge		: 1;		//	该错误是被设置还是被清除，0--被清除，1--被置起
    uint32_t unused		: 3;		//	未使用
    uint32_t counter	: 16;		//	错误产生次数
    uint32_t happentime;            //  时间戳
} s_system_error_t;



static uint8_t severity_level_flag;	//	不同优先级错误的标志, 对应位置一表示有对应优先级的错误需要被处理

static s_system_error_t controller_errors[CONTROLLER_ERR_NUM] = 
{
    {0,                                        SEVERITY_NO_ERR   },
    {CONTROLLER_BUSY,                          SEVERITY_WARNING  },
    {CONTROLLER_ALARM_CONFIGURATION_ERROR,     SEVERITY_CRITICAL },
    {CONTROLLER_STATS_CONFIGURATION_ERROR,     SEVERITY_CRITICAL },
    {CONTROLLER_AES_NOT_SUPPORTED,             SEVERITY_CRITICAL },
    {CONTROLLER_SSL_NOT_SUPPORTED,             SEVERITY_CRITICAL },
    {CONTROLLER_DES3_NOT_SUPPORTED,            SEVERITY_CRITICAL },
    {BDCOM_NOT_RESPONDING,                     SEVERITY_CRITICAL },
    {CONTROLLER_RATE_GROUP_CONFIGURATION_ERROR,SEVERITY_MAJOR    },
    {CONTROLLER_RATE_GROUP_NOT_USED,           SEVERITY_WARNING  },
};
static s_system_error_t creditcard_errors[CREDITCARD_ERR_NUM] =
{
    {0,                                      SEVERITY_NO_ERR  },
    {CREDITCARD_NOT_RESPONDING,              SEVERITY_CRITICAL},
    {CREDITCARD_NETWORK_ERROR,               SEVERITY_CRITICAL},
    {CREDITCARD_PINPAD_ERROR,                SEVERITY_CRITICAL},
    {CREDITCARD_READER_ERROR,           SEVERITY_CRITICAL},
    {CREDITCARD_OTHER_ERROR,                 SEVERITY_CRITICAL},
};
static s_system_error_t printer_errors[PRINTER_ERR_NUM] =
{
    {0,                                  SEVERITY_NO_ERR  },
    {PRINTER_NOT_RESPONDING,             SEVERITY_CRITICAL},
    {PRINTER_UNIT_ERROR,                 SEVERITY_CRITICAL},
    {PRINTER_OUT_OF_PAPER,               SEVERITY_CRITICAL},
    {PRINTER_PAPER_JAM,                  SEVERITY_CRITICAL},
    {PRINTER_PAPER_LOW,                  SEVERITY_MAJOR   },
    {PRINTER_OTHER_ERROR,                SEVERITY_WARNING },
};
static s_system_error_t bill_errors[BILL_ERR_NUM] =
{
    {0,                                  SEVERITY_NO_ERR  },
    {BILL_NOT_RESPONDING,                SEVERITY_CRITICAL},
    {BILL_RECEIVE_FULL,                  SEVERITY_CRITICAL},
    {BILL_DISPENSER_EMPTY,                SEVERITY_CRITICAL},
    {BILL_JAM,                           SEVERITY_CRITICAL},
    {BILL_RECEIVE_WARNING,               SEVERITY_MAJOR   },
    {BILL_DISPENSER_LOW,                  SEVERITY_MAJOR   },
    {BILL_SENSER_ERROR,                  SEVERITY_WARNING },
    {BILL_OTHER_ERROR,                   SEVERITY_MAJOR   },
};
static s_system_error_t billdispenser_errors[BILL_ERR_NUM] =
{
    {0,                                  SEVERITY_NO_ERR  },
    {BILL_NOT_RESPONDING,                SEVERITY_CRITICAL},
    {BILL_RECEIVE_FULL,                  SEVERITY_CRITICAL},
    {BILL_DISPENSER_EMPTY,                SEVERITY_CRITICAL},
    {BILL_JAM,                           SEVERITY_CRITICAL},
    {BILL_RECEIVE_WARNING,               SEVERITY_MAJOR   },
    {BILL_DISPENSER_LOW,                  SEVERITY_MAJOR   },
    {BILL_SENSER_ERROR,                  SEVERITY_WARNING },
    {BILL_OTHER_ERROR,                   SEVERITY_MAJOR   },
};
static s_system_error_t coin_errors[COIN_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR  },
    {COIN_NOT_RESPONDING,            SEVERITY_CRITICAL},
    {COIN_RECEIVE_FULL,              SEVERITY_CRITICAL},
    {COIN_DISPENSER_EMPTY,            SEVERITY_CRITICAL},
    {COIN_JAM,                       SEVERITY_CRITICAL},
    {COIN_RECEIVE_WARNING,           SEVERITY_MAJOR   },
    {COIN_DISPENSER_LOW,              SEVERITY_MAJOR   },
    {COIN_SENSER_ERROR,              SEVERITY_MAJOR   },
    {COIN_OTHER_ERROR,               SEVERITY_MAJOR   },
};
static s_system_error_t coindispenser_errors[COIN_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR  },
    {COIN_NOT_RESPONDING,            SEVERITY_CRITICAL},
    {COIN_RECEIVE_FULL,              SEVERITY_CRITICAL},
    {COIN_DISPENSER_EMPTY,            SEVERITY_CRITICAL},
    {COIN_JAM,                       SEVERITY_CRITICAL},
    {COIN_RECEIVE_WARNING,           SEVERITY_MAJOR   },
    {COIN_DISPENSER_LOW,              SEVERITY_MAJOR   },
    {COIN_SENSER_ERROR,              SEVERITY_MAJOR   },
    {COIN_OTHER_ERROR,               SEVERITY_MAJOR   },
};
static s_system_error_t coindispenser2_errors[COIN_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR  },
    {COIN_NOT_RESPONDING,            SEVERITY_CRITICAL},
    {COIN_RECEIVE_FULL,              SEVERITY_CRITICAL},
    {COIN_DISPENSER_EMPTY,            SEVERITY_CRITICAL},
    {COIN_JAM,                       SEVERITY_CRITICAL},
    {COIN_RECEIVE_WARNING,           SEVERITY_MAJOR   },
    {COIN_DISPENSER_LOW,              SEVERITY_MAJOR   },
    {COIN_SENSER_ERROR,              SEVERITY_MAJOR   },
    {COIN_OTHER_ERROR,               SEVERITY_MAJOR   },
};
static s_system_error_t lcdscreen_errors[LCDSCREEN_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR  },
    {LCD_SCREEN_NOT_RESPONDING,      SEVERITY_CRITICAL},
    {LCD_SCREEN_ERROR,               SEVERITY_MAJOR   },
    {LCD_ILLUMINATION_ERROR,         SEVERITY_WARNING },
    {LCD_SCREEN_OTHER_ERROR,         SEVERITY_WARNING },
};
static s_system_error_t ledscreen_errors[LED_SCREEN_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR},
    {LED_SCREEN_NOT_RESPONDING,      SEVERITY_CRITICAL},
    {LED_SCREEN_OTHER_ERROR,         SEVERITY_CRITICAL},
};
static s_system_error_t door_errors[DOOR_ERR_NUM] =
{
    {0,                              SEVERITY_NO_ERR  },
    {UPPER_DOOR_SAFETY_ALARM,        SEVERITY_CRITICAL},
    {LOWER_DOOR_SAFETY_ALARM,        SEVERITY_CRITICAL},
    {BATTERY_DOOR_SAFETY_ALARM,      SEVERITY_CRITICAL},
    {UPPER_DOOR_OPEN,                SEVERITY_WARNING},
    {LOWER_DOOR_OPEN,                SEVERITY_WARNING},
    {BATTERY_DOOR_OPEN,              SEVERITY_WARNING},
    {UPPER_DOOR_CLOSE,               SEVERITY_WARNING},
    {LOWER_DOOR_CLOSE,               SEVERITY_WARNING},
    {BATTERY_DOOR_CLOSE,             SEVERITY_WARNING},
};
static s_system_error_t solarpanel_errors[SOLARPANEL_ERR_NUM] =
{
    {0,                                SEVERITY_NO_ERR  },
    {SOLARPANEL_NOT_RESPONDING,        SEVERITY_CRITICAL},
    {SOLARPANEL_UNIT_ERROR,            SEVERITY_MAJOR   },
};
static s_system_error_t ioctrl_errors[IOCTRL_ERR_NUM] =
{
    {0,                                SEVERITY_NO_ERR  },
    {IO_UNIT_NOT_RESPONDING,           SEVERITY_CRITICAL},
};
static s_system_error_t usb_errors[USB_ERR_NUM] =
{
    {0,                                SEVERITY_NO_ERR  },
    {USB1_ERROR,                       SEVERITY_MAJOR   },
    {USB2_ERROR,                       SEVERITY_MAJOR   },
    {USB3_ERROR,                       SEVERITY_MAJOR   },
};

static s_system_error_t * const sys_error[SYSTEM_NUMBER] = 	//	系统错误状态
{
    0,
    controller_errors,
    creditcard_errors,
    printer_errors,
    bill_errors,
    coin_errors,
    lcdscreen_errors,
    door_errors,
    solarpanel_errors,
    ioctrl_errors,
    usb_errors,
    ledscreen_errors,
    coindispenser_errors,
    billdispenser_errors,
    coindispenser2_errors,
};
#if SYSTEM_ERROR_DEBUG_EN
#define  ErrorTableCheck(module, index)    \
{\
                if (sys_error[index] != module)\
                {\
                    ErrorDebug("error",#module"'s id %d is error !", index);\
                }\
}
#else
#define ErrorTableCheck(module, index)   do{}while(0)
#endif

/**
 *  @breif   获取模块的错误类型数量
 */
static int GetErrorNum(e_alarm_code_t module_id)
{
    switch (module_id)
    {
        case SYSTEM_CONTROLLER      :  ErrorTableCheck(controller_errors, module_id); return CONTROLLER_ERR_NUM;
        case SYSTEM_CREDITCARD      :  ErrorTableCheck(creditcard_errors, module_id); return CREDITCARD_ERR_NUM; 
        case SYSTEM_PRINTER         :  ErrorTableCheck(printer_errors,    module_id); return PRINTER_ERR_NUM; 

        case SYSTEM_BILL_DISPENSER  :  ErrorTableCheck(billdispenser_errors,  module_id); return BILL_ERR_NUM;
        case SYSTEM_BILLCOLLECTOR   :  ErrorTableCheck(bill_errors,           module_id); return BILL_ERR_NUM;  
        
        case SYSTEM_COIN_DISPENSER  :  ErrorTableCheck(coindispenser_errors,  module_id); return COIN_ERR_NUM; 
        case SYSTEM_COIN_DISPENSER2 :  ErrorTableCheck(coindispenser2_errors, module_id); return COIN_ERR_NUM; 
        case SYSTEM_COIN_UNIT       :  ErrorTableCheck(coin_errors,       module_id); return COIN_ERR_NUM;  
        
        case SYSTEM_DISPLAY         :  ErrorTableCheck(lcdscreen_errors,  module_id); return LCDSCREEN_ERR_NUM;    
        case SYSTEM_DOOR            :  ErrorTableCheck(door_errors,       module_id); return DOOR_ERR_NUM;    
        case SYSTEM_SOLARPANEL      :  ErrorTableCheck(solarpanel_errors, module_id); return SOLARPANEL_ERR_NUM;  
        case SYSTEM_IOCTRL          :  ErrorTableCheck(ioctrl_errors,     module_id); return IOCTRL_ERR_NUM; 
        case SYSTEM_USB             :  ErrorTableCheck(usb_errors,        module_id); return USB_ERR_NUM;
        case SYSTEM_LEDSCREEN       :  ErrorTableCheck(ledscreen_errors,  module_id); return LED_SCREEN_ERR_NUM;  
        default:  return 0;
    }
}


/**
 *  @breif   检查数据结构是否正确赋值，如果没有正确赋值，则会死机, 在系统初始化时调用检测
 */
void CheckError(void)
{
    int i, j, len, err = 0;
    for (j=1; j< ALARM_CODE_SYSTEM_NUM; j++)
    {
        len = GetErrorNum((e_alarm_code_t)j);
        for (i=0; i<len; i++)
        {
            if (sys_error[j][i].id != i)
            {
                ErrorDebug("error","%d 's err_id %d is error !", j, i);
                err = 1;
            }
        }
        
    }
    while (err);
}

/**
 *  @breif   检查对应的模块和错误id是否正确
 */
static int8_t CheckErrorID(e_alarm_code_t module, uint8_t err_id)
{
    int err_max;
    if (((err_max = GetErrorNum(module)) == 0) || (err_id > err_max) || (sys_error[module][err_id].id != err_id))
    {
        return -1;
    }
    return 0;
}

/**
 *  @breif   设置对应的错误等级标识，标识对应等级的错误需要处理
 */
static void SetSeverityFlag(e_alarm_code_t module, uint8_t err_id)
{
	if (sys_error[module][err_id].severity == SEVERITY_CRITICAL)
	{
		severity_level_flag |= (SEVERITY_CRITICAL);
	}
	else if (sys_error[module][err_id].severity == SEVERITY_MAJOR)
	{
		severity_level_flag |= (SEVERITY_MAJOR);
	}
	else if (sys_error[module][err_id].severity == SEVERITY_WARNING)
	{
		severity_level_flag |= (SEVERITY_WARNING);
	}
}


/******************************************************************
** 函数名称:  ActivateSystemError
** 功能描述:  激活对应的错误，错误状态改为1，发送状态改为1(未发送)，当发送完成后发送状态变为0
** 输 入:     e_alarm_code_t module -- 错误类型，表示某一错误种类
**            uint8_t err_id -- 错误id，表示上面指的错误种类的错误类型
** 输 出:     0 -- 激活成功
**            -1 -- 激活失败，错误号错误
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
int8_t ActivateSystemError(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }

    if (sys_error[module][err_id].edge == 0)
    {
        sys_error[module][err_id].edge = 1;
        sys_error[module][err_id].counter++;
        sys_error[module][err_id].sendflag = 1;
        sys_error[module][err_id].happentime = time(0);
        SetSeverityFlag(module, err_id);
	}
    return 0;
}
/******************************************************************
** 函数名称:  ReActivateSystemError
** 功能描述:  重新激活所有的错误发送状态，发送状态改为1(未发送)，当发送完成后发送状态变为0
** 输 入:     无
** 输 出:     0 -- 激活成功
**            -1 -- 激活失败，错误号错误
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
int8_t ReActivateSystemError(void)
{
	uint8_t module, err_id, max_num;
	for (module = 1; module < SYSTEM_NUMBER; module++)
	{
        max_num = GetErrorNum((e_alarm_code_t)module);
        for (err_id=0; err_id<max_num; err_id++)
        {
            if (sys_error[module][err_id].edge)
    		{
    			sys_error[module][err_id].sendflag = 1;
    		    SetSeverityFlag((e_alarm_code_t)module, err_id);
            }
        }
	}
	return 0;
}
/******************************************************************
** 函数名称:  ClearSystemError
** 功能描述:  清除对应的错误状态，错误状态改为0，发送状态改为1(未发送)，当发送完成后发送状态变为0
** 输 入:     e_alarm_code_t module -- 错误类型，表示某一错误种类
**            uint8_t err_id -- 错误id，表示上面指的错误种类的错误类型
** 输 出:     0 -- 清除成功
**            -1 -- 清除失败，错误号错误
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
int8_t ClearSystemError(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
	if (sys_error[module][err_id].edge == 1)
	{
		//  错误被修正
	    sys_error[module][err_id].sendflag = 1;
	    sys_error[module][err_id].edge = 0;
        sys_error[module][err_id].happentime = time(0);
	    SetSeverityFlag(module, err_id);
	}
    return 0;
}
int8_t ClearModuleAllError(e_alarm_code_t module)
{
    int8_t max_num = GetErrorNum(module);
    int8_t err_id;
    if (max_num == 0)
    {
        return -1;
    }
    for (err_id=0; err_id<max_num; err_id++)
    {
    	if (sys_error[module][err_id].edge == 1)
    	{
    		//  错误被修正
    	    sys_error[module][err_id].sendflag = 1;
    	    sys_error[module][err_id].edge = 0;
            sys_error[module][err_id].happentime = time(0);
    	    SetSeverityFlag(module, err_id);
    	}
    }
    return 0;
}

/******************************************************************
** 函数名称:  SetSystemErrorSentFlag
** 功能描述:  清除未发送状态，发送状态改为0, 如果错误被修正，则该模块ID的错误清除
** 输 入:     e_alarm_code_t module -- 错误类型，表示某一错误种类
** 输 出:     0 -- 设置成功
**            -1 -- 设置失败，错误号错误
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
int8_t ClearErrorSentFlag(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
    sys_error[module][err_id].sendflag = 0;
    return 0;
}

/******************************************************************
** 函数名称:  GetErrorID
** 功能描述:  获取对应模块的错误id
** 输 入:     e_alarm_code_t module -- 错误模块，表示某一错误种类
**          state_mask -- 状态搜索条件，当为ERROR_GETID_MASK时不匹配错误状态
**          sendflag_mask -- 发送标志搜索条件，当为ERROR_GETID_MASK时不匹配发送标志
**          level  -- 错误等级搜索条件，当为SEVERITY_MASK时不匹配错误等级，支持多种错误等级匹配
**                    例如匹配warning和major      则输入SEVERITY_WARNING+SEVERITY_MAJOR
** 输 出:   >0 模块的错误ID
*            0 没有错误
*           <0 参数错误
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
int8_t GetErrorID(e_alarm_code_t module, uint8_t state_mask, uint8_t sendflag_mask, uint8_t level)
{ 
    int err_max, i;
    if ((err_max = GetErrorNum(module)) == 0)
    {
        return -1;
    }
    for (i=1; i<err_max; i++)
    {
        if ((state_mask == ERROR_GETID_MASK) || (sys_error[module][i].edge == state_mask))
        {
            if ((sendflag_mask == ERROR_GETID_MASK) || (sys_error[module][i].sendflag == sendflag_mask))
            {
                if ((level == SEVERITY_MASK) || (sys_error[module][i].severity & level))
                {
                    return i;
                }
            }
        }
    }
    return 0;
    
}



/**
  * @brief  获取对应错误的等级
  * @param  module: 可以检测错误的模块id
  * @param  err_id: 对应错误的错误id                    
  * @retval SEVERITY_NO_ERR: 没有错误
  *         SEVERITY_WARNING:   警告
  *         SEVERITY_MAJOR:     主要的错误
  *         SEVERITY_CRITICAL:  紧急错误
  */
int8_t GetErrorSeverity(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
    return sys_error[module][err_id].severity;
}

/**
  * @brief  获取对应错误的发送状态
  * @param  module: 可以检测错误的模块id
  * @param  err_id: 对应错误的错误id                    
  * @retval 0: 未发送
  *         1: 已发送
  */
int8_t GetErrorSendFlag(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
    return sys_error[module][err_id].sendflag;
}

/**
  * @brief  获取对应错误的错误次数
  * @param  module: 可以检测错误的模块id
  * @param  err_id: 对应错误的错误id                    
  * @retval 0: 未发送
  *         1: 已发送
  */
int8_t GetErrorState(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
    return sys_error[module][err_id].edge;
}

/**
  * @brief  获取对应错误的发生次数
  * @param  module: 可以检测错误的模块id
  * @param  err_id: 对应错误的错误id                    
  * @retval 发生错误的次数 
  */
int16_t GetErrorCount(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return -1;
    }
    return sys_error[module][err_id].counter;
}


/**
  * @brief  获取对应错误的发生时间
  * @param  module: 可以检测错误的模块id
  * @param  err_id: 对应错误的错误id                    
* @retval 发生错误的时间 0: 参数错误
  */
uint32_t GetErrorHappenTime(e_alarm_code_t module, uint8_t err_id)
{
    if (CheckErrorID(module, err_id))
    {
        return 0;
    }
    return sys_error[module][err_id].happentime;
}


/******************************************************************
** 函数名称:  GetErrorSeverityFlag
** 功能描述:  获取对应错误优先级是否有产生错误
** 输 入:     e_sys_err_severity_oam_t flag -- 错误优先级
** 输 出:     对应错误优先级的错误状态
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
uint8_t GetErrorSeverityFlag(e_sys_err_severity_t flag)
{
    return (severity_level_flag & (flag));
}



/******************************************************************
** 函数名称:  ClearErrorSeverityFlag
** 功能描述:  清除对应错误优先级错误状态
** 输 入:     e_sys_err_severity_oam_t flag -- 错误优先级
** 输 出:     错误优先级的错误状态
** 全局变量:  无
** 调用模块:  无
** 备注:
** 作 者:     杜其俊/aiven
** 日 期:     2014.04.23
******************************************************************/
uint8_t ClearErrorSeverityFlag(e_sys_err_severity_t flag)
{
    severity_level_flag &= ~(flag);
    return severity_level_flag;
}


#endif
