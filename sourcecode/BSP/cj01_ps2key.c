#include	"..\bsp_includes.h"
#include    "queue.h"
#include    "cj01_ps2key.h"
#include "cj01_IRQ_priority.h" 
static struct
{
    uint32_t g_keybit : 4;//KEY的位数
    uint32_t g_oddparity : 4;//奇偶校验位
    uint32_t g_keydata : 8;//KEY的数值
    uint32_t g_key_old : 8;//上一次接收到的key值
    uint32_t ps2_nodata_delay : 8;
} ps2_static_data;


static uint8_t ps2_io_buffer[64];

/*****************************************************************************************
** 函数名称 ：	PS2Timer
** 函数功能 ：  PS2键盘的定时器处理，需要每5MS处理一次
** 输    入 ：	void
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void PS2Timer(void)
{
    if (ps2_static_data.ps2_nodata_delay < 100)
    {
        ps2_static_data.ps2_nodata_delay++;
    }
}


/*****************************************************************************************
** 函数名称 ：	Ps2key_Open
** 函数功能 ：	打开PS2端口
** 输    入 ：	int32_t port	：PS2端口号
** 输    入 ：	void * config	：配置参数，无
** 输    入 ：	uint8_t len		：配置参数的长度
** 输    出 ：	int8_t    		0：操作成功	 -1：操作失败
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Ps2key_Open(int32_t port, const void *config, uint8_t len)
{
    uint8_t err;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    NVIC_InitTypeDef   NVIC_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(PS2_GPIO_CLK_CLK, ENABLE);//使能GPIO时钟
    RCC_AHB1PeriphClockCmd(PS2_GPIO_DAT_CLK, ENABLE);//使能GPIO时钟

    GPIO_InitStructure.GPIO_Pin = PS2_GPIO_PIN_CLK;     //设置PS2的CLK口为输入  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PS2_GPIO_PORT_CLK, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PS2_GPIO_PIN_DAT;     //设置PS2的DAT口为输入  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PS2_GPIO_PORT_DAT, &GPIO_InitStructure);


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    SYSCFG_EXTILineConfig(PS2_EXTI_PORTSOURCE, PS2_EXTI_PINSOURCE);

    EXTI_InitStructure.EXTI_Line = PS2_EXTI_LINE;                     //CLK为外部中断口
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  //下降沿,上升沿中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);


    NVIC_InitStructure.NVIC_IRQChannel = PS2_IRQ_N;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PS2_IRQ_CHANNEL_PREEMPTION_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = PS2_IRQ_CHANNEL_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    QueueCreateInline(ps2_io_buffer, sizeof(ps2_io_buffer), NULL, NULL, err);
    if (err == QUEUE_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************************
** 函数名称 ：	Ps2key_Read
** 函数功能 ：	读出PS2端口上的数据
** 输    入 ：	int32_t port			：PS2端口号
** 输    入 ：	void * buf				：指向读出数据
** 输    入 ：	uint16_t buf_len		：读出数据的长度
** 输    出 ：	int16_t    			1：操作成功 0：操作失败
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int32_t Ps2key_Read(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t err = 0;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    QueueReadInline(ps2_io_buffer, buf, err);
    if (err == QUEUE_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************************
** 函数名称 ：	Ps2key_Close
** 函数功能 ：
** 输    入 ：	int32_t port
** 输    出 ：	int8_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Ps2key_Close(int32_t port)
{
    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Ps2key_Write
** 函数功能 ：
** 输    入 ：	int32_t port
** 输    入 ：	void * buf
** 输    入 ：	uint16_t buf_len
** 输    出 ：	int8_t
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Ps2key_Write(int32_t port, void *buf, uint32_t buf_len)
{
    //IO0DIR |= (PS2DAT + PS2CLK);
    //IO0CLR |= (PS2DAT + PS2CLK);
    //Delay10us(10);
    //IO0DIR &= (~PS2CLK);
    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Ps2key_Ioctl
** 函数功能 ：	对PS2端口口操作控制
** 输    入 ：	int32_t port				IO端口号
** 输    入 ：	uint32_t cmd				读写权限
** 输    入 ：	va_list argp				要读、写的数据   要读、写的数据长度
** 输    出 ：	int8_t    			        0：操作成功 -1：操作失败
** 全局变量 :
** 调用模块 :   Ps2key_Write，Ps2key_Read
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Ps2key_Ioctl(int32_t port, uint32_t cmd, va_list argp)
{
    int8_t ret = -1;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    switch (cmd)
    {
    case CLEAR_BUFFER:
        QueueFlushInline(ps2_io_buffer);
    default:
        break;
    }
    return ret;
}



/*****************************************************************************************
** 函数名称 ：	PS2_KEY_IRQHandler
** 函数功能 ：	按键中断处理函数，放在外部中断之中运行
** 输    入 ：	void
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void PS2_KEY_IRQHandler(void)
{
    uint8_t err;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    err = err;

    if (GPIO_ReadInputDataBit(PS2_GPIO_PORT_CLK, PS2_GPIO_PIN_CLK) == RESET)
    {
        //数据校验正确性
        if (ps2_static_data.ps2_nodata_delay >= 10)
        {
            ps2_static_data.g_keybit = 0;
            ps2_static_data.g_keydata = 0;
            ps2_static_data.g_oddparity = 0;
        }
        ps2_static_data.ps2_nodata_delay = 0;
        ps2_static_data.g_keybit++;
        if (ps2_static_data.g_keybit == 1)
        {
            if (GPIO_ReadInputDataBit(PS2_GPIO_PORT_DAT, PS2_GPIO_PIN_DAT) == SET)//数据位为1,即启始位不为0，为错误
            {
                ps2_static_data.g_keybit = 0;
                ps2_static_data.g_keydata = 0;
                //g_key_asiic = 0;
                ps2_static_data.g_oddparity = 0;
            }
        }
        else if ((ps2_static_data.g_keybit > 1) && (ps2_static_data.g_keybit < 10))//接收8位数据
        {
            ps2_static_data.g_keydata = ps2_static_data.g_keydata >> 1;
            if (GPIO_ReadInputDataBit(PS2_GPIO_PORT_DAT, PS2_GPIO_PIN_DAT) == SET)//数据位为1
            {
                ps2_static_data.g_keydata = ps2_static_data.g_keydata | 0x80;
                ps2_static_data.g_oddparity++;
            }
        }
        else if (ps2_static_data.g_keybit == 10)
        {
            if (GPIO_ReadInputDataBit(PS2_GPIO_PORT_DAT, PS2_GPIO_PIN_DAT) == SET)//数据位为1,即奇校验,有偶数个1
            {
                if (ps2_static_data.g_oddparity & 0x01)
                {
                    ps2_static_data.g_keybit = 0;
                    ps2_static_data.g_keydata = 0;
                    //g_key_asiic = 0;
                }
            }
            else//数据位为0,即奇校验,有奇数个1
            {
                if (!(ps2_static_data.g_oddparity & 0x01))
                {
                    ps2_static_data.g_keybit = 0;
                    ps2_static_data.g_keydata = 0;
                    //g_key_asiic = 0;
                }
            }
            ps2_static_data.g_oddparity = 0;
        }
        else if (ps2_static_data.g_keybit == 11)
        {
            if (GPIO_ReadInputDataBit(PS2_GPIO_PORT_DAT, PS2_GPIO_PIN_DAT) != SET)//数据位为1,即停止位不为1，为错误
            {
                ps2_static_data.g_keybit = 0;
                ps2_static_data.g_keydata = 0;
                //g_key_asiic = 0;
            }
            else
            {
                ps2_static_data.g_keybit = 0;//当中断11次后表示一帧数据收完，清变量准备下一次接收
                ps2_static_data.g_oddparity = 0;
                err = ps2_static_data.g_keydata;
                //Uart0_Write(0, &err, 1);
                if (ps2_static_data.g_key_old != ps2_static_data.g_keydata)
                {
                    QueueWriteInline(ps2_io_buffer, ps2_static_data.g_keydata, err);
                }
                if (ps2_static_data.g_key_old == 0xF0)
                {
                    ps2_static_data.g_key_old = 0;
                }
                else
                {
                    ps2_static_data.g_key_old = ps2_static_data.g_keydata;
                }
                //g_key_asiic = Ps2_KeyDeCode(ps2_static_data.g_keydata);//解按键码
                ps2_static_data.g_keydata = 0;
            }
        }
    }
}

