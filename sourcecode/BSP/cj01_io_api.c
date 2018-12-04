#include "cj01_io_api.h"
#include "bsp_includes.h"
#include "spi.h"
#include "lwip\ip_addr.h"

#define IOFunc(port)    port##Open,##port##Close,##port##Read,##port##Write,##port##Ioctl,0,0 

//GPIO端口接口管理数组
static s_ioGroup_t ioGroup[PORT_GROUP_END];
//BSP层操作数组
const s_IoOpreations_t io_opreations[PORT_END] =
{
//    端口号		读写权限		 IO组           打开函数		关闭函数		读取函数		写入函数		控制函数    异步读,  异步写
    { RTC_PORT,		READ_WRITE,		&ioGroup[gRTC_PORT],        IOFunc(Rtc)         },
    { COM1,		    READ_WRITE,		&ioGroup[gCOM1],            IOFunc(Uart)        },
    { COM6,		    READ_WRITE,		&ioGroup[gCOM6],            IOFunc(Uart)        },
    { COM3,	    	READ_WRITE,		&ioGroup[gCOM3],            IOFunc(Uart)        },
    { GPIO_IN_01,   READ_ONLY,    	&ioGroup[gGPIO_IN_01],      IOFunc(GPIO)		},
    { GPIO_IN_02,   READ_ONLY,    	&ioGroup[gGPIO_IN_02],      IOFunc(GPIO)		},
    { GPIO_IN_03,   READ_ONLY,    	&ioGroup[gGPIO_IN_03],      IOFunc(GPIO)		},
    { GPIO_IN_04,   READ_ONLY,    	&ioGroup[gGPIO_IN_04],      IOFunc(GPIO)		},
    { GPIO_IN_05,   READ_ONLY,    	&ioGroup[gGPIO_IN_05],      IOFunc(GPIO)		},
    { GPIO_IN_06,   READ_ONLY,    	&ioGroup[gGPIO_IN_06],      IOFunc(GPIO)		},
    { GPIO_IN_07,   READ_ONLY,    	&ioGroup[gGPIO_IN_07],      IOFunc(GPIO)		},
    { GPIO_IN_08,   READ_ONLY,    	&ioGroup[gGPIO_IN_08],      IOFunc(GPIO)		},
    { GPIO_OUT_01,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_01],     IOFunc(GPIO)		},
    { GPIO_OUT_02,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_02],     IOFunc(GPIO)		},
    { GPIO_OUT_03,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_03],     IOFunc(GPIO)		},
    { GPIO_OUT_04,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_04],     IOFunc(GPIO)		},
    { GPIO_OUT_05,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_05],     IOFunc(GPIO)		},
    { GPIO_OUT_06,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_06],     IOFunc(GPIO)		},
    { GPIO_OUT_07,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_07],     IOFunc(GPIO)		},
    { GPIO_OUT_08,  WRITE_ONLY,    	&ioGroup[gGPIO_OUT_08],     IOFunc(GPIO)		},
    { ST_LED,       WRITE_ONLY,    	&ioGroup[gSTLED],           IOFunc(GPIO)		},
    { GPIO_IN_MENU, READ_ONLY,    	&ioGroup[gGPIO_IN_MENU],    IOFunc(GPIO)		},
    { WDT,          READ_ONLY,    	&ioGroup[gWDT],             IOFunc(Wdt_)		},
    { PS2,          READ_ONLY,    	&ioGroup[gPS2],             IOFunc(Ps2key_)	    },
    { INT_FLASH,    READ_ONLY,    	&ioGroup[gINT_FLASH],       IOFunc(Flash_)	    },
    { MINI_PCIE,    READ_WRITE,     &ioGroup[gCOM3],            IOFunc(MiniPcie)    },
};




//uint32_t G_test1;
//uint32_t G_test2;

//  检查 io_opreations 的参数是否正确, 并且初始化 io_list
void CheckIoOpreations(void)
{
    int i; 
    for (i = 0; i < PORT_END; i++)
    {
        while (io_opreations[i].port != i);
        io_opreations[i].group->io_sem = NULL;
        io_opreations[i].group->task_prio = 0xFF;
    }
}


/*****************************************************************************************
** 函 数 名 称 ：CheckPort
** 函 数 功 能 ：校验接口端口号的合法性
** 输 入 ：port ：接口的端口号
** 输 出 ：0：接口的端口号合法
**       ：-1：接口的端口号不合法
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月17日
*****************************************************************************************/

int8_t CheckPort(int32_t port)
{
    int8_t ret = 0;

    if ((port < 0) || (port >= PORT_END))
    {
        ret = -1;
    }

    return (int8_t)ret;
}


/*****************************************************************************************
** 函 数 名 称 ：IoOpen
** 函 数 功 能 ：打开接口(申请对port接口操作)并且初始化设备
** 输 入 ：port ：接口端口号
**       ：config：初始化配置参数
**       ：config_len：config参数的长度
** 输 出 ：0：打开成功
**       ：-1：端口号非法
**       ：-2：接口初始化失败
**       ：-3：获取该IO口资源超时
**       ：-4：打开权限不支持
**       ：-5：创建该IO口的sem失败
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月17日
*****************************************************************************************/

int8_t IoOpen(int32_t port, const void *config, uint8_t config_len)
{
    uint8_t err;
    uint8_t prio;

    prio = GetCurrentTaskPrio();

    if (CheckPort(port) == -1)//端口不合法
    {
        return -1;
    }

    if (io_opreations[port].group->io_sem == NULL)
    {
        io_opreations[port].group->io_sem = OSSemCreate(1);

        if (io_opreations[port].group->io_sem == NULL)
        {
            return  -5;
        }
    }

    //G_test1 = port;
    //G_test2 = prio;

    OSSemPend(io_opreations[port].group->io_sem, 0, &err); //等待资源

    if (err == OS_ERR_TIMEOUT)
    {
        return -3;
    }

    if ((io_opreations[port].io_open) && (io_opreations[port].io_open(port, (void *)config, config_len) < 0))
    {
        OSSemPost(io_opreations[port].group->io_sem);
        //io_opreations[port].group->task_prio = 0xFF;
        return -2;
    }

    io_opreations[port].group->task_prio = prio;

    return 0;
}


/*****************************************************************************************
** 函 数 名 称 ：IoClose
** 函 数 功 能 ：打开设备(申请对port接口操作)并且初始化设备
** 输 入 ：port ：接口端口号
** 输 出 ：1：端口已经关闭
**       ：0：关闭成功
**       ：-1：端口号非法
**       ：-2：关闭失败
**       ：-3：不是当前任务打开的端口不能关闭
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/

int8_t IoClose(int32_t port)
{
    if (CheckPort(port) < 0)//端口非法
    {
        return -1;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if ((io_opreations[port].io_close) && (io_opreations[port].io_close(port) < 0))
    {
        return -2;
    }

    io_opreations[port].group->task_prio = 0xFF;
    OSSemPost(io_opreations[port].group->io_sem);

    return 0;
}


/*****************************************************************************************
** 函 数 名 称 ：IoRead
** 函 数 功 能 ：通过该端口读取数据，成功返回读取的字节数，失败返回负数
** 输 入 ：port ：接口端口号
**       ：index：读取的索引地址
**       ：ret_buf：存放读取的数据
** 输 出 ：>0：读取的字节数
**       ：-1：读取失败
**       ：-2：端口号非法
**       ：-3：端口被其他进程打开
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/
int32_t IoRead(int32_t port, void *buf, uint32_t buf_len)
{
    if (CheckPort(port) < 0)//端口非法
    {
        return -2;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if (io_opreations[port].io_read)
    {
        return io_opreations[port].io_read(port, buf, buf_len);
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************************
** 函 数 名 称 : IoAsynchronRead
** 函 数 功 能 ：通过该端口读取数据，成功返回读取的字节数，失败返回负数
** 输 入 ：port ：接口端口号
**       ：index：读取的索引地址
**       ：ret_buf：存放读取的数据
**       : callback: 读完成后的回调函数
** 输 出 ：>0：读取的字节数
**       ：-1：读取失败
**       ：-2：端口号非法
**       ：-3：端口被其他进程打开
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/
int32_t IoAsynchronRead(int32_t port, void *buf, uint32_t buf_len, void * callback)
{
    if (CheckPort(port) < 0)//端口非法
    {
        return -2;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if (io_opreations[port].io_asyn_read)
    {
        return io_opreations[port].io_asyn_read(port, buf, buf_len, callback);
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************************
** 函 数 名 称 ：IoWrite
** 函 数 功 能 ：向该端口写数据
** 输 入 ：port ：接口端口号
**       ：buf：要写入的数据
**       ：buf_len：写入数据的长度
** 输 出 ：0：写入成功
**       ：-1：写入失败
**       ：-2：端口号非法
**       ：-3：端口被其他进程打开
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/

int8_t IoWrite(int32_t port, void *buf, uint32_t buf_len)
{
    if (CheckPort(port) < 0)//端口非法
    {
        return -2;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if (io_opreations[port].io_write)
    {
        return io_opreations[port].io_write(port, buf, buf_len);
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************************
** 函 数 名 称 : IoAsynchronWrite
** 函 数 功 能 ：向该端口写数据
** 输 入 ：port ：接口端口号
**       ：buf：要写入的数据
**       ：buf_len：写入数据的长度
**       : callback: 读完成后的回调函数
** 输 出 ：0：写入成功
**       ：-1：写入失败
**       ：-2：端口号非法
**       ：-3：端口被其他进程打开
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/

int8_t IoAsynchronWrite(int32_t port, void *buf, uint32_t buf_len, void * callback)
{
    if (CheckPort(port) < 0)//端口非法
    {
        return -2;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if (io_opreations[port].io_asyn_write)
    {
        return io_opreations[port].io_asyn_write(port, buf, buf_len, callback);
    }
    else
    {
        return -1;
    }
}


/*****************************************************************************************
** 函 数 名 称 ：IoIoctl
** 函 数 功 能 ：通过命令码控制该接口
** 输 入 ：port ：接口端口号
**       ：cmd：命令码
**       ：...：可变参数，根据命令是否传入参数使用
** 输 出 ：0：操作成功
**       ：-1：操作失败
**       ：-2：端口号非法
**       ：-3：端口被其他进程打开
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月18日
*****************************************************************************************/

int8_t Ioctl(int32_t port, uint32_t cmd, ...)
{
    int8_t err;
    va_list argp;                   /* 定义保存函数参数的结构 */

    if (CheckPort(port) < 0)//端口非法
    {
        return -2;
    }

    if (io_opreations[port].group->task_prio != GetCurrentTaskPrio())
    {
        return -3;
    }

    if (io_opreations[port].io_ioctl)
    {
        va_start(argp, cmd);
        err = io_opreations[port].io_ioctl(port, cmd, argp);
        va_end(argp);                                     // 将argp置为NULL
        return err;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************************
** 函 数 名 称 ：GetCurrentTaskPrio
** 函 数 功 能 ：获取当前任务的优先级
** 输 入 ：
** 输 出 ：返回当前任务的优先级
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月26日
*****************************************************************************************/

uint8_t GetCurrentTaskPrio(void)
{
    return OSTCBCur->OSTCBPrio;
}

extern int8_t InitLocalNet(uint8_t dhcp, ip_addr_t ipaddr, ip_addr_t netmask, ip_addr_t gateway);
extern void TaskModbus(void);
extern void ChipCommInit(void);
void BSPInit(void)
{
    SysTimeInit();
    GPIOInit();
    ChipCommInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //中断分组配置
    MX_FATFS_Init();
    DMA_AllInit(); 
    ADC_AllInit();
}

void AppInit(void);

/************************************************************************************
** 函 数 名 称 : TaskBsp
** 函 数 功 能 ：处理需要单独处理的BSP级任务，比如IO口滤波
** 输 入 ：
** 输 出 ：返回当前任务的优先级
** 全局变量:
** 调用模块:
**
** 作　者: 赵国刚
** 日　期: 2014年2月26日
*****************************************************************************************/
OS_FLAG_GRP *bsp_os_flag;
uint8_t  openddd[14];
void TaskBsp(void *pdata)
{
    uint8_t err;
    ip_addr_t ip = {0};
    u_bsp_os_flag_t bsp_flag;
    pdata = pdata;
    CheckIoOpreations();
    BSPInit();
    //OSStatInit();
    InitLocalNet(1, ip, ip, ip);
    bsp_os_flag = OSFlagCreate(0, &err);

    while (bsp_os_flag == 0);
   //创建初始化任务
    AppInit(); 
    while (1)
    {
        //  BSP 进程功能代码
        //  获取标志
        bsp_flag.bsp_os_flag_word = OSFlagPend(bsp_os_flag, 0xFFFF, OS_FLAG_WAIT_SET_ANY, 0, &err);
        //  清掉标志
        OSFlagPend(bsp_os_flag, 0xFFFF, OS_FLAG_CONSUME + OS_FLAG_WAIT_SET_ANY, 0, &err);


        if (bsp_flag.bsp_os_flag.timer_1ms)
        {
            GPIODebounce();
            TaskModbus();
            CUR_DataToVoltage();
            CheckWWANState();
        }
    }
}


__weak void AppInit(void)
{
}

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/

