

#include "bsp_includes.h"
#include "cj01_mdb.h"
#include "cj01_usart.h"
#include "queue.h"
#include "cj01_IRQ_priority.h" 

extern uint32_t UART3RecvBuf[64];
extern uint32_t UART3SendBuf[64];


typedef struct
{
    uint16_t *in;
    uint16_t *out;
    uint16_t dat_len;
    uint16_t buf_len;
    uint16_t buf[2];
} s_mdb_queue_t;



extern uint8_t usart_mode ; //串口3工作模式，0为串口模式，1为MDB模式


/*****************************************************************************************
** 函数名称 ：	MdbOpen
** 函数功能 ：  打开MDB口，对MDB口做初始化
** 输    入 ：	int32_t port	IO端口号
** 输    入 ：	void * config	配置参数
** 输    入 ：	uint8_t len		配置参数的长度	
** 输    出 ：	int8_t  		0：操作成功,	-1：操作失败
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/08/04
*****************************************************************************************/
int8_t MdbOpen(int32_t port, void *config, uint8_t len)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    int8_t tmp = 0;
   	s_mdb_queue_t *queue_temp;

	MDB_ENABLE_GPIO_RCC(); //使能GPIO时钟
	MDB_ENABLE_USART_RCC(); //使能USART时钟

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIO时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_0 | GPIO_PIN_1;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
    GPIO_SetBits(GPIOB,GPIO_PIN_0);   //选择通道11
    GPIO_SetBits(GPIOB,GPIO_PIN_1);
    OSTimeDly(OS_TICKS_PER_SEC/200);
    
	//串口对应引脚复用映射
	GPIO_PinAFConfig(MDB_GPIO_PORT_TX, MDB_GPIO_PIN_SOURCE_TX, MDB_GPIO_AF_USART); //GPIO_TX复用为USART
	GPIO_PinAFConfig(MDB_GPIO_PORT_RX, MDB_GPIO_PIN_SOURCE_RX, MDB_GPIO_AF_USART); //GPIO_RX复用为USART

	//IO口模式配置
	GPIO_InitStructure.GPIO_Pin = MDB_GPIO_PIN_TX; //TX引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(MDB_GPIO_PORT_TX, &GPIO_InitStructure); //初始化TX引脚

	GPIO_InitStructure.GPIO_Pin = MDB_GPIO_PIN_RX; //RX引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(MDB_GPIO_PORT_RX, &GPIO_InitStructure); //初始化RX引脚

	//USART初始化设置
    USART_InitStructure.USART_BaudRate = 9600;//波特率9600
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为9位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
    USART_Init(MDB_USART_PORT, &USART_InitStructure); //初始化串口1

    USART_Cmd(MDB_USART_PORT, ENABLE);  //使能串口1

    USART_ClearFlag(MDB_USART_PORT, USART_FLAG_TC);

    USART_ITConfig(MDB_USART_PORT, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = MDB_USART_IRQ_N;//串口中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MDB_IRQ_CHANNEL_PREEMPTION_PRIORITY; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MDB_IRQ_CHANNEL_SUB_PRIORITY;      //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

    queue_temp = MDB_RECV_QUENE;
    queue_temp->in = queue_temp->buf;
    queue_temp->out = queue_temp->buf;
    queue_temp->dat_len = 0;
    queue_temp->buf_len = (MDB_RECV_QUEUE_BUFF_LEN - 12) / 2;

    queue_temp = MDB_SEND_QUENE;
    queue_temp->in = queue_temp->buf;
    queue_temp->out = queue_temp->buf;
    queue_temp->dat_len = 0;
    queue_temp->buf_len = (MDB_SEND_QUEUE_BUFF_LEN - 12) / 2;
    usart_mode = 1; //串口3工作模式为MDB
    return tmp;
}




/*****************************************************************************************
** 函数名称 ：	MdbClose
** 函数功能 ：  关闭相应的mdb端口，释放资源
** 输    入 ：	int32_t port			
** 输    出 ：	int8_t  			
** 全局变量 :   0：操作成功, -1：操作失败
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/08/04
*****************************************************************************************/
int8_t MdbClose(int32_t port)
{
    USART_TypeDef *GPIOPort = MDB_USART_PORT;
    s_mdb_queue_t *queue_temp = MDB_SEND_QUENE;
    while ((queue_temp->dat_len != 0) || (USART_GetFlagStatus(GPIOPort, USART_FLAG_TXE) == RESET))
    {
        OSTimeDly(OS_TICKS_PER_SEC/200);
    }
    usart_mode = 0; //串口3工作模式切换回普通串口
    return 0;
}


/*****************************************************************************************
** 函数名称 ：	MdbWrite
** 函数功能 ：  
** 输    入 ：	int32_t port			
** 输    入 ：	void * buf			要写的数据
** 输    入 ：	uint16_t buf_len	要写的数据长度		
** 输    出 ：	int8_t  			0：操作成功,-1：操作失败
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/08/04
*****************************************************************************************/
int8_t MdbWrite(int32_t port, void *buf, uint16_t buf_len)
{
    uint16_t *data = (uint16_t *)buf;
    s_mdb_queue_t *queue_temp = MDB_SEND_QUENE;
    uint16_t temp;
    USART_TypeDef *GPIOPort= MDB_USART_PORT;

#if OS_CRITICAL_METHOD == 3u
        uint32_t cpu_sr;
#endif

#if 1
        OS_ENTER_CRITICAL();
    	while (buf_len > 0)
        {
    		if (queue_temp->dat_len < queue_temp->buf_len)
    		{
    			*(queue_temp->in++) = *(data++);
    			queue_temp->dat_len++;
    			buf_len--;
    			//  处理buffer溢出
    			if (queue_temp->in >= (queue_temp->buf + queue_temp->buf_len))
    			{
                    queue_temp->in = queue_temp->buf;
    			}
                if (USART_GetFlagStatus(GPIOPort, USART_FLAG_TXE) != RESET)
    		    {
                    temp = (uint16_t)(*queue_temp->out);

                    if ((*queue_temp->out & 0xFF00) == MDB_MODE_SET)
                    {
                        temp = (1 << 8)+ (temp & 0xFF); 
                    }
                    else
                    {
                        temp = (temp & 0xFF); 
                    }
    				queue_temp->out++;
    			    queue_temp->dat_len--;
    				//  处理buffer溢出
    				if (queue_temp->out >= (queue_temp->buf + queue_temp->buf_len))
    				{
    	                queue_temp->out = queue_temp->buf;
    				}
                    USART_SendData(GPIOPort, (uint16_t)temp); //通过外设 USART 发送单个数据 
                    USART_ITConfig(GPIOPort, USART_IT_TXE, ENABLE);//使能发送中断
    		    }
    		}
    		else
    		{
                if (USART_GetFlagStatus(GPIOPort, USART_FLAG_TXE) != RESET)
    		    {
    		        //UART发送保持寄存器空
                    temp = (uint16_t)(*queue_temp->out);
                    if ((*queue_temp->out & 0xFF00) == MDB_MODE_SET)
                    {
                        temp |= (1 << 9); //第九位置1
                    }
                    else
                    {
                        temp &= ~(1 << 9); //第九位清0
                    }

    				queue_temp->out++;
    			    queue_temp->dat_len--;
    				//  处理buffer溢出
    				if (queue_temp->out >= (queue_temp->buf + queue_temp->buf_len))
    				{
    	                queue_temp->out = queue_temp->buf;
    				}
                    USART_SendData(GPIOPort, (uint16_t)temp); //通过外设 USART 发送单个数据 
                    USART_ITConfig(GPIOPort, USART_IT_TXE, ENABLE);//使能发送中断
    		    }
    	        OS_EXIT_CRITICAL();
    			OSTimeDly(OS_TICKS_PER_SEC/200);
    	        OS_ENTER_CRITICAL();
    		}
    	}
    	OS_EXIT_CRITICAL();
#endif
    return 0;
}


/*****************************************************************************************
** 函 数 名 称 ：Uart0_Read
** 函 数 功 能 ：读Uart0口的数据
** 输 入 ：	port：IO端口号
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
int16_t MdbRead(int32_t port, void *buf, uint16_t buf_len)
{
    int16_t len = 0;
    int16_t dat_len = buf_len;
    s_mdb_queue_t *queue_temp = MDB_RECV_QUENE;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    OS_ENTER_CRITICAL();
    while ((queue_temp->dat_len > 0) && (dat_len > 0))
    {

        len = queue_temp->buf + queue_temp->buf_len - queue_temp->out;
        if (len > dat_len)
        {
            len = dat_len;
        }
        if (len > queue_temp->dat_len)
        {
            len = queue_temp->dat_len;
        }
        memcpy(buf, queue_temp->out, len * 2);
        queue_temp->out += len;
        queue_temp->dat_len -= len;
        dat_len -= len;
        if (queue_temp->out >= (queue_temp->buf + queue_temp->buf_len))
        {
            queue_temp->out = queue_temp->buf;
        }

    }
    OS_EXIT_CRITICAL();

    return buf_len - dat_len;
}

/*****************************************************************************************
** 函 数 名 称 ：Uart0_Ioctl
** 函 数 功 能 ：对Uart0口操作控制
** 输 入 ：	port：IO端口号
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
int8_t MdbIoctl(int32_t port, uint32_t cmd, va_list argp)
{
    int8_t ret = -1;
    s_mdb_queue_t *queue_temp;
#if OS_CRITICAL_METHOD == 3u
    uint32_t cpu_sr;
#endif
    switch (cmd)
    {
    case CLEAR_BUFFER:
        OS_ENTER_CRITICAL();
        queue_temp = MDB_RECV_QUENE;
        queue_temp->in = queue_temp->buf;
        queue_temp->out = queue_temp->buf;
        queue_temp->dat_len = 0;
        queue_temp->buf_len = (MDB_RECV_QUEUE_BUFF_LEN - 12) / 2;

        queue_temp = MDB_SEND_QUENE;
        queue_temp->in = queue_temp->buf;
        queue_temp->out = queue_temp->buf;
        queue_temp->dat_len = 0;
        queue_temp->buf_len = (MDB_SEND_QUEUE_BUFF_LEN - 12) / 2;
        OS_EXIT_CRITICAL();
        break;
    case CLEAR_INPUT_BUFFER:
        OS_ENTER_CRITICAL();
        queue_temp = MDB_RECV_QUENE;
        queue_temp->in = queue_temp->buf;
        queue_temp->out = queue_temp->buf;
        queue_temp->dat_len = 0;
        queue_temp->buf_len = (MDB_RECV_QUEUE_BUFF_LEN - 12) / 2;
        OS_EXIT_CRITICAL();
        break;
    case CLEAR_OUTPUT_BUFFER:
        OS_ENTER_CRITICAL();
        queue_temp = MDB_SEND_QUENE;
        queue_temp->in = queue_temp->buf;
        queue_temp->out = queue_temp->buf;
        queue_temp->dat_len = 0;
        queue_temp->buf_len = (MDB_SEND_QUEUE_BUFF_LEN - 12) / 2;
        OS_EXIT_CRITICAL();
        break;
    case MDB_POWER_OP:
        ret = va_arg(argp, int);
        if (ret)
        {
            ret = GPIO_OUT_L;
        }
        else
        {
            ret = GPIO_OUT_H;
        }
        //ret = IoWrite(GPIO_20, &ret, 1);
        break;
    default:
        break;
    }
    return ret;
}


/*****************************************************************************************
** 函数名称 ：	MDB_IRQHandler
** 函数功能 ：	MDB中断服务程序需要放进相应的串口中断函数内处理
** 输    入 ：	void				
** 输    出 ：	void    			
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/08/12
*****************************************************************************************/
void MDB_IRQHandler(void)               
{
    uint16_t Res;
    uint16_t temp;
	USART_TypeDef *GPIOPort = MDB_USART_PORT;
    s_mdb_queue_t *send_queue = MDB_SEND_QUENE;
    s_mdb_queue_t *resv_queue = MDB_RECV_QUENE;

#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
    OSIntEnter();
#endif

    if (USART_GetITStatus(GPIOPort, USART_IT_RXNE) != RESET) //检查指定的 USART 接收中断发生与否 
    {   //接收中断
        do
        {
            Res = USART_ReceiveData(GPIOPort);//读取接收到的数据
            if (resv_queue->dat_len < resv_queue->buf_len)
            {
                if ((Res & 0xFF00) == 0)
                {
                    Res = MDB_MODE_CLR + (Res & 0xFF);                   
                }
                else
                {
                    Res = MDB_MODE_SET  + (Res & 0xFF);
                }
                *resv_queue->in = Res;
                resv_queue->dat_len++;
                if (++resv_queue->in >= (resv_queue->buf + resv_queue->buf_len))
                {
                    resv_queue->in = resv_queue->buf;
                }
            }
 
        } while (USART_GetITStatus(GPIOPort, USART_IT_RXNE) != RESET);
        USART_ClearITPendingBit(GPIOPort, USART_IT_RXNE);   //清接收中断标记
    }
    //    else if ((USART_GetITStatus(GPIOPort, USART_IT_TXE) != RESET) || (USART_GetITStatus(GPIOPort, USART_IT_IDLE) != RESET))
    if (USART_GetITStatus(GPIOPort, USART_IT_TXE) != RESET)
    {
        //发送中断
        if (send_queue->dat_len > 0)
        {
            temp = (uint16_t)(*send_queue->out);
            if ((*send_queue->out & 0xFF00) == MDB_MODE_SET)
            {
                temp |= (1 << 9); //第九位置1
            }
            else
            {
                temp &= ~(1 << 9); //第九位清0
            }

            send_queue->dat_len--;
            if (++send_queue->out >= (send_queue->buf + send_queue->buf_len))
            {
                send_queue->out = send_queue->buf;
            }
            USART_SendData(GPIOPort, (uint16_t)temp); //通过外设 USART 发送单个数据 
        }
        else
        {
            USART_ITConfig(GPIOPort, USART_IT_TXE, DISABLE);   //队列空，则禁止发送中断
        }
    }

#if SYSTEM_SUPPORT_OS
    OSIntExit();        //退出中断
#endif
}





