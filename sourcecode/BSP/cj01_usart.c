#include "cj01_usart.h"
#include "bsp_includes.h"
#include "queue.h"
#include "cj01_IRQ_priority.h" 
uint32_t UART1RecvBuf[512];
uint32_t UART1SendBuf[64];
uint32_t UART2RecvBuf[64];
uint32_t UART2SendBuf[64];
uint32_t UART3RecvBuf[64];
uint32_t UART3SendBuf[64];
uint32_t UART6RecvBuf[64];
uint32_t UART6SendBuf[64];



uint8_t usart_mode = 0; //串口3工作模式，0为串口模式，1为MDB模式

typedef struct
{
    uint8_t *in;
    uint8_t *out;
    uint16_t dat_len;
    uint16_t buf_len;
    uint8_t buf[4];
} s_uart_queue_t;

//初始化串口
int8_t UartOpen(int32_t port, const void *config, uint8_t len)
{
    s_UartStr_t *data = NULL;
    
    GPIO_TypeDef *GPIO_PORT_TX = NULL, *GPIO_PORT_RX = NULL;    //GPIO端口
    uint16_t GPIO_PIN_TX, GPIO_PIN_RX;  //引脚PIN
    uint8_t GPIO_PinSource_TX, GPIO_PinSource_RX; //复用端口
    uint8_t GPIO_AF_USART;  //复用串口
    USART_TypeDef *USARTPort = NULL;   //串口端口
    uint8_t  USART_IRQn;   //中断通道
    uint8_t IRQChannelPreemptionPriority;   //抢占优先级
    uint8_t IRQChannelSubPriority;//子优先级
    DataQueue *recv_queue_temp = NULL;
    uint16_t recv_queue_buff_len=0;
    DataQueue *send_queue_temp = NULL;
    uint16_t send_queue_buff_len=0;
    
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    if (port == COM1)
    {
        GPIO_PORT_TX = GPIOA; //发送GPIO端口
        GPIO_PORT_RX = GPIOA;   //接收GPIO端口
        GPIO_PinSource_TX = GPIO_PinSource9;//发送复用端口
        GPIO_PinSource_RX = GPIO_PinSource10; //接收复用端口
        GPIO_AF_USART = GPIO_AF_USART1;//复用串口
        GPIO_PIN_TX = GPIO_PIN_9;//发送引脚PIN
        GPIO_PIN_RX = GPIO_PIN_10; //接收引脚PIN
        USARTPort = USART1;//串口端口
        USART_IRQn = USART1_IRQn;//中断通道
        IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0;//抢占优先级
        IRQChannelSubPriority = SUB_PRIORITY_USART1;//子优先级
        recv_queue_temp = (DataQueue *)UART1RecvBuf;
        send_queue_temp = (DataQueue *)UART1SendBuf;
        recv_queue_buff_len = sizeof(UART1RecvBuf);
        send_queue_buff_len = sizeof(UART1SendBuf);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOA时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //使能USART1时钟
    }
/*    else if (port == COM2)
    {
        GPIO_PORT_TX = GPIOA; //发送GPIO端口
        GPIO_PORT_RX = GPIOA;   //接收GPIO端口
        GPIO_PinSource_TX = GPIO_PinSource2;//发送复用端口
        GPIO_PinSource_RX = GPIO_PinSource3; //接收复用端口
        GPIO_AF_USART = GPIO_AF_USART2;//复用串口
        GPIO_PIN_TX = GPIO_PIN_2;//发送引脚PIN
        GPIO_PIN_RX = GPIO_PIN_3; //接收引脚PIN
        USARTPort = USART2;//串口端口
        USART_IRQn = USART2_IRQn;//中断通道
        IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0;//抢占优先级
        IRQChannelSubPriority = SUB_PRIORITY_USART2;//子优先级
        recv_queue_temp = (DataQueue *)UART2RecvBuf;
        send_queue_temp = (DataQueue *)UART2SendBuf;
        recv_queue_buff_len = sizeof(UART2RecvBuf);
        send_queue_buff_len = sizeof(UART2SendBuf);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOB时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //使能USART3时钟
    }*/
    else if (port == COM6)
    {
        GPIO_PORT_TX = GPIOC; //发送GPIO端口
        GPIO_PORT_RX = GPIOC;   //接收GPIO端口
        GPIO_PinSource_TX = GPIO_PinSource6;//发送复用端口
        GPIO_PinSource_RX = GPIO_PinSource7; //接收复用端口
        GPIO_AF_USART = GPIO_AF_USART6;//复用串口
        GPIO_PIN_TX = GPIO_PIN_6;//发送引脚PIN
        GPIO_PIN_RX = GPIO_PIN_7; //接收引脚PIN
        USARTPort = USART6;//串口端口
        USART_IRQn = USART6_IRQn;//中断通道
        IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0;//抢占优先级
        IRQChannelSubPriority = SUB_PRIORITY_USART6;//子优先级
        recv_queue_temp = (DataQueue *)UART6RecvBuf;
        send_queue_temp = (DataQueue *)UART6SendBuf;
        recv_queue_buff_len = sizeof(UART6RecvBuf);
        send_queue_buff_len = sizeof(UART6SendBuf);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //使能GPIOA时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); //使能USART2时钟
    }
	else if (port == COM3)
	{
		GPIO_PORT_TX = GPIOB; //发送GPIO端口
		GPIO_PORT_RX = GPIOB;   //接收GPIO端口
		GPIO_PinSource_TX = GPIO_PinSource10;//发送复用端口
		GPIO_PinSource_RX = GPIO_PinSource11; //接收复用端口
		GPIO_AF_USART = GPIO_AF_USART3;//复用串口
		GPIO_PIN_TX = GPIO_PIN_10;//发送引脚PIN
		GPIO_PIN_RX = GPIO_PIN_11; //接收引脚PIN
		USARTPort = USART3;//串口端口
		USART_IRQn = USART3_IRQn;//中断通道
		IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0;//抢占优先级
		IRQChannelSubPriority = SUB_PRIORITY_USART3;//子优先级
		recv_queue_temp = (DataQueue *)UART3RecvBuf;
		send_queue_temp = (DataQueue *)UART3SendBuf;
		recv_queue_buff_len = sizeof(UART3RecvBuf);
		send_queue_buff_len = sizeof(UART3SendBuf);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //使能GPIOB时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3时钟
        
        //串口通道选择
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIO时钟
        GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1;//
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
        GPIO_ResetBits(GPIOB,GPIO_PIN_0);   //选择通道00
        GPIO_ResetBits(GPIOB,GPIO_PIN_1);
        OSTimeDly(OS_TICKS_PER_SEC/200);
//        GPIO_SetBits(GPIOB,GPIO_PIN_0);
//        GPIO_SetBits(GPIOB,GPIO_PIN_1);
	}
    else
    {
        return -1;
    }

    //GPIO端口设置

    data = (s_UartStr_t *)config;
    
    //串口对应引脚复用映射
    GPIO_PinAFConfig(GPIO_PORT_TX, GPIO_PinSource_TX, GPIO_AF_USART); //GPIO_TX复用为USART
    GPIO_PinAFConfig(GPIO_PORT_RX, GPIO_PinSource_RX, GPIO_AF_USART); //GPIO_RX复用为USART

    //IO口模式配置
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_TX; //TX引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIO_PORT_TX, &GPIO_InitStructure); //初始化TX引脚

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RX; //RX引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIO_PORT_RX, &GPIO_InitStructure); //初始化RX引脚


    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = data->baud;//波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    if (data->stop_b == 1)
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    }
    else if (data->stop_b == 2)
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;//两个停止位
    }
    else
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    }

    if (data->parity == 0)
    {
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    }
    else if (data->parity == 1)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;//使用奇偶校验需要为9位数据格式
        USART_InitStructure.USART_Parity = USART_Parity_Odd;//奇校验
    }
    else if (data->parity == 2)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;//使用奇偶校验需要为9位数据格式
        USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验
    }
    else if (data->parity == 3)
    {
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    }
    else
    {
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    }
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
    USART_Init(USARTPort, &USART_InitStructure); //初始化串口


    USART_Cmd(USARTPort, ENABLE);  //使能串口

    USART_ClearFlag(USARTPort, USART_FLAG_TC);

    USART_ITConfig(USARTPort, USART_IT_RXNE, ENABLE);//开启相关中断

    //Usart NVIC 中断配置
    NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;//串口中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQChannelPreemptionPriority; //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQChannelSubPriority;      //子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

    //初始化串口队列
    if (QueueCreate((void *)recv_queue_temp,recv_queue_buff_len,NULL,NULL) == NOT_OK)
    {
        return -1;
    }
    if (QueueCreate((void *)send_queue_temp,send_queue_buff_len,NULL,NULL) == NOT_OK)
    {
        return -1;
    }

    return 1;   //串口打开成功
}

/*****************************************************************************************
** 函 数 名 称 ：Uart0_Close
** 函 数 功 能 ：关闭相应的端口，释放资源
** 输 入 ： port：IO端口号
** 输 出 ： 0：操作成功
**       ： -1：操作失败
** 全局变量:
** 调用模块:
**
** 作　者: Arjun
** 日　期: 2014年3月11日
*****************************************************************************************/

int8_t UartClose(int32_t port)
{
    DataQueue *queue_temp = 0;
    uint8_t temp;
    USART_TypeDef *GPIOPort;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif

    if (port == COM1)
    {
        queue_temp = (DataQueue *)UART1SendBuf;
        GPIOPort = USART1;
    }
    else if ((port == COM3))
    {
        queue_temp = (DataQueue *)UART3SendBuf;
        GPIOPort = USART3;
    }
    else if (port == COM6)
    {
        queue_temp = (DataQueue *)UART6SendBuf;
        GPIOPort = USART6;
    }
    else
    {
        return -1;
    }
    while (QueueNData(queue_temp) != 0)
    {
        if((USART_GetFlagStatus(GPIOPort, USART_FLAG_TXE) != RESET))
        {
            OS_ENTER_CRITICAL();
            QueueRead(&temp, queue_temp);         //发送最初入队的数据/
            OS_EXIT_CRITICAL();
            USART_SendData(GPIOPort, (uint8_t) temp); //通过外设 USART 发送单个数据 
            USART_ITConfig(GPIOPort, USART_IT_TC, ENABLE);//使能发送中断
        }
        OSTimeDly(OS_TICKS_PER_SEC/200);
    }
    return 0;
}


/*****************************************************************************************
** 函 数 名 称 ：Uart0_Write
** 函 数 功 能 ：向Uart0口写数据
** 输 入 ： port：IO端口号
**       ： buf：要写的数据
**       ： len：要写的数据长度
** 输 出 ： 0：操作成功
**       ： -1：操作失败
** 全局变量:
** 调用模块:Uart0Putch
**
** 作　者: Arjun
** 日　期: 2014年3月11日
*****************************************************************************************/

int8_t UartWrite(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t *data = (uint8_t *)buf;
    DataQueue *queue_temp = 0;
    uint8_t temp;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    USART_TypeDef *GPIOPort;

    if (port == COM1)
    {
        queue_temp = (DataQueue *)UART1SendBuf;
        GPIOPort = USART1;
    }
	else if ((port == COM3))
    {
        queue_temp = (DataQueue *)UART3SendBuf;
        GPIOPort = USART3;
    }
    else if (port == COM6)
    {
        queue_temp = (DataQueue *)UART6SendBuf;
        GPIOPort = USART6;
    }
    else
    {
        return -1;
    }
    OS_ENTER_CRITICAL();
    while (buf_len-- > 0)
    {
        if (QueueWrite((void *)queue_temp, *data++) != QUEUE_OK) //数据入队
        {
            //如果数据入队失败，说明队列满了，等待队列中的数据发送出去
            data--;
			buf_len++;
            OS_EXIT_CRITICAL();
            OSTimeDly(OS_TICKS_PER_SEC/100);
            OS_ENTER_CRITICAL();
        }
    }
    if(USART_GetFlagStatus(GPIOPort, USART_FLAG_TXE) != RESET)                
    {
        //UART1发送保持寄存器空
        QueueRead(&temp, queue_temp);         //发送最初入队的数据/
        USART_SendData(GPIOPort, (uint8_t) temp); //通过外设 USART 发送单个数据 
        USART_ITConfig(GPIOPort, USART_IT_TC, ENABLE);//使能发送中断
    }
    OS_EXIT_CRITICAL();
    
    return 0;
}


/*****************************************************************************************
** 函 数 名 称 ：Uart0_Read
** 函 数 功 能 ：读Uart0口的数据
** 输 入 ： port：IO端口号
**       ： buf：存放读到的数据
**       ： len：要读取数据的长度，如果为0，则表示有多少读多少
** 输 出 ： 操作成功返回读到的字节数
**       ： -1：操作失败
** 全局变量:
** 调用模块:
**
** 作　者: Arjun
** 日　期: 2014年3月11日
*****************************************************************************************/
int32_t UartRead(int32_t port, void *buf, uint32_t buf_len)
{
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    int16_t len = 0;
    uint8_t temp,*tmp;
    tmp = (uint8_t *)buf;
    
    DataQueue *queue_temp = 0;
    if (port == COM1)
    {
        queue_temp = (DataQueue *)UART1RecvBuf;
    }
	else if ((port == COM3))
    {
        queue_temp = (DataQueue *)UART3RecvBuf;
    }
    else if (port == COM6)
    {
        queue_temp = (DataQueue *)UART6RecvBuf;
    }
    else
    {
        return -1;
    }

    OS_ENTER_CRITICAL();
    while((QueueRead(&temp, queue_temp) == QUEUE_OK) && (len < buf_len))
    {
        tmp[len++] = temp;
        if (len == buf_len)
        {
            break;  
        }
    }
    OS_EXIT_CRITICAL();

    return len;
}

/*****************************************************************************************
** 函 数 名 称 ：Uart0_Ioctl
** 函 数 功 能 ：对Uart0口操作控制
** 输 入 ： port：IO端口号
**       ： cmd ：读写权限
**       ： arg：要读、写的数据
**       ： len：要读、写的数据长度
** 输 出 ： 0：操作成功
**       ： -1：操作失败
** 全局变量:
** 调用模块:Uart0Read,Uart0Write
**
** 作　者: Arjun
** 日　期: 2014年3月11日
*****************************************************************************************/

int8_t UartIoctl(int32_t port, uint32_t cmd, va_list argp)
{
    int8_t ret = -1;
    DataQueue *queue_recv_temp = 0;
    DataQueue *queue_send_temp = 0;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    //  USART_TypeDef * GPIOPort;

    if (port == COM1)
    {
        queue_recv_temp = (DataQueue *)UART1RecvBuf;
        queue_send_temp = (DataQueue *)UART1SendBuf;
        //      GPIOPort = USART1;
    }
	else if ((port == COM3))
    {
        queue_recv_temp = (DataQueue *)UART3RecvBuf;
        queue_send_temp = (DataQueue *)UART3SendBuf;
        //      GPIOPort = USART3;
    }
    else if (port == COM6)
    {
        queue_recv_temp = (DataQueue *)UART6RecvBuf;
        queue_send_temp = (DataQueue *)UART6SendBuf;
    }
    else
    {
        return -1;
    }

    switch (cmd)
    {
    case CLEAR_BUFFER:
        OS_ENTER_CRITICAL();
        QueueFlush(queue_recv_temp);
        QueueFlush(queue_send_temp);
        OS_EXIT_CRITICAL();
        break;

    case CLEAR_INPUT_BUFFER:
        OS_ENTER_CRITICAL();
        QueueFlush(queue_recv_temp);
        OS_EXIT_CRITICAL();
        break;

    case CLEAR_OUTPUT_BUFFER:
        OS_ENTER_CRITICAL();
        QueueFlush(queue_send_temp);
        OS_EXIT_CRITICAL();
        break;

        default
            :
                return -2;
    }

    return ret;
}

static void usart_IRQHandler(USART_TypeDef *USARTPort, DataQueue *recv_queue_temp, DataQueue *send_queue_temp)
{
    uint8_t Res;
    uint8_t temp;
    if (USART_GetITStatus(USARTPort, USART_IT_RXNE) != RESET) //检查指定的 USART 接收中断发生与否 
    {   //接收中断
        do
        {
            Res = USART_ReceiveData(USARTPort);//读取接收到的数据
            QueueWrite(recv_queue_temp, Res);
        } while (USART_GetITStatus(USARTPort, USART_IT_RXNE) != RESET);
        USART_ClearITPendingBit(USARTPort, USART_IT_RXNE);   //清接收中断标记
    }
//    else if ((USART_GetITStatus(USARTPort, USART_IT_TXE) != RESET) || (USART_GetITStatus(USARTPort, USART_IT_IDLE) != RESET))
    if(USART_GetITStatus(USARTPort, USART_IT_TC) != RESET)
    {
        //发送中断
        if (QueueRead(&temp, send_queue_temp) == QUEUE_OK)
        {
            USART_SendData(USARTPort, temp);       //发送数据
        }
        else
        {
            USART_ITConfig(USARTPort, USART_IT_TC, DISABLE);//使能发送中断
        }
    }
}

#if 1
void USART1_IRQHandler(void)          //串口1中断服务程序
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif
    usart_IRQHandler(USART1, (DataQueue *)UART1RecvBuf, (DataQueue *)UART1SendBuf);
    
#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}
#endif

void USART2_IRQHandler(void)                //串口2中断服务程序
{
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif

    usart_IRQHandler(USART2, (DataQueue *)UART2RecvBuf, (DataQueue *)UART2SendBuf);

#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}

extern void ModbusSlaveReceive(uint8_t *dat);
void USART6_IRQHandler(void)      //串口2中断服务程序
{
    uint8_t res;
    uint8_t temp;
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif
    //usart_IRQHandler(USART6, (DataQueue *)UART6RecvBuf, (DataQueue *)UART6SendBuf);
    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET) //检查指定的 USART 接收中断发生与否 
    {   //接收中断
        do
        {
            res = USART_ReceiveData(USART6);//读取接收到的数据
            //QueueWrite(recv_queue_temp, Res);
            ModbusSlaveReceive(&res);
        } while (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET);
        USART_ClearITPendingBit(USART6, USART_IT_RXNE);   //清接收中断标记
    }
//    else if ((USART_GetITStatus(USARTPort, USART_IT_TXE) != RESET) || (USART_GetITStatus(USARTPort, USART_IT_IDLE) != RESET))
    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
    {
        //发送中断
        if (QueueRead(&temp, UART6SendBuf) == QUEUE_OK)
        {
            USART_SendData(USART6, temp);       //发送数据
        }
        else
        {
            USART_ITConfig(USART6, USART_IT_TC, DISABLE);//使能发送中断
        }
    }
#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}

void USART3_IRQHandler(void)    //串口3中断服务程序
{
    if (usart_mode == 1)
    {   //如果工作模式为MDB模式，执行MDB中断函数
        MDB_IRQHandler();
        return;
    }
    else
    {
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
        OSIntEnter();
#endif
        usart_IRQHandler(USART3, (DataQueue *)UART3RecvBuf, (DataQueue *)UART3SendBuf);

#if SYSTEM_SUPPORT_OS
        OSIntExit();        //退出中断
#endif
        return;
    }


}



